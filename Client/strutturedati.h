#ifndef STRUTTUREDATI_H
#define STRUTTUREDATI_H

#include <pthread.h>

//dimensioni buffer per leggere e scrivere sulla socket
#define MAXLETTORE 256
#define MAXSCRITTORE 16

//serve alla funzione partita per gestire gli input
enum tipo_giocatore
{
    PROPRIETARIO,
    AVVERSARIO
};

extern char griglia[3][3];

#endif