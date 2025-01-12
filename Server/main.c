#include "lso.h"

//liste che saranno gestite dai thread
const struct nodo_partita *testa_partite = NULL;
const struct nodo_giocatore *testa_giocatori = NULL;
//il puntatore alla testa delle liste è allocato staticamente

int main()
{
    int sd, client_sd; //socket descriptor del server e dei client
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    sd = inizializza_server(); //il server gira sulla porta 8080
}