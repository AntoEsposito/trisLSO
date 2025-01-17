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
    int sd_giocatore;
    struct nodo_giocatore *next_node;
};
struct nodo_partita
{
    char proprietario[MAXPLAYER];
    int sd_proprietario;
    char avversario[MAXPLAYER];
    int sd_avversario; 
    enum stato_partita stato;
    struct nodo_partita *next_node;
};

//liste che saranno gestite dai thread (inizializzate in funzioni.c)
extern struct nodo_partita *testa_partite;
extern struct nodo_giocatore *testa_giocatori;

#endif