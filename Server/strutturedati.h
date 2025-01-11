#ifndef STRUTTUREDATI_H
#define STRUTTUREDATI_H

#include <pthread.h>

#define MAXPLAYER 32 //il nome di un player può essere lungo massimo 31 caratteri

enum stato_partita
{
    NUOVA_CREAZIONE,
    IN_ATTESA,
    IN_CORSO,
    TERMINATA
};
enum stato_giocatore
{
    IN_LOBBY,
    IN_PARTITA
};

struct giocatore
{
    char nome[MAXPLAYER];
    enum stato_giocatore stato;
    unsigned int vittorie;
    unsigned int sconfitte;
    unsigned int pareggi;
    pthread_t tid_giocatore;
    struct giocatore *next_node;
};
struct partita
{
    char proprietario[MAXPLAYER];
    enum stato_partita stato;
    char giocatori[2][MAXPLAYER]; //array che contiene 2 stringhe giocatori
    pthread_t tid_partita;
    struct partita *next_node;
};


#endif