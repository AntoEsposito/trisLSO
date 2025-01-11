#include "lso.h"

int inizializza_server()
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