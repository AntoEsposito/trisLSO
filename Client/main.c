#include "funzioni.h"

char griglia[3][3] = {{0,0,0},{0,0,0},{0,0,0}};

int sd = 0;

const char NOERROR = '0';
const char ERROR = '1';

int main()
{
    signal(SIGUSR1, SIGUSR1_handler);
    pthread_t tid;

    inizializza_socket();

    if (pthread_create(&tid, NULL, thread_fun, NULL) < 0)
        perror("thread creation error"), exit(EXIT_FAILURE);

    pthread_join(tid, NULL);
    if (errno != 0) perror("Si è verificato un errore");
    else printf("Uscita\n");
    return 0;
}
