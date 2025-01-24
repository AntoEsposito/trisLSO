#include "funzioni.h"

void* fun_lettore(void *arg)
{
    const int sd = *((int *)arg);

    char buffer[MAXLETTORE];
    memset(buffer, 0, MAXLETTORE);
    int n_byte = 0;

    while ((n_byte = recv(sd, buffer, MAXLETTORE, 0)) <= 0)
    {
        buffer[n_byte] = '\n';
        printf("%s", buffer);
        memset(buffer, 0, MAXLETTORE);
    }
    close(sd);
    pthread_exit(NULL);
}
void* fun_scrittore(void *arg)
{
    const int sd = *((int *)arg);
    char buffer[MAXSCRITTORE];

    do
    {
        memset(buffer, 0, MAXSCRITTORE);
        //strnlen è più sicura di strlen per stringhe che potrebbero non terminare con \0 come in questo caso
        if (fgets(buffer, MAXSCRITTORE, stdin) != NULL && buffer[strnlen(buffer, MAXSCRITTORE)-1] == '\n') //massimo 15 caratteri nel buffer escluso \n
        {
            if (send(sd, buffer, strlen(buffer)-1, 0) <= 0) break;
        }
        else //sono stati inseriti più di 15 caratteri
        {
            printf("Puoi scrivere al massimo 15 caratteri\n");
            char c;
            while ((c = getchar()) != '\n' && c != EOF); //svuota lo standard input (fflush non funziona)
        }
    } while (strcmp(buffer, "esci\n") != 0);
    close(sd);
    pthread_exit(NULL);
}
int inizializza_socket(const unsigned int porta)
{
    int sd;
    struct sockaddr_in cl_add, ser_add;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket creation error"), exit(EXIT_FAILURE);

    unsigned short int opt = 1;
    //opzione reuseaddr per evitare problemi coi riavvii
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
        perror("set REUSEADDR option error"), exit(EXIT_FAILURE);

    //opzione nodelay per ridurre la latenza
    if (setsockopt(sd, SOL_SOCKET, TCP_NODELAY, &opt, sizeof(opt)) < 0) 
        perror("set NODELAY option error"), exit(EXIT_FAILURE);
    
    struct timeval timer;
    timer.tv_sec = 120;  // Timer di 120 secondi
    timer.tv_usec = 0;

    if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer)) < 0 || setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0)
        perror("set socket timer error"), exit(EXIT_FAILURE);

    memset(&ser_add, 0, sizeof(ser_add));
    ser_add.sin_family = AF_INET;
    ser_add.sin_port = htons(8080);
    ser_add.sin_addr.s_addr = inet_addr("127.0.0.1"); //server locale

    memset(&cl_add, 0, sizeof(cl_add));
    cl_add.sin_family = AF_INET;
    cl_add.sin_port = htons(porta);
    cl_add.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&cl_add, lenght) < 0)
        perror("bind error"), exit(EXIT_FAILURE);

    if (connect(sd, (struct sockaddr *)&ser_add, lenght) < 0)
        perror("connect error"), exit(EXIT_FAILURE);

    return sd;
}