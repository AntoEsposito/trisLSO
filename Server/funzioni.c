#include "funzioni.h"

struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore)
{
    //il nodo diventa la nuova testa!
    struct nodo_giocatore *nodo = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nodo == NULL)
    {
        pthread_exit(NULL);
    }

    memset(nodo, 0, sizeof(struct nodo_giocatore)); //pulisce il nodo per sicurezza
    strcpy(nodo -> nome, nome_giocatore);
    nodo -> vittorie = 0;
    nodo -> sconfitte = 0;
    nodo -> pareggi = 0;
    nodo -> stato = IN_LOBBY;
    nodo -> tid_giocatore = pthread_self();

    if (testa != NULL) nodo -> next_node = testa;
    else nodo -> next_node = NULL;

    return nodo;
}
bool esiste_giocatore(struct nodo_giocatore *testa, const char *nome_giocatore)
{
    if (testa != NULL)
    {
        struct nodo_giocatore *tmp = testa;
        while (tmp -> next_node != NULL)
        {
            if (strcmp(tmp -> nome, nome_giocatore) == 0) return true;
            tmp = tmp -> next_node;
        }
    }
    return false;
}
char* verifica_giocatore(struct nodo_giocatore *testa, const int client_sd)
{
    char *giocatore = (char *) malloc(MAXPLAYER*sizeof(char));
    memset(giocatore, 0, MAXPLAYER);
    int n_byte;

    if (send(client_sd, "Benvenuto, inserisci il tuo nome per registrarti (max 15 caratteri)\n", 69, 0) <= 0)
    {
        close(client_sd);
        free(giocatore);
        pthread_exit(NULL);
    }

    while(true)
    {
        //si occupa il codice del client di verificare che i caratteri inviati siano al massimo 15
        if ((n_byte = recv(client_sd, giocatore, MAXPLAYER, 0)) <= 0 )
        {
            close(client_sd);
            free(giocatore);
            pthread_exit(NULL);
        }

        giocatore[n_byte] = '\0';
        if(!(esiste_giocatore(testa, giocatore))) break;

        if (send(client_sd, "Il nome selezionato è già utilizzato\n", 40, 0) <= 0)
        {
            close(client_sd);
            free(giocatore);
            pthread_exit(NULL);
        }
    }

    if (send(client_sd, "Registrazione completata\n", 26, 0) <= 0)
    {
        close(client_sd);
        free(giocatore);
        pthread_exit(NULL);
    }

    return giocatore;
}
struct nodo_giocatore* registra_giocatore(struct nodo_giocatore *testa, const int client_sd)
{
    char *nome_giocatore = verifica_giocatore(testa, client_sd);
    testa = crea_giocatore_in_testa(testa, nome_giocatore);
    free(nome_giocatore);
    return testa;
}
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo)
{
    if (nodo != NULL && testa == nodo) //significa che si sta cercando di cancellare la testa
    {
        free(nodo);
        testa = NULL;
    }
    else if (nodo != NULL)
    {
        struct nodo_giocatore *tmp = testa;
        while(tmp -> next_node != nodo)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;
        free(nodo);
    }
    return testa;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unsigned short int invia_partite(struct nodo_partita *testa, const int client_sd)
{
    char outbuffer[MAXOUT];
    memset(outbuffer, 0, MAXOUT);

    unsigned int indice = 0; //conta le partite a cui è possibile aggiungersi
    char stringa_indice[3]; //l'indice verrà convertito in questa stringa
    memset(stringa_indice, 0, 3);

    struct nodo_partita *tmp = testa;
    char stato_partita[27];

    if(testa == NULL)
    {
        if(send(client_sd, "Non ci sono partite attive al momento, scrivi \"crea\" per crearne una nuova\n", 76, 0) <= 0) return 1;
    }
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
            if(send(client_sd, outbuffer, strlen(outbuffer), 0) <= 0) return 1;

            tmp = tmp -> next_node;
        }
        while (tmp -> next_node != NULL);
        if(send(client_sd, "Unisciti a una partita in attesa scrivendo il relativo ID o scrivi \"crea\" per crearne una\n", 91, 0) <= 0) return 1;
    }
    return 0;
}


int inizializza_server() //crea la socket, si mette in ascolto e restituisce il socket descriptor
{
    int sd;
    struct sockaddr_in address;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
        perror("socket creation error"), exit(EXIT_FAILURE);

    memset(&address, 0, lenght);
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *) &address, lenght)<0)
        perror("binding error"), exit(EXIT_FAILURE);

    if (listen(sd, 5)<0)
        perror("listen error"), exit(EXIT_FAILURE);

    return sd;
}
