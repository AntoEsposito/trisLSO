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

struct nodo_giocatore* crea_giocatore();
bool esiste_giocatore(struct nodo_giocatore *testa, const char *giocatore);
unsigned short int registra_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo, const int client_sd); //restituisce 0 in caso di successo e 1 in caso di errore
struct nodo_giocatore* aggiungi_giocatore(struct nodo_giocatore *testa, const int client_sd);
struct nodo_giocatore* cancella_giocatore(struct nodo_giocatore *testa, struct nodo_giocatore *nodo);


#endif
