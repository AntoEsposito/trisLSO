#include "funzioni.h"

char griglia[3][3] = {{0,0,0},{0,0,0},{0,0,0}};

int sd = 0;

//costanti per la disconnessione dalla partita
const char NOERROR = '0';
const char ERROR = '1';

//costanti di esito partita
const char NESSUNO = '0';
const char VITTORIA = '1';
const char SCONFITTA = '2';
const char PAREGGIO = '3';

int main()
{
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGTERM, SIGTERM_handler);

    inizializza_socket();

    funzione_lobby();
    return 0;
}
