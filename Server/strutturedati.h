#ifndef STRUTTUREDATI_H
#define STRUTTUREDATI_H

#include <pthread.h>

#define MAXPLAYER 16 //il nome di un player può essere lungo massimo 15 caratteri
#define MAXOUT 128 //dimensione del buffer che usa il server per mandare messaggi

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

struct nodo_giocatore
{
    char nome[MAXPLAYER];
    enum stato_giocatore stato;
    unsigned int vittorie;
    unsigned int sconfitte;
    unsigned int pareggi;
    pthread_t tid_giocatore;
    struct nodo_giocatore *next_node;
};
struct nodo_partita
{
    char proprietario[MAXPLAYER];
    char avversario[MAXPLAYER];
    enum stato_partita stato;
    pthread_t tid_partita;
    struct nodo_partita *next_node;
};


#endif