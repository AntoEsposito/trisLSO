#ifndef FUNZIONI_H
#define FUNZIONI_H

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

//funzioni di gestione giocatori
//crea un nodo giocatore e lo aggiunge in testa
void crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd);
//verifica se esiste un nodo con lo stesso nome dato in input
bool esiste_giocatore(const char *nome_giocatore);
//restituisce il nome del giocatore se la registrazione va a buon fine
char* verifica_giocatore(const int client_sd);
//aggiunge un nuovo giocatore in lista
void registra_giocatore(const int client_sd);
//elimina un nodo giocatore dalla lista e restituisce la nuova testa
void cancella_giocatore(struct nodo_giocatore *nodo);
//invia il segnale SIGUSR2 a tutti gli altri thread con giocatore in lobby
void segnala_nuovo_giocatore(const pthread_t tid_mittente);
//prende in input la lista e un tid, restituisce il socket descriptor del client gestito dal tid
int cerca_sd_giocatore(const pthread_t tid);
//funzioni di gestione partite
//crea un nodo partita e lo mette in testa alla lista
void crea_partita_in_testa(const char *nome_proprietario, const int id_proprietario);
//se il proprietario accetta la richiesta di unione alla partita inserisce i dati dell'avversario nel nodo partita e restituisce vero, falso altrimenti
bool unione_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario);
//elimina un nodo partita dalla lista e restituisce la nuova testa
void cancella_partita(struct nodo_partita *nodo);
//funzioni generali server
//crea socket con protocollo TCP, si mette in ascolto sulla porta 8080 e restituisce socket descriptor del server
int inizializza_server();
//funzioni di signal handling
//invia le informazioni sulle partite al client appena entrato e ogni volta che riceve SIGUSR1
void invia_partite();
//gestisce il segnale SIGUSR2 avvisando tutti i giocatori in lobby dell'entrata di un nuovo giocatore
void handler_nuovo_giocatore();

#endif
