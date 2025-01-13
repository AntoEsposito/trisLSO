#include "lso.h"

struct nodo_giocatore* crea_giocatore()
{
    struct nodo_giocatore *nodo = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nodo == NULL)
    {
        perror("errore creazione giocatore");
        pthread_exit(NULL);
    }

    memset(nodo, 0, sizeof(struct nodo_giocatore)); //pulisce il nodo per sicurezza
    nodo -> vittorie = 0;
    nodo -> sconfitte = 0;
    nodo -> pareggi = 0;
    nodo -> stato = IN_LOBBY;
    nodo -> tid_giocatore = pthread_self();
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
            if (strcmp(tmp -> nome, giocatore) == 0) return true;
            tmp = tmp -> next_node;
        }
    }
    return false;
}
unsigned short int registra_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo, const int client_sd)
{
    char giocatore[MAXPLAYER];
    memset(giocatore, 0, MAXPLAYER);
    int n_byte;

    if (send(client_sd, "Benvenuto, inserisci il tuo nome per registrarti (max 15 caratteri)\n", 69, 0) <= 0) return 1;

    while(true)
    {
        //si occupa il codice del client di verificare che i caratteri inviati siano al massimo 15
        if ((n_byte = recv(client_sd, giocatore, MAXPLAYER, 0)) <= 0 ) return 1;

        giocatore[n_byte] = '\0';
        if(esiste_giocatore(testa, giocatore) == false) break;

        if (send(client_sd, "Il nome selezionato è già utilizzato\n", 40, 0) <= 0) return 1;
    } 
    //registrazione terminata, il campo nome viene inizializzato
    strcpy(nodo -> nome, giocatore);

    if (send(client_sd, "Registrazione completata, benvenuto\n", 37, 0) <= 0) return 1;

    return 0;
}
struct nodo_giocatore* aggiungi_giocatore(struct nodo_giocatore *testa, const int client_sd)
{
    if (testa == NULL)
    {
        testa = crea_giocatore();
        if (registra_giocatore(testa, testa, client_sd) == 1) 
        {
            close(client_sd);
            free(testa);
            pthread_exit(NULL);
        }
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
        if (registra_giocatore(testa, nuovo_giocatore, client_sd) == 1) 
        {
            cancella_giocatore(testa, nuovo_giocatore);
            pthread_exit(NULL);
        }
    }
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
