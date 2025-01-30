#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

//inizializzazione dei mutex
pthread_mutex_t mutex_partite = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_giocatori = PTHREAD_MUTEX_INITIALIZER;

//costanti di errore
const char NOERROR = '0';
const char ERROR = '1';


int main()
{
    int sd = inizializza_server();
    printf("Server running on port 8080\n");
    
    const int opt = 1;
    int client_sd;
    struct sockaddr_in client_address;
    socklen_t lenght = sizeof(struct sockaddr_in);

    //i thread sono creati in stato detatched
    pthread_attr_t attr;    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //handler per i segnali che gestiranno i thread, impostati col flag SA_RESTART per evitare che le read li blocchino
    struct sigaction *sa = (struct sigaction *) malloc(sizeof(struct sigaction));
    memset(sa, 0, sizeof(struct sigaction));
    sa -> sa_flags = SA_RESTART;

    sa -> sa_handler = invia_partite;   
    sigaction(SIGUSR1, sa, NULL);
    sa -> sa_handler = handler_nuovo_giocatore;
    sigaction(SIGUSR2, sa, NULL);
    signal(SIGALRM, sigalrm_handler); //non c'è bisgono del flag restart
    free(sa);
    //il server può terminare solo inviandogli esplicitamente un segnale che lo termina
    while (true)
    {
        if ((client_sd = accept(sd, (struct sockaddr *) &client_address, &lenght)) < 0)
            { perror("accept error\n"); continue; }
        //disattiva il buffering dei dati (algoritmo di Nagle)
        if (setsockopt(client_sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
            { perror("set tcpnodelay error"); continue; }

        pthread_t tid;
        if (pthread_create(&tid, &attr, thread_giocatore, &client_sd) != 0)
        {
            perror("thread creation error\n");
            continue;
        }
    }
    return 0;
}