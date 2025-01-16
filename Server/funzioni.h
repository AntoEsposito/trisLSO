#ifndef LSO_H
#define LSO_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include "strutturedati.h"

//crea socket con protocollo TCP, restituisce socket descriptor del server
int inizializza_server();
//invia le informazioni sulle partite al client appena entrato e ogni volta che riceve SIGUSR1 (implementata nel main)
void invia_partite();
//crea un nodo giocatore e lo aggiunge in testa
struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore, const int client_sd);
//verifica se esiste un nodo con lo stesso nome dato in input
bool esiste_giocatore(struct nodo_giocatore *testa, const char *nome_giocatore);
//restituisce il nome del giocatore se la registrazione va a buon fine
char* verifica_giocatore(struct nodo_giocatore *testa, const int client_sd);
//aggiunge un nuovo giocatore in lista
struct nodo_giocatore* registra_giocatore(struct nodo_giocatore *testa, const int client_sd);
//invia il segnale SIGUSR2 a tutti gli altri thread con giocatore in lobby
void segnala_nuovo_giocatore(struct nodo_giocatore *testa, const pthread_t tid_mittente);
//gestisce il segnale SIGUSR2 avvisando tutti i giocatori in lobby dell'entrata di un nuovo giocatore (implementata nel main)
void handler_nuovo_giocatore();
//elimina un nodo dalla lista e restituisce la nuova testa
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo);
//prende in input la lista e un tid, restituisce il socket descriptor del client gestito dal tid
int cerca_sd_giocatore(struct nodo_giocatore *testa, const pthread_t tid);
//crea un nodo partita e lo mette in testa alla lista
struct nodo_partita* crea_partita_in_testa(struct nodo_partita *testa, const char *nome_proprietario, const int id_proprietario);
//se il proprietario accetta la richiesta di unione alla partita inserisce i dati dell'avversario nel nodo partita e restituisce vero, falso altrimenti
bool unione_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario);


#endif
