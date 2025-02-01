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
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include "strutturedati.h"


/////////////////funzioni generali server

//crea socket con protocollo TCP, si mette in ascolto sulla porta 8080 e restituisce socket descriptor del server
int inizializza_server();
//start function del thread che gestisce un client
void* thread_giocatore(void *sd_giocatore);


/////////////////funzioni di registrazione

//aggiunge un nuovo giocatore in lista
struct nodo_giocatore* registra_giocatore(const int client_sd);
//restituisce il nome del giocatore se la registrazione va a buon fine
char* verifica_giocatore(const int client_sd);
//verifica se esiste un nodo con lo stesso nome dato in input
bool esiste_giocatore(const char *nome_giocatore);


/////////////////funzioni di gestione partite

//funzione che gestisce il ciclo di vita di un giocatore
void funzione_lobby(struct nodo_giocatore *dati_giocatore);
//se il proprietario accetta la richiesta di unione alla partita inserisce i dati dell'avversario nel nodo partita e restituisce vero, falso altrimenti
bool accetta_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario);

//funzione che gestisce la partita tra 2 giocatori
void gioca_partita(struct nodo_partita *dati_partita);
//restituisce true se entrambi i giocatori decidono di giocare una rivincita
bool rivincita(const int sd_proprietario, const int sd_avversario);


/////////////////funzioni di gestione lista giocatori

//crea un nodo giocatore e lo aggiunge in testa
struct nodo_giocatore* crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd);
//restituisce il nodo del giocatore dal sd dato in input
struct nodo_giocatore* trova_giocatore_da_sd(const int sd);
//restituisce il nodo del giocatore gestito dal tid dato in input
struct nodo_giocatore* trova_giocatore_da_tid(const pthread_t tid);
//elimina un nodo giocatore dalla lista
void cancella_giocatore(struct nodo_giocatore *nodo);

/////////////////funzioni di gestione lista partite

//crea un nodo partita e lo mette in testa alla lista
struct nodo_partita* crea_partita_in_testa(const char *nome_proprietario, const int id_proprietario);
//restituisce la partita giocata dal giocatore se esiste, NULL altrimenti
struct nodo_partita* trova_partita_da_sd(const int sd);
//restituisce la partita in attesa con l'id in input
struct nodo_partita* trova_partita_da_indice(const unsigned int indice);
//elimina un nodo partita dalla lista
void cancella_partita(struct nodo_partita *nodo);


/////////////////funzioni di signal handling

//invia SIGUSR1 a tutti i thread con giocatori in lobby
void segnala_cambiamento_partite();
//invia le informazioni sulle partite al client appena entrato e ogni volta che riceve SIGUSR1
void invia_partite();

//invia il segnale SIGUSR2 a tutti gli altri thread con giocatore in lobby
void segnala_nuovo_giocatore();
//gestisce il segnale SIGUSR2 avvisando tutti i giocatori in lobby dell'entrata di un nuovo giocatore
void handler_nuovo_giocatore();

//gestisce gli errori di rete eliminando il nodo del giocatore che ha causato l'errore, l'eventuale nodo partita e manda SIGALRM al relativo thread
void error_handler(const int sd_giocatore);
//gestisce il segnale SIGALRM facendo chiudere al thread la sua socket e chiamando pthread_exit()
void sigalrm_handler();
//gestisce SIGTERM inviato da docker quando stoppa i container
void SIGTERM_handler();


#endif
