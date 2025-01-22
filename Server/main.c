#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;
struct nodo_tid *testa_thread = NULL;

int main()
{
    int sd = inizializza_server();
    int client_sd;
    struct sockaddr_in client_address; //socket address dei client
    socklen_t lenght = sizeof(struct sockaddr_in);

    //i thread sono creati in stato detatched
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

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
        struct nodo_tid *nodo = crea_nodo_tid();
        if (pthread_create(&(nodo->tid), &attr, thread_giocatore, &client_sd) != 0)
        {
            perror("thread creation error");
            cancella_nodo_tid(nodo -> tid);
            continue;
        }
    }
    return 0;
}