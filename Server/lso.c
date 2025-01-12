#include "lso.h"

struct nodo_partita* crea_partita()
{
    struct nodo_partita *nodo = (struct nodo_partita *) malloc(sizeof(struct nodo_partita));
    if (nodo == NULL)
    {
        perror("errore creazione partita");
        return NULL;
    }
    memset(nodo, 0, sizeof(struct nodo_partita)); //pulisce la struct per sicurezza
    nodo -> next_node = NULL;
    return nodo;
}
struct nodo_partita* aggiungi_partita(struct nodo_partita *testa, struct nodo_partita *nodo)
{
    if (nodo != NULL)
    {
        struct nodo_partita *tmp = testa;
        while (tmp -> next_node != NULL) 
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo;
    }
    return testa;
}
struct nodo_partita* cancella_partita(struct nodo_partita *testa, struct nodo_partita *nodo)
{
    if (testa != NULL && nodo != NULL)
    {
        struct nodo_partita *tmp = testa;
        while (tmp -> next_node != nodo)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;

        free(nodo);
    }
    return testa;
}


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
void aggiungi_giocatore(struct nodo_giocatore *testa, const int client_sd)
{
    if (testa == NULL)
    {
        registra_giocatore(testa, client_sd);
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
        registra_giocatore(nuovo_giocatore, client_sd);
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
