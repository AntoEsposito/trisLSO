#ifndef LSO_H
#define LSO_H

#include <stdio.h>
#include <stdlib.h>
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

int inizializza_server(); //socket con protocollo TCP, restituisce socket descriptor del server

void invia_partite(); //invia le informazioni sulle partite al client appena entrato e ogni volta che riceve SIGUSR1 (implementata nel main)

struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore, const int client_sd); //crea nodo e lo aggiunge in testa
bool esiste_giocatore(struct nodo_giocatore *testa, const char *nome_giocatore); //verifica se esiste un nodo con lo stesso nome dato in input
char* verifica_giocatore(struct nodo_giocatore *testa, const int client_sd); //restituisce il nome del giocatore se la registrazione va a buon fine
struct nodo_giocatore* registra_giocatore(struct nodo_giocatore *testa, const int client_sd); //chiama le 3 funzioni sopra per aggiungere un nuovo giocatore in lista
void segnala_nuovo_giocatore(struct nodo_giocatore *testa, const pthread_t tid_mittente); //invia il segnale SIGUSR2 a tutti gli altri thread con giocatore in lobby
void handler_nuovo_giocatore(); //gestisce il segnale SIGUSR2 avvisando tutti i giocatori in lobby dell'entrata di un nuovo giocatore (implementata nel main)
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo);

int cerca_client_sd(struct nodo_giocatore *testa, const pthread_t tid); //prende in input la lista e un tid, restituisce il socket descriptor del client gestito dal tid


#endif
