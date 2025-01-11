#include "lso.h"

//liste che saranno gestite dai thread
const struct nodo_partita *lista_partite; 
const struct nodo_giocatore *lista_giocatori;
//la testa delle liste è allocata staticamente, rimane sempre invariata e non viene cancellata per tutto il processo

int main()
{
    lista_partite = inizializza_partite();
    lista_giocatori = inizializza_giocatori();

    int sd, client_sd; //socket descriptor del server e dei client
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    sd = inizializza_server(); //il server gira sulla porta 8080
}