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

//si connette alla socket del server, restituisce il sd del client
int inizializza_socket();
//funzione che legge dalla socket
void* thread_fun(void *arg);
//funzione che scrive sulla socket
void* fun_scrittore(void *arg);
//gestisce la richiesta di rivincita, restituisce true se la rivincita è stata accettata
bool rivincita(const int sd, const enum tipo_giocatore tipo);
//funzione che gestisce la partita tra 2 giocatori, incluse eventuali rivincite
void gioca_partite(char *inbuffer, const int sd, const enum tipo_giocatore tipo);
//aggiorna la griglia di gioco e il numero giocate, invia la giocata e l'esito della partita al server, restituisce l'esito
char invia_giocata(unsigned short int *n_giocate, const int sd);
//riceve la giocata dal server, aggiorna la griglia, restituisce l'esito
char ricevi_giocata(unsigned short int *n_giocate, const int sd);
//controlla chi ha vinto e restituisce l'esito
char controllo_esito(const unsigned short int *n_giocate);
//controlla se il giocatore ha inserito un input valido
bool controllo_giocata(const int giocata);
//inserisce O nella griglia in base alla giocata del giocatore
void inserisci_O(const unsigned short int giocata);
//inserisce X nella griglia in base alla giocata dell'avversario
void inserisci_X(const unsigned short int giocata);
//stampa la griglia attuale
void stampa_griglia();
//manda un messaggio di errore e chiude il processo
void error_handler(const int sd);
//uccide il thread quando viene inviato il segnale SIGUSR1
void SIGUSR1_handler();

#endif