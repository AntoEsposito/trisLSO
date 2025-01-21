#include "funzioni.h"

int main()
{
    int sd = inizializza_server();
    int client_sd;
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    //variabili per creare i thread
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //i thread sono creati in stato detatched

    //handler per i segnali che gestiranno i thread
    signal(SIGUSR1, invia_partite);
    signal(SIGUSR2, handler_nuovo_giocatore);
    signal(SIGALRM, sigalrm_handler);

    //il server può terminare solo inviandogli esplicitamente un segnale che lo termina
    while (true)
    {
        if ((client_sd = accept(sd, (struct sockaddr *) &client_address, &lenght)) < 0)
        {
            perror("accept error");
            continue;
        }
        if (pthread_create(&tid, &attr, thread_giocatore, &client_sd) != 0)
        {
            perror("thread creation error");
            continue;
        }
    }
    return 0;
}