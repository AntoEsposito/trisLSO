#include "funzioni.h"

int main()
{
    int sd, client_sd; //socket descriptor del server e dei client
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    sd = inizializza_server();
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