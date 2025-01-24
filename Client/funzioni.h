#ifndef FUNZIONI_H
#define FUNZIONI_H

#define MAXLETTORE 256
#define MAXSCRITTORE 16

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

//si connette alla socket del server, restituisce il sd del client
int inizializza_socket(unsigned const int porta);
//funzione che legge dalla socket
void* fun_lettore(void *arg);
//funzione che scrive sulla socket
void* fun_scrittore(void *arg);

#endif