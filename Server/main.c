#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

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

    pthread_t tid_array[MAXTHREAD];
    unsigned short int indice_tid = 0;
    //il server può terminare solo inviandogli esplicitamente un segnale che lo termina
    while (true)
    {
        if ((client_sd = accept(sd, (struct sockaddr *) &client_address, &lenght)) < 0)
        {
            perror("accept error");
            continue;
        }
        if (pthread_create(&tid_array[indice_tid], &attr, thread_giocatore, &client_sd) != 0)
        {
            perror("thread creation error");
            continue;
        }
        indice_tid++;
    }
    return 0;
}