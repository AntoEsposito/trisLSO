#ifndef STRUTTUREDATI_H
#define STRUTTUREDATI_H

#include <pthread.h>

//dimensioni buffer per leggere e scrivere sulla socket
#define MAXLETTORE 512
#define MAXSCRITTORE 16

//serve alla funzione partita per gestire gli input
enum tipo_giocatore
{
    PROPRIETARIO,
    AVVERSARIO
};

extern char griglia[3][3];
extern int sd;

extern const char NOERROR;
extern const char ERROR;

extern const char NESSUNO;
extern const char VITTORIA;
extern const char SCONFITTA;
extern const char PAREGGIO;

#endif