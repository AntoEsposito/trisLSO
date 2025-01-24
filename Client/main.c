#include "funzioni.h"

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("specificare porta del client");
        exit(EXIT_FAILURE);
    }
    pthread_t tid_lettore;
    pthread_t tid_scrittore;

    const unsigned int porta = atoi(argv[1]);
    int sd = inizializza_socket(porta);

    if (pthread_create(&tid_lettore, NULL, fun_lettore, &sd) < 0)
        perror("thread creation error"), exit(EXIT_FAILURE);

    if (pthread_create(&tid_scrittore, NULL, fun_scrittore, &sd) < 0)
        perror("thread creation error"), exit(EXIT_FAILURE);

    pthread_join(tid_lettore, NULL);
    pthread_join(tid_scrittore, NULL);

    printf("Uscita\n");
    return 0;
}
