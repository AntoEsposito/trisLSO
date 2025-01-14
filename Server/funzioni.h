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

struct nodo_partita* crea_partita();
struct nodo_partita* aggiungi_partita(struct nodo_partita *testa, struct nodo_partita *nodo);
struct nodo_partita* cancella_partita(struct nodo_partita *testa, struct nodo_partita *nodo);
unsigned short int invia_partite(struct nodo_partita *testa, const int client_sd); //invia le informazioni sulle partite al client

struct nodo_giocatore* crea_giocatore_in_testa(struct nodo_giocatore *testa, const char *nome_giocatore); 
bool esiste_giocatore(struct nodo_giocatore *testa, const char *giocatore);
char* verifica_giocatore(struct nodo_giocatore *testa, const int client_sd); //restituisce il nome del giocatore se la registrazione va a buon fine
struct nodo_giocatore* aggiungi_giocatore(struct nodo_giocatore *testa, const int client_sd);
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo);


#endif
