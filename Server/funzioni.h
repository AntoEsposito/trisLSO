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

unsigned short int invia_partite(struct nodo_partita *testa, const int client_sd); //invia le informazioni sulle partite al client

struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore); //crea nodo e lo aggiunge in testa
bool esiste_giocatore(struct nodo_giocatore *testa, const char *nome_giocatore); //verifica se esiste un nodo con lo stesso nome dato in input
char* verifica_giocatore(struct nodo_giocatore *testa, const int client_sd); //restituisce il nome del giocatore se la registrazione va a buon fine
struct nodo_giocatore* registra_giocatore(struct nodo_giocatore *testa, const int client_sd); //chiama le 3 funzioni sopra per aggiungere un nuovo giocatore in lista
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo);


#endif
