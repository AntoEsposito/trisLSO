#include "funzioni.h"

struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore, const int client_sd)
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
    nuova_testa -> client_sd = client_sd;
    nuova_testa -> next_node = testa;

    return nuova_testa;
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
    testa = crea_giocatore_in_testa(testa, nome_giocatore, client_sd);
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
void segnala_nuovo_giocatore(struct nodo_giocatore *testa, const pthread_t tid_mittente)
{ 
    pthread_t tid_ricevente;
    struct nodo_giocatore *tmp = testa;
    do
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente) pthread_kill(tid_ricevente, SIGUSR2);
        tmp = tmp -> next_node;
    } while (tmp -> next_node != NULL);
}
int cerca_client_sd(struct nodo_giocatore *testa, const pthread_t tid)
{
    struct nodo_giocatore *tmp = testa;
    do
    {
        if(tmp -> tid_giocatore == tid) return tmp -> client_sd;
        tmp = tmp -> next_node;
    } while (tmp -> next_node != NULL);
    return 0; //se non lo trova (caso improbabile)
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct nodo_partita* crea_partita_in_testa(struct nodo_partita *testa, const char *nome_proprietario)
{
    struct nodo_partita *nuova_testa = (struct nodo_partita *) malloc(sizeof(struct nodo_partita));
    if (nuova_testa == NULL) return testa; //creazione fallita, la lista rimane invariata

    memset(nuova_testa, 0, sizeof(struct nodo_partita));
    strcpy(nuova_testa -> proprietario, nome_proprietario);
    nuova_testa -> stato = NUOVA_CREAZIONE;
    nuova_testa -> next_node = testa;
    if(testa -> stato == NUOVA_CREAZIONE) testa -> stato = IN_ATTESA;

    return nuova_testa;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
