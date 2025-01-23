#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

int main()
{
    //NOTA IMPORTANTE: la printf NON funziona senza la presenza di \n
    //le send del server non terminano mai col carattere \n, se ne occupa il codice client
    int sd = inizializza_server();
    int client_sd;
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    //i thread sono creati in stato detatched
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //handler per i segnali che gestiranno i thread
    struct sigaction *sa = (struct sigaction *) malloc(sizeof(struct sigaction)); //visual studio da errore ma compila
    memset(sa, 0, sizeof(struct sigaction));
    sa -> sa_flags = SA_RESTART; //in modo che i segnali non vengano bloccati da eventuali read

    sa -> sa_handler = invia_partite;   
    sigaction(SIGUSR1, sa, NULL);
    sa -> sa_handler = handler_nuovo_giocatore;
    sigaction(SIGUSR2, sa, NULL);
    sa -> sa_handler = sigalrm_handler;
    sigaction(SIGALRM, sa, NULL);

    free(sa);
    //il server può terminare solo inviandogli esplicitamente un segnale che lo termina
    while (true)
    {
        if ((client_sd = accept(sd, (struct sockaddr *) &client_address, &lenght)) < 0)
        {
            perror("accept error\n");
            continue;
        }
        pthread_t tid;
        if (pthread_create(&tid, &attr, thread_giocatore, &client_sd) != 0)
        {
            perror("thread creation error\n");
            continue;
        }
    }
    return 0;
}