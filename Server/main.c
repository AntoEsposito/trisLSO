#include "funzioni.h"

//il puntatore alla testa delle liste è allocato staticamente per essere gestito dai thread
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

int main()
{
    int sd, client_sd; //socket descriptor del server e dei client
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    sd = inizializza_server(); //il server gira sulla porta 8080
    signal(SIGUSR1, invia_partite);
    signal(SIGUSR2, handler_nuovo_giocatore);
    while(1)
    {
        if ((client_sd = accept(sd, (struct sockaddr *) &client_address, &lenght)) < 0)
        {
            perror("accept error");
            continue;
        }
    }
    return 0;
}


void handler_nuovo_giocatore()
{
    const pthread_t tid = pthread_self();
    int sd;
    struct nodo_giocatore *tmp = testa_giocatori;
    do
    {
        if (tmp -> tid_giocatore != tid)
        {
            sd = tmp -> client_sd;
            //l'handler ignora gli errori e va alla successiva iterazione del ciclo per essere il più veloce possibile
            if (send(sd, "Un nuovo giocatore è appena entrato in lobby!\n", 48, 0) <= 0) continue;
        }
        tmp = tmp -> next_node;
    } while (tmp -> next_node != NULL);
}
void invia_partite()
{
    const pthread_t tid = pthread_self();
    struct nodo_partita *tmp = testa_partite;
    const int client_sd = cerca_client_sd(testa_giocatori, tid);

    char outbuffer[MAXOUT];
    memset(outbuffer, 0, MAXOUT);

    unsigned int indice = 0; //conta le partite a cui è possibile aggiungersi
    char stringa_indice[3]; //l'indice verrà convertito in questa stringa
    memset(stringa_indice, 0, 3);

    char stato_partita[27];

    if(tmp == NULL) send(client_sd, "Non ci sono partite attive al momento, scrivi \"crea\" per crearne una nuova\n", 76, 0);
    //anche questa funzione non fa error checking in quanto handler che deve essere eseguito molte volte e velocemente
    else
    {
        do
        {
            memset(stato_partita, 0, 27);
            switch (tmp -> stato)
            {
                case NUOVA_CREAZIONE:
                    strcpy(stato_partita, "Nuova creazione: in attesa");
                    break;
                case IN_ATTESA:
                    strcpy(stato_partita, "In attesa di un giocatore");
                    break;
                case IN_CORSO:
                    strcpy(stato_partita, "In corso");
                    break;
                case TERMINATA:
                    strcpy(stato_partita, "Terminata");
                    break;
            }

            strcat(outbuffer, "Partita di "); strcat(outbuffer, tmp -> proprietario); strcat(outbuffer, "\n");
            strcat(outbuffer, "Avversario: "); strcat(outbuffer, tmp -> avversario); strcat(outbuffer, "\n");
            strcat(outbuffer, "Stato: "); strcat(outbuffer, stato_partita);

            if (tmp -> stato == NUOVA_CREAZIONE || tmp -> stato == IN_ATTESA)
            {
                indice ++;
                sprintf(stringa_indice, "%u", indice);
                strcat(outbuffer, " || ID: "); strcat(outbuffer, stringa_indice);
            }
            strcat(outbuffer, "\n");
            send(client_sd, outbuffer, strlen(outbuffer), 0);

            tmp = tmp -> next_node;
        }
        while (tmp -> next_node != NULL);
        send(client_sd, "Unisciti a una partita in attesa scrivendo il relativo ID o scrivi \"crea\" per crearne una\n", 91, 0);
    }
}