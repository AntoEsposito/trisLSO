#include "funzioni.h"

//inizializzazione delle liste
struct nodo_partita *testa_partite = NULL;
struct nodo_giocatore *testa_giocatori = NULL;

//inizializzazione dei mutex
pthread_mutex_t mutex_partite = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_giocatori = PTHREAD_MUTEX_INITIALIZER;


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

    struct timeval timer; //rappresenta il tempo massimo dopo il quale le socket vengono considerate inattive e quindi automaticamente chiuse
    timer.tv_sec = 120; //secondi, lo abbasseremo dopo il testing
    timer.tv_usec = 0; //millisecondi

    //handler per i segnali che gestiranno i thread, impostati col flag SA_RESTART per evitare che le read li blocchino
    struct sigaction *sa = (struct sigaction *) malloc(sizeof(struct sigaction));
    memset(sa, 0, sizeof(struct sigaction));
    sa -> sa_flags = SA_RESTART;

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
            { perror("accept error\n"); continue; }
        //timer per ricezione
        if (setsockopt(client_sd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0)
            { perror("set rcv timer error"); continue; }
        //timer per invio
        if (setsockopt(client_sd, SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer)) < 0)
            { perror("set send timer error"); continue; }
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