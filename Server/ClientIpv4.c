#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define INMAX 1024
#define OUTMAX 1024


int main(int argc, char *argv[])
{
    if(argc!=3)
    {
        printf("specificare porta del client e indirizzo ip del server\n");
        return 1;
    }
    int sd;
    struct sockaddr_in cl_add, ser_add;
    int porta = atoi(argv[1]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
        perror("socket creation"), exit(1);

    memset(&ser_add, 0, sizeof(ser_add));
    ser_add.sin_family = AF_INET;
    ser_add.sin_port = htons(8080);
    ser_add.sin_addr.s_addr = inet_addr(argv[2]); //server locale

    memset(&cl_add, 0, sizeof(cl_add));
    cl_add.sin_family = AF_INET;
    cl_add.sin_port = htons(porta);
    cl_add.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&cl_add, sizeof(cl_add))<0)
        perror("bind error"), exit(1);

    if (connect(sd, (struct sockaddr *) &ser_add, sizeof(ser_add))<0)
        perror("connect error"), exit(1);

    char messaggio[OUTMAX];
    char inbuff[INMAX];
    memset(messaggio, 0, OUTMAX);
    while(1)
    {
        recv(sd, inbuff, INMAX, 0);
        printf("%s\n", inbuff);
        fgets(messaggio, OUTMAX, stdin);

        send(sd, messaggio, strlen(messaggio), 0);
        if (strcmp(messaggio, "chiudi\n")==0) break;
        recv(sd, inbuff, INMAX, 0);
        printf("%s\n", inbuff);
        memset(messaggio, 0, OUTMAX);
    }
    close(sd);
    return 0;
}