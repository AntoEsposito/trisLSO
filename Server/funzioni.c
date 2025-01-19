#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

//funzioni di gestione giocatori

void crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd)
{
    struct nodo_giocatore *nuova_testa = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nuova_testa == NULL) pthread_exit(NULL);

    memset(nuova_testa, 0, sizeof(struct nodo_giocatore)); //pulisce il nodo per sicurezza
    strcpy(nuova_testa -> nome, nome_giocatore);
    nuova_testa -> vittorie = 0;
    nuova_testa -> sconfitte = 0;
    nuova_testa -> pareggi = 0;
    nuova_testa -> stato = IN_LOBBY;
    nuova_testa -> tid_giocatore = pthread_self();
    nuova_testa -> sd_giocatore = client_sd;
    nuova_testa -> next_node = testa_giocatori;

    testa_giocatori = nuova_testa;
}
bool esiste_giocatore(const char *nome_giocatore)
{
    if (testa_giocatori != NULL)
    {
        struct nodo_giocatore *tmp = testa_giocatori;
        while (tmp -> next_node != NULL)
        {
            if (strcmp(tmp -> nome, nome_giocatore) == 0) return true;
            tmp = tmp -> next_node;
        }
    }
    return false;
}
struct nodo_giocatore* trova_giocatore_da_nome(const char *nome_giocatore) //TODO
{
    if (testa_giocatori != NULL)
    {
        struct nodo_giocatore *tmp = testa_giocatori;
        while (tmp -> next_node != NULL)
        {
            if (strcmp(tmp -> nome, nome_giocatore) == 0) return tmp;
            tmp = tmp -> next_node;
        }
    }
    return NULL; //improbabile
}
struct nodo_giocatore* trova_giocatore_da_tid(const pthread_t tid)  //TODO
{
    if (testa_giocatori != NULL)
    {
        struct nodo_giocatore *tmp = testa_giocatori;
        do
        {
            if (tmp -> tid_giocatore == tid) return tmp;
            tmp = tmp -> next_node;
        } while (tmp -> next_node != NULL);
    }
    return NULL; //improbabile
}
char* verifica_giocatore(const int client_sd)
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
        if(!(esiste_giocatore(giocatore))) break;

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
void registra_giocatore(const int client_sd)
{
    char *nome_giocatore = verifica_giocatore(client_sd);
    crea_giocatore_in_testa(nome_giocatore, client_sd);
    free(nome_giocatore);
}
void cancella_giocatore(struct nodo_giocatore *nodo)
{
    if (nodo != NULL && testa_giocatori == nodo) //significa che si sta cercando di cancellare la testa
    {
        free(nodo);
        testa_giocatori = testa_giocatori -> next_node;
    }
    else if (nodo != NULL)
    {
        struct nodo_giocatore *tmp = testa_giocatori;
        while(tmp -> next_node != nodo)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;
        free(nodo);
    }
}
void segnala_nuovo_giocatore(const pthread_t tid_mittente)
{ 
    pthread_t tid_ricevente;
    struct nodo_giocatore *tmp = testa_giocatori;
    do
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente) pthread_kill(tid_ricevente, SIGUSR2);
        tmp = tmp -> next_node;
    } while (tmp -> next_node != NULL);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di gestione partite

void crea_partita_in_testa(const char *nome_proprietario, const int sd_proprietario)
{
    struct nodo_partita *nuova_testa = (struct nodo_partita *) malloc(sizeof(struct nodo_partita));
    if (nuova_testa == NULL) return; //creazione fallita, la lista rimane invariata

    memset(nuova_testa, 0, sizeof(struct nodo_partita));
    strcpy(nuova_testa -> proprietario, nome_proprietario);
    nuova_testa -> sd_proprietario = sd_proprietario;
    nuova_testa -> stato = NUOVA_CREAZIONE;
    nuova_testa -> next_node = testa_partite;
    if(testa_partite -> stato == NUOVA_CREAZIONE) testa_partite -> stato = IN_ATTESA;

    testa_partite = nuova_testa;
}
bool unione_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario)
{
    const int sd_proprietario = partita -> sd_proprietario;
    char buffer[MAXOUT];
    memset(buffer, 0, MAXOUT);
    char risposta; //si occupa il codice client di verificare che l'input sia o "s" o "n"
    strcat(buffer, nome_avversario); strcat(buffer, " vuole unirsi alla tua partita, accetti? [s/n]\n");

    if(send(sd_proprietario, buffer, strlen(buffer), 0) <= 0) //il proprietario si è disconnesso o simili
    {
        error_handler(partita, partita -> proprietario);
        return false;
    }
    if(recv(sd_proprietario, &risposta, 1, 0) <= 0)
    {
        error_handler(partita, partita -> proprietario);
        return false;
    }

    risposta = toupper(risposta); //case insensitive
    if(risposta == 'S')
    {
        strcpy(partita -> avversario, nome_avversario);
        partita -> sd_avversario = sd_avversario;
        return true;
    }
    else return false;
}
void cancella_partita(struct nodo_partita *nodo)
{
    if (nodo != NULL && testa_partite == nodo) //significa che si sta cercando di cancellare la testa
    {
        free(nodo);
        testa_partite = testa_partite -> next_node;
    }
    else if (nodo != NULL)
    {
        struct nodo_partita *tmp = testa_partite;
        while(tmp -> next_node != nodo)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;
        free(nodo);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni generali server

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
void error_handler(struct nodo_partita *partita, const char *nome_giocatore)
{
    if (partita != NULL) cancella_partita(partita);

    struct nodo_giocatore *giocatore = trova_giocatore_da_nome(nome_giocatore);
    const pthread_t tid = giocatore -> tid_giocatore;

    if(giocatore != NULL) 
    {
        pthread_kill(tid, SIGALRM);
        cancella_giocatore(giocatore);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di signal handling

void sigalrm_handler()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    close(giocatore -> sd_giocatore);
    pthread_exit(NULL);
}
void handler_nuovo_giocatore()
{
    const pthread_t tid = pthread_self();
    struct nodo_giocatore *tmp = testa_giocatori;

    char messaggio[MAXOUT];
    memset(messaggio, 0, MAXOUT);
    strcat(messaggio, tmp -> nome); strcat(messaggio, " è appena entrato in lobby!\n");

    int sd;
    do
    {
        if (tmp -> tid_giocatore != tid)
        {
            sd = tmp -> sd_giocatore;
            //l'handler ignora gli errori per essere il più veloce possibile
            send(sd, messaggio, strlen(messaggio), 0);
        }
        tmp = tmp -> next_node;
    } while (tmp -> next_node != NULL);
}
void invia_partite()
{
    const pthread_t tid = pthread_self();
    struct nodo_partita *tmp = testa_partite;
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(tid);
    const int client_sd = giocatore -> sd_giocatore;

    char outbuffer[MAXOUT];
    memset(outbuffer, 0, MAXOUT);

    unsigned int indice = 0; //conta le partite a cui è possibile aggiungersi
    char stringa_indice[3]; //l'indice verrà convertito in questa stringa
    memset(stringa_indice, 0, 3);

    char stato_partita[27];

    if(tmp == NULL) send(client_sd, "Non ci sono partite attive al momento, scrivi \"crea\" per crearne una nuova\n", 76, 0);
    //anche questa funzione non fa error checking in quanto handler
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