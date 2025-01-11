#include "lso.h"

int main()
{
    int sd, client_sd; //socket descriptor del server e dei client
    struct sockaddr_in client_address; //struttura socket dei client
    socklen_t lenght = sizeof(struct sockaddr_in);
    sd = inizializza_server(); //il server gira sulla porta 8080
}