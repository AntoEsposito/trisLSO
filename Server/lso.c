#include "lso.h"


struct nodo_giocatore* crea_giocatore()
{
    struct nodo_giocatore *nodo = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nodo == NULL)
    {
        perror("errore creazione giocatore");
        return NULL;
    }
    memset(nodo, 0, sizeof(struct nodo_giocatore)); //pulisce la struct per sicurezza
    nodo -> next_node = NULL;
    return nodo;
}
bool esiste_giocatore(struct nodo_giocatore *testa, const char *giocatore)
{
    if (testa != NULL)
    {
        struct nodo_giocatore *tmp = testa;
        while (tmp -> next_node != NULL)
        {
            if ((strcmp(tmp -> nome, giocatore)) == 0) return true;
            tmp = tmp -> next_node;
        }
    }
    return false;
}
void registra_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo, const int client_sd)
{
    char giocatore[MAXPLAYER];
    memset(giocatore, 0, MAXPLAYER);
    int n_byte;

    if (send(client_sd, "Benvenuto, inserisci il tuo nome per registrarti (max 15 caratteri)\n", 69, 0) <= 0)
    {
        perror("errore invio messaggio");
        close(client_sd);
        pthread_exit(NULL);
    }
    while(true)
    {
        if ((n_byte = recv(client_sd, giocatore, MAXPLAYER, 0)) <= 0 ) //si occupa il client di verificare che i caratteri inviati siano al massimo 15
        {
            perror("errore ricezione messaggio");
            close(client_sd);
            pthread_exit(NULL);
        }

        giocatore[n_byte] = '\0';
        if(esiste_giocatore(testa, giocatore) == false) break;

        if (send(client_sd, "Il nome selezionato è già utilizzato\n", 40, 0) <= 0)
        {
            perror("errore invio messaggio");
            close(client_sd);
            pthread_exit(NULL);
        }
    } 
    //registrazione terminata, il nodo giocatore viene inizializzato
    strcpy(nodo -> nome, giocatore);
    nodo -> vittorie = 0;
    nodo -> sconfitte = 0;
    nodo -> pareggi = 0;
    nodo -> stato = IN_LOBBY;
    nodo -> tid_giocatore = pthread_self();

    if (send(client_sd, "Registrazione completata, benvenuto\n", 37, 0) <= 0)
    {
        perror("errore invio messaggio");
        close(client_sd);
        pthread_exit(NULL);
    }
}
void aggiungi_giocatore(struct nodo_giocatore *testa, const int client_sd)
{
    if (testa == NULL)
    {
        registra_giocatore(testa, testa, client_sd);
    }
    else
    {
        struct nodo_giocatore *nuovo_giocatore = crea_giocatore();
        struct nodo_giocatore *tmp = testa;
        while(tmp -> next_node != NULL)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nuovo_giocatore;
        registra_giocatore(testa, nuovo_giocatore, client_sd);
    }
}
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo)
{
    if (nodo != NULL && strcmp(testa -> nome, nodo -> nome )==0) //significa che si sta cercando di cancellare la testa
    {
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

int inizializza_server() //crea la socket, si mette in ascolto e restituisce il socket descriptor
{
    int sd;
    struct sockaddr_in address;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
        perror("errore creazione socket"), exit(EXIT_FAILURE);

    memset(&address, 0, lenght);
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *) &address, lenght)<0)
        perror("errore nel binding"), exit(EXIT_FAILURE);

    if (listen(sd, 5)<0)
        perror("listen error"), exit(EXIT_FAILURE);

    return sd;
}
