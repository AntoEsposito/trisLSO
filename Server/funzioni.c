#include "funzioni.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni generali server

int inizializza_server() //crea la socket, si mette in ascolto e restituisce il socket descriptor
{
    int sd;
    int opt = 1; //1 = abilita, 0 = disabilita
    struct sockaddr_in address;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket creation error"), exit(EXIT_FAILURE);

    //imposta la socket attivando SO_REUSEADDR che permette di riavviare velocemente il server in caso di crash o riavvii
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    {
        perror("Errore setsockopt");
        close(sd);
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, lenght);
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *) &address, lenght)<0)
        perror("binding error"), exit(EXIT_FAILURE);

    if (listen(sd, 5)<0)
        perror("listen error"), exit(EXIT_FAILURE);

    return sd;
}

void* thread_giocatore(void *sd)
{
    const int sd_giocatore = *((int *)sd); //cast del puntatore void a int e deferenziazione
    struct nodo_giocatore *giocatore = registra_giocatore(sd_giocatore);
    segnala_nuovo_giocatore();

    funzione_lobby(giocatore);

    close(sd_giocatore);   
    cancella_giocatore(giocatore);
    printf("giocatore cancellato\n"); 
    pthread_exit(NULL);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di registrazione

struct nodo_giocatore* registra_giocatore(const int client_sd)
{
    char *nome_giocatore = verifica_giocatore(client_sd);
    struct nodo_giocatore *nodo = crea_giocatore_in_testa(nome_giocatore, client_sd);
    free(nome_giocatore);
    return nodo;
}

char* verifica_giocatore(const int client_sd)
{
    char *nome_giocatore = (char *) malloc(MAXPLAYER*sizeof(char));
    if (nome_giocatore == NULL)
    {
        send(client_sd, "errore\n", 7, MSG_NOSIGNAL);
        close(client_sd);
        printf("errore verifica\n");
        pthread_exit(NULL);
    }

    if (send(client_sd, "Inserisci il tuo nome per registrarti (max 15 caratteri)\n", 57, MSG_NOSIGNAL) < 0) 
    {
        close(client_sd);
        free(nome_giocatore);
        printf("errore verifica\n");
        pthread_exit(NULL);
    }

    bool nome_trovato = false;

    do
    {
        memset(nome_giocatore, 0, MAXPLAYER);
        
        //si occupa il codice del client di verificare che i caratteri inviati siano al massimo 15
        if (recv(client_sd, nome_giocatore, MAXPLAYER, 0) <= 0 )
        {
            close(client_sd);
            free(nome_giocatore);
            printf("errore verifica\n");
            pthread_exit(NULL);
        }

        if(!esiste_giocatore(nome_giocatore)) nome_trovato = true;
        else if (send(client_sd, "Il nome inserito è già utilizzato, prova un altro nome (max 15 caratteri)\n", 76, MSG_NOSIGNAL) < 0)
        {
            close(client_sd);
            free(nome_giocatore);
            printf("errore verifica\n");
            pthread_exit(NULL);
        }
    } while (!nome_trovato);

    if (send(client_sd, "Registrazione completata\n", 25, MSG_NOSIGNAL) < 0)
    {
        close(client_sd);
        free(nome_giocatore);
        printf("errore verifica\n");
        pthread_exit(NULL);
    }

    return nome_giocatore;
}

bool esiste_giocatore(const char *nome_giocatore)
{
    pthread_mutex_lock(&mutex_giocatori);
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        if (strcmp(tmp -> nome, nome_giocatore) == 0) 
        {
            pthread_mutex_unlock(&mutex_giocatori);
            return true;
        }
        tmp = tmp -> next_node;
    }
    pthread_mutex_unlock(&mutex_giocatori); 
    return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di gestione partite

void funzione_lobby(struct nodo_giocatore *dati_giocatore)
{
    char inbuffer[MAXIN]; //contiene le "scelte" del giocatore
    char outbuffer[MAXOUT]; //contiene tutte le statistiche del giocatore formattate in un'unica stringa

    const int sd_giocatore = dati_giocatore -> sd_giocatore;
    bool connesso = true;   
    do
    {
        memset(inbuffer, 0, MAXIN);
        memset(outbuffer, 0, MAXOUT);
        //conversione in stringhe delle statistiche del giocatore
        char vittorie[3]; sprintf(vittorie, "%u", dati_giocatore -> vittorie);
        char sconfitte[3]; sprintf(sconfitte, "%u", dati_giocatore -> sconfitte);
        char pareggi[3]; sprintf(pareggi, "%u", dati_giocatore -> pareggi);

        outbuffer[0] = '\n';
        strcat(outbuffer, dati_giocatore -> nome);
        strcat(outbuffer, "\nvittorie: "); strcat(outbuffer, vittorie);
        strcat(outbuffer, "\nsconfitte: "); strcat(outbuffer, sconfitte);
        strcat(outbuffer, "\npareggi: "); strcat(outbuffer, pareggi);

        if (send(sd_giocatore, outbuffer, strlen(outbuffer), MSG_NOSIGNAL) < 0) error_handler(sd_giocatore);
        invia_partite();
        do //recv può essere interrota da un segnale e restituire EINTR come errore
        { //questo ciclo gestisce l'errore
            if (errno == EINTR) errno = 0;
            if (recv(sd_giocatore, inbuffer, MAXIN, 0) <= 0 && errno != EINTR) error_handler(sd_giocatore);
        } while (errno == EINTR);
        
        int i = 0;
        while (inbuffer[i] != '\0') //case insensitive
        {
            inbuffer[i] = toupper(inbuffer[i]);
            i++;
        }
        if (strcmp(inbuffer, "ESCI") == 0) connesso = false;
        else if (strcmp(inbuffer, "CREA") == 0) 
        {
            struct nodo_partita *nodo_partita = crea_partita_in_testa(dati_giocatore -> nome, sd_giocatore);
            if (nodo_partita != NULL)
            {
                gioca_partita(nodo_partita);
                cancella_partita(nodo_partita);
                printf("partita cancellata\n"); 
            }
            else if (send(sd_giocatore, "Impossibile creare partita, attendi qualche minuto\n", 51, MSG_NOSIGNAL) < 0) error_handler(sd_giocatore);
        }
        else 
        {
            int indice = atoi(inbuffer);
            struct nodo_partita *partita = NULL;
            if (indice != 0) partita = (trova_partita_da_indice(indice));
            if (partita == NULL)
            {
                if (send(sd_giocatore, "Partita non trovata\n", 20, MSG_NOSIGNAL) < 0) error_handler(sd_giocatore);
            }
            else
            {
                dati_giocatore -> stato = IN_RICHIESTA;
                if (!accetta_partita(partita, sd_giocatore, dati_giocatore -> nome))
                {
                    if (partita -> richiesta_unione == false)
                        {if (send(sd_giocatore, "Richiesta di unione rifiutata\n", 30, MSG_NOSIGNAL) < 0) error_handler(sd_giocatore);}
                    else   
                        {if (send(sd_giocatore, "Un altro giocatore ha già richiesto di unirsi a questa partita\n", 64, MSG_NOSIGNAL) < 0) error_handler(sd_giocatore);}
                    dati_giocatore -> stato = IN_LOBBY;
                }
                else
                {
                    dati_giocatore -> stato = IN_PARTITA;
                    pthread_mutex_lock(&(dati_giocatore -> stato_mutex));
                    while (dati_giocatore -> stato != IN_LOBBY)
                    {
                        //controllo periodico in caso il giocatore si disconnetta mentre è in partita
                        pthread_cond_wait(&(dati_giocatore -> stato_cv), &(dati_giocatore -> stato_mutex));
                    }
                    pthread_mutex_unlock(&(dati_giocatore -> stato_mutex));
                }
            }
        }
    } while (connesso);
}

bool accetta_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario)
{
    if (partita -> richiesta_unione == true) return false;
    const int sd_proprietario = partita -> sd_proprietario;
    char buffer[MAXOUT];
    memset(buffer, 0, MAXOUT);
    strcat(buffer, nome_avversario); strcat(buffer, " vuole unirsi alla tua partita, accetti? [s/n]\n");

    char risposta = '\0'; //si occupa il codice client di verificare che l'input sia o "s" o "n"

    if(send(sd_avversario, "In attesa del proprietario...\n", 30, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
    partita -> richiesta_unione = true;

    if(send(sd_proprietario, buffer, strlen(buffer), MSG_NOSIGNAL) < 0) 
    {
        error_handler(sd_proprietario);
        return false;
    }
    if(recv(sd_proprietario, &risposta, 1, 0) <= 0)
    {
        error_handler(sd_proprietario);
        return false;
    }

    risposta = toupper(risposta); //case insensitive
    if(risposta == 'S')
    {
        strcpy(partita -> avversario, nome_avversario);
        partita -> sd_avversario = sd_avversario;
        if (send(sd_proprietario, "Richiesta accettata, inizia la partita!\n", 40, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        if (send(sd_avversario, "Il proprietario ha accettato la richiesta, inizia la partita!\n", 62, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
        if (partita != NULL) partita -> stato = IN_CORSO;
        if (partita != NULL) partita -> richiesta_unione = false;
        if (partita != NULL) pthread_cond_signal(&(partita -> stato_cv));
        return true;
    }
    else
    {
        if (send(sd_proprietario, "Richiesta rifiutata, in attesa di un avversario...\n", 51, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        partita -> richiesta_unione = false;
    }
    return false;
}


void gioca_partita(struct nodo_partita *dati_partita)
{
    const int sd_proprietario = dati_partita -> sd_proprietario;
    struct nodo_giocatore *proprietario = trova_giocatore_da_sd(sd_proprietario);
    proprietario -> stato = IN_PARTITA;
    if (send(sd_proprietario, "Partita creata, in attesa di un avversario...\n", 46, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
    segnala_cambiamento_partite();

    pthread_mutex_lock(&(dati_partita -> stato_mutex));
    while (dati_partita -> stato != IN_CORSO)
    {
        //controllo periodico in caso il proprietario si disconnetta mentre la partita è in attesa
        struct timespec tempo_attesa;
        clock_gettime(CLOCK_REALTIME, &tempo_attesa);
        tempo_attesa.tv_sec += 1;
        tempo_attesa.tv_nsec = 0;
        int result = pthread_cond_timedwait(&(dati_partita -> stato_cv), &(dati_partita -> stato_mutex), &tempo_attesa);
        if (result == ETIMEDOUT) 
        { 
            //messaggio nullo che controlla se la socket è ancora attiva
            if (send(sd_proprietario, "\0", 1, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        }
    }
    pthread_mutex_unlock(&(dati_partita -> stato_mutex));

    const int sd_avversario = dati_partita -> sd_avversario;
    struct nodo_giocatore *avversario = trova_giocatore_da_sd(sd_avversario);

    unsigned int round = 0;

    do
    {
        dati_partita -> stato = IN_CORSO;
        round++;
        segnala_cambiamento_partite();

        bool errore = false; //rappresenta un errore di disconnesione
        char giocata = '\0';
        char esito_proprietario = '0'; //il codice del client cambia il valore di questa variabile quando la partita finisce
        char esito_avversario = '0';
        //'0' = ancora in corso '1' = vince proprietario, '2' = vince avversario, '3' = pareggio

        //inizia la partita
        do
        {   //di default il server invia '0' per segnalare che è tutto ok, altrimenti è l'error handler a mandare '1' all'altro giocatore
            //inoltre; visto che la partita è gestita dal thread proprietario, l'error handler si occupa anche di sbloccare l'avversario
            //in caso fosse il proprietario a disconnettersi
            if (round%2 != 0)
            {
                //inizia il proprietario
                if (recv(sd_proprietario, &giocata, 1, 0) <= 0) {error_handler(sd_proprietario); errore = true; break;}
                if (recv(sd_proprietario, &esito_proprietario, 1, 0) <= 0) {error_handler(sd_proprietario); errore = true; break;}
                if (send(sd_avversario, &NOERROR, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_avversario); errore = true; break;}
                if (send(sd_avversario, &giocata, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_avversario); errore = true; break;}
                if (esito_proprietario != '0') break;

                //turno dell'avversario
                if (recv(sd_avversario, &giocata, 1, 0) <= 0) {error_handler(sd_avversario); errore = true; break;}
                if (recv(sd_avversario, &esito_avversario, 1, 0) <= 0) {error_handler(sd_avversario); errore = true; break;}
                if (send(sd_proprietario, &NOERROR, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_proprietario); errore = true; break;}
                if (send(sd_proprietario, &giocata, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_proprietario); errore = true; break;}
            }
            else 
            {
                //inizia l'avversario
                if (recv(sd_avversario, &giocata, 1, 0) <= 0) {error_handler(sd_avversario); errore = true; break;}
                if (recv(sd_avversario, &esito_avversario, 1, 0) <= 0) {error_handler(sd_avversario); errore = true; break;}
                if (send(sd_proprietario, &NOERROR, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_proprietario); errore = true; break;}
                if (send(sd_proprietario, &giocata, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_proprietario); errore = true; break;}
                if (esito_avversario != '0') break;

                //turno del proprietario
                if (recv(sd_proprietario, &giocata, 1, 0) <= 0) {error_handler(sd_proprietario); errore = true; break;}
                if (recv(sd_proprietario, &esito_proprietario, 1, 0) <= 0) {error_handler(sd_proprietario); errore = true; break;}
                if (send(sd_avversario, &NOERROR, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_avversario); errore = true; break;}
                if (send(sd_avversario, &giocata, 1, MSG_NOSIGNAL) < 0) {error_handler(sd_avversario); errore = true; break;}
            }
        } while (esito_proprietario == '0' && esito_avversario == '0');

        //in caso di errore si aggiornano le vittorie e si esce dalla partita
        if (errore) 
        {
            proprietario -> vittorie++;
            proprietario -> stato = IN_LOBBY;
            break;
        }

        printf("esito p:%c\n", esito_proprietario);
        printf("esito a:%c\n", esito_avversario);
        //si aggiornano i contatori dei giocatori
        if (esito_proprietario == '1' || esito_avversario == '2')
        {
            proprietario -> vittorie++;
            avversario -> sconfitte++;
        }
        else if (esito_proprietario == '2' || esito_avversario == '1')
        {
            proprietario -> sconfitte++;
            avversario -> vittorie++;
        }
        else if (esito_proprietario == '3' || esito_avversario == '3')
        {
            proprietario -> pareggi++;
            avversario -> pareggi++;
        }
        dati_partita -> stato = TERMINATA;
        segnala_cambiamento_partite();

        //partita finita, rimane in stato terminata finchè la rivincita viene accettata o rifiutata
    } while (rivincita(sd_proprietario, sd_avversario));
}

bool rivincita(const int sd_proprietario, const int sd_avversario)
{
    struct nodo_giocatore *proprietario = trova_giocatore_da_sd(sd_proprietario);
    struct nodo_giocatore *avversario = trova_giocatore_da_sd(sd_avversario);
    char risposta_avversario = '\0';
    char risposta_proprietario = '\0';

    if (send(sd_avversario, "Rivincita? [s/n]\n", 17, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
    if (recv(sd_avversario, &risposta_avversario, 1, 0) <= 0) error_handler(sd_avversario);
    
    if (risposta_avversario == 'N') {if (send(sd_proprietario, "Rivincita rifiutata dall'avversario, ritorno in lobby\n", 54, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);}
    else 
    {
        if (send(sd_avversario, "In attesa del proprietario...\n", 30, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
        if (send(sd_proprietario, "L'avversario vuole la rivincita, accetti? [s/n]\n", 48, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        if (recv(sd_proprietario, &risposta_proprietario, 1, 0) <= 0) error_handler(sd_proprietario);
    }
    if (risposta_proprietario != 'S') //si torna alla lobby
    {
        if (risposta_proprietario == 'N')
        {
            if (send(sd_avversario, "Rivincita rifiutata dal proprietario\n", 37, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
            if (send(sd_proprietario, "Ritorno in lobby\n", 17, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        }
        proprietario -> stato = IN_LOBBY;
        if (avversario != NULL)
        {
            avversario -> stato = IN_LOBBY;
            pthread_cond_signal(&(avversario -> stato_cv));
        }
        return false;
    }
    if (risposta_avversario == 'S' && risposta_proprietario == 'S')
    {
        if (send(sd_avversario, "Rivincita accettata, pronti per il prossimo round\n", 50, MSG_NOSIGNAL) < 0) error_handler(sd_avversario);
        if (send(sd_proprietario, "Rivincita accettata, pronti per il prossimo round\n", 50, MSG_NOSIGNAL) < 0) error_handler(sd_proprietario);
        return true;
    }
    return false; //questo return non viene mai raggiunto
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di gestione lista giocatori

struct nodo_giocatore* crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd)
{
    struct nodo_giocatore *nuova_testa = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nuova_testa == NULL)
    {
        send(client_sd, "errore\n", 7, MSG_NOSIGNAL); 
        close(client_sd);
        printf("errore creazione giocatore\n");
        pthread_exit(NULL);
    }

    memset(nuova_testa, 0, sizeof(struct nodo_giocatore)); //pulisce il nodo per sicurezza
    strcpy(nuova_testa -> nome, nome_giocatore);
    nuova_testa -> vittorie = 0;
    nuova_testa -> sconfitte = 0;
    nuova_testa -> pareggi = 0;
    pthread_mutex_init(&(nuova_testa -> stato_mutex), NULL);
    pthread_cond_init(&(nuova_testa -> stato_cv), NULL);
    nuova_testa -> stato = IN_LOBBY;
    nuova_testa -> tid_giocatore = pthread_self();
    nuova_testa -> sd_giocatore = client_sd;
    pthread_mutex_lock(&mutex_giocatori);
    nuova_testa -> next_node = testa_giocatori;

    testa_giocatori = nuova_testa;
    pthread_mutex_unlock(&mutex_giocatori);
    return nuova_testa;
}

struct nodo_giocatore* trova_giocatore_da_sd(const int sd)
{
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        if (tmp -> sd_giocatore == sd) return tmp;
        tmp = tmp -> next_node;
    } 
    return NULL; //improbabile
}

struct nodo_giocatore* trova_giocatore_da_tid(const pthread_t tid)
{
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        if (tmp -> tid_giocatore == tid) return tmp;
        tmp = tmp -> next_node;
    }
    return NULL; //improbabile
}

void cancella_giocatore(struct nodo_giocatore *nodo)
{
    pthread_mutex_lock(&mutex_giocatori);
    if (nodo != NULL && testa_giocatori == nodo) //significa che si sta cercando di cancellare la testa
    {
        testa_giocatori = testa_giocatori -> next_node;
        free(nodo);
    }
    else if (nodo != NULL)
    {
        struct nodo_giocatore *tmp = testa_giocatori;
        while(tmp -> next_node != nodo && tmp != NULL) //in teoria è impossibile che tmp diventi null
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;
        free(nodo);
    }
    pthread_mutex_unlock(&mutex_giocatori);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di gestione lista partite

struct nodo_partita* crea_partita_in_testa(const char *nome_proprietario, const int sd_proprietario)
{
    struct nodo_partita *nuova_testa = (struct nodo_partita *) malloc(sizeof(struct nodo_partita));
    if (nuova_testa == NULL) return NULL;

    memset(nuova_testa, 0, sizeof(struct nodo_partita));
    strcpy(nuova_testa -> proprietario, nome_proprietario);
    nuova_testa -> sd_proprietario = sd_proprietario;
    pthread_mutex_init(&(nuova_testa -> stato_mutex), NULL);
    pthread_cond_init(&(nuova_testa -> stato_cv), NULL);
    nuova_testa -> stato = NUOVA_CREAZIONE;
    nuova_testa -> richiesta_unione = false;
    pthread_mutex_lock(&mutex_partite);
    nuova_testa -> next_node = testa_partite;
    if(testa_partite != NULL && testa_partite -> stato == NUOVA_CREAZIONE) testa_partite -> stato = IN_ATTESA;

    testa_partite = nuova_testa;
    pthread_mutex_unlock(&mutex_partite);
    return nuova_testa;
}

struct nodo_partita* trova_partita_da_sd(const int sd)
{
    struct nodo_partita *tmp = testa_partite;
    while(tmp != NULL)
    {
        if(tmp -> sd_proprietario == sd || tmp -> sd_avversario == sd) return tmp;
        tmp = tmp -> next_node;
    }
    return NULL;
}

struct nodo_partita* trova_partita_da_indice(const unsigned int indice)
{
    unsigned int indice_partita = 0;
    pthread_mutex_lock(&mutex_partite);
    struct nodo_partita *tmp = testa_partite;
    while (tmp != NULL)
    {
        if (tmp -> stato == IN_ATTESA || tmp -> stato == NUOVA_CREAZIONE) indice_partita++;
        if (indice_partita == indice) 
        {
            pthread_mutex_unlock(&mutex_partite);
            return tmp;
        }
        tmp = tmp -> next_node;
    }
    pthread_mutex_unlock(&mutex_partite);
    return NULL;
}

void cancella_partita(struct nodo_partita *nodo)
{
    pthread_mutex_lock(&mutex_partite);
    if (nodo != NULL && testa_partite == nodo) //significa che si sta cercando di cancellare la testa
    {
        testa_partite = testa_partite -> next_node;
        free(nodo);
        segnala_cambiamento_partite();
    }
    else if (nodo != NULL)
    {
        struct nodo_partita *tmp = testa_partite;
        while(tmp != NULL && tmp -> next_node != nodo)
        {
            tmp = tmp -> next_node;
        }
        if (tmp != NULL)
        {
            tmp -> next_node = nodo -> next_node;
            free(nodo);
            segnala_cambiamento_partite();
        }
    }
    pthread_mutex_unlock(&mutex_partite);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di signal handling

void segnala_cambiamento_partite()
{ 
    const pthread_t tid_mittente = pthread_self();
    pthread_t tid_ricevente = 0;
    //per essere sicuri che la testa non cambi nel frattempo
    pthread_mutex_lock(&mutex_giocatori);
    struct nodo_giocatore *tmp = testa_giocatori;
    pthread_mutex_unlock(&mutex_giocatori);

    while (tmp != NULL)
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente && tmp -> stato == IN_LOBBY) pthread_kill(tid_ricevente, SIGUSR1);
        tmp = tmp -> next_node;
    }
}

void invia_partite()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    const int client_sd = giocatore -> sd_giocatore;
    pthread_mutex_lock(&mutex_partite);
    struct nodo_partita *tmp = testa_partite;

    char outbuffer[MAXOUT];
    memset(outbuffer, 0, MAXOUT);

    unsigned int indice = 0; //conta le partite a cui è possibile aggiungersi
    char stringa_indice[3]; //l'indice verrà convertito in questa stringa
    memset(stringa_indice, 0, 3);

    char stato_partita[28];

    if (tmp == NULL) {if (send(client_sd, "\nNon ci sono partite attive al momento, scrivi \"crea\" per crearne una o \"esci\" per uscire\n", 90, MSG_NOSIGNAL) < 0) error_handler(client_sd);}
    else
    {
        while (tmp != NULL)
        {
            memset(outbuffer, 0, MAXOUT);
            memset(stato_partita, 0, 28);
            switch (tmp -> stato)
            {
                case NUOVA_CREAZIONE:
                    strcpy(stato_partita, "Creata da poco\n");
                    break;
                case IN_ATTESA:
                    strcpy(stato_partita, "In attesa di un giocatore\n");
                    break;
                case IN_CORSO:
                    strcpy(stato_partita, "In corso\n");
                    break;
                case TERMINATA:
                    strcpy(stato_partita, "Terminata\n");
                    break;
            }
            strcat(outbuffer, "\n\nLISTA PARTITE");
            strcat(outbuffer, "\nPartita di "); strcat(outbuffer, tmp -> proprietario);
            strcat(outbuffer, "\nAvversario: "); strcat(outbuffer, tmp -> avversario);
            strcat(outbuffer, "\nStato: "); strcat(outbuffer, stato_partita);

            if (tmp -> stato == NUOVA_CREAZIONE || tmp -> stato == IN_ATTESA)
            {
                indice++;
                sprintf(stringa_indice, "%u\n", indice);
                strcat(outbuffer, "ID: "); strcat(outbuffer, stringa_indice);
            }
            if (send(client_sd, outbuffer, strlen(outbuffer), MSG_NOSIGNAL) < 0) error_handler(client_sd);

            tmp = tmp -> next_node;
        }
        if (send(client_sd, "\nUnisciti a una partita in attesa scrivendo il relativo ID, scrivi \"crea\" per crearne una o \"esci\" per uscire\n", 110, MSG_NOSIGNAL) < 0) error_handler(client_sd);
    }
    pthread_mutex_unlock(&mutex_partite);
}


void segnala_nuovo_giocatore()
{
    //per essere sicuri che la testa non cambi nel frattempo
    pthread_mutex_lock(&mutex_giocatori);
    struct nodo_giocatore *tmp = testa_giocatori;
    pthread_mutex_unlock(&mutex_giocatori);

    const pthread_t tid_mittente = pthread_self();
    pthread_t tid_ricevente = 0;

    while (tmp != NULL)
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente && (tmp -> stato == IN_LOBBY || tmp -> stato == IN_RICHIESTA)) pthread_kill(tid_ricevente, SIGUSR2);
        tmp = tmp -> next_node;
    }
}

void handler_nuovo_giocatore()
{
    pthread_mutex_lock(&mutex_giocatori);
    struct nodo_giocatore *tmp = testa_giocatori;
    pthread_mutex_unlock(&mutex_giocatori);

    char messaggio[MAXOUT];
    memset(messaggio, 0, MAXOUT);
    if (tmp != NULL) //controllo teoricamente inutile
    {
        strcat(messaggio, tmp -> nome); 
        strcat(messaggio, " è appena entrato in lobby!\n");

        struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());

        if (send(giocatore -> sd_giocatore, messaggio, strlen(messaggio), MSG_NOSIGNAL) < 0) error_handler(giocatore -> sd_giocatore);
    }
}


void error_handler(const int sd_giocatore)
{
    pthread_t tid = 0;
    struct nodo_giocatore *giocatore = trova_giocatore_da_sd(sd_giocatore);
    if (giocatore != NULL) tid = giocatore -> tid_giocatore;
    struct nodo_partita *partita = trova_partita_da_sd(sd_giocatore);

    if (partita != NULL) 
    {
        if (partita -> stato == IN_CORSO)
        {   //le send non fanno error checking perchè in caso di errore si creerebbe una ricorsione infinita
            if (sd_giocatore == partita -> sd_proprietario) 
            {
                send(partita -> sd_avversario, &ERROR, 1, MSG_NOSIGNAL);
                struct nodo_giocatore *avversario = trova_giocatore_da_sd(partita -> sd_avversario);
                if (avversario != NULL)
                {
                    avversario -> vittorie++;
                    avversario -> stato = IN_LOBBY;
                    pthread_cond_signal(&(avversario -> stato_cv));
                }
            }
            else send(partita -> sd_proprietario, &ERROR, 1, MSG_NOSIGNAL);
        }
        cancella_partita(partita); 
        printf("errore: partita cancellata\n");
    }
    if (giocatore != NULL) pthread_kill(tid, SIGALRM);
}

void sigalrm_handler()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    close(giocatore -> sd_giocatore);
    cancella_giocatore(giocatore);
    printf("errore: giocatore cancellato\n"); 
    pthread_exit(NULL);
}