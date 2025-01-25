#include "funzioni.h"

char griglia[3][3] = {{0,0,0},{0,0,0},{0,0,0}};

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("specificare porta del client");
        exit(EXIT_FAILURE);
    }
    signal(SIGUSR1, SIGUSR1_handler);
    pthread_t tid;

    const unsigned int porta = atoi(argv[1]);
    int sd = inizializza_socket(porta);

    if (pthread_create(&tid, NULL, thread_fun, &sd) < 0)
        perror("thread creation error"), exit(EXIT_FAILURE);

    pthread_join(tid, NULL);

    if (errno = ETIMEDOUT) printf("Disconnesso per inattività\n");
    else if (errno = ECONNRESET) printf("Si è verificato un errore di rete\n");
    else printf("Uscita\n");
    return 0;
}
