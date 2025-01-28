#include "funzioni.h"

char griglia[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
int sd = 0;

int main()
{
    struct sigaction *sa = (struct sigaction *) malloc(sizeof(struct sigaction));
    memset(sa, 0, sizeof(struct sigaction));
    sa -> sa_flags = SA_RESTART;

    sa -> sa_handler = SIGUSR1_handler; 
    sigaction(SIGUSR1, sa, NULL);
    sa -> sa_handler = SIGINT_handler;
    sigaction(SIGINT, sa, NULL);
    free(sa);

    pthread_t tid;

    inizializza_socket();

    if (pthread_create(&tid, NULL, thread_fun, NULL) < 0)
        perror("thread creation error"), exit(EXIT_FAILURE);

    pthread_join(tid, NULL);

    if (errno == ETIMEDOUT) printf("Disconnesso per inattività\n");
    else if (errno == ECONNRESET) printf("Si è verificato un errore di rete\n");
    else printf("Uscita\n");
    return 0;
}
