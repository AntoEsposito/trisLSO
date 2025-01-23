#include "funzioni.h"

//funzioni di gestione giocatori

struct nodo_giocatore* crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd)
{
    struct nodo_giocatore *nuova_testa = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nuova_testa == NULL)
    {
        send(client_sd, "errore", 6, 0); 
        close(client_sd);
        cancella_nodo_tid(pthread_self());
        cancella_nodo_tid(pthread_self());
        pthread_exit(NULL);
    }

    memset(nuova_testa, 0, sizeof(struct nodo_giocatore)); //pulisce il nodo per sicurezza
    strcpy(nuova_testa -> nome, nome_giocatore);
    nuova_testa -> vittorie = 0;
    nuova_testa -> sconfitte = 0;
    nuova_testa -> pareggi = 0;
    nuova_testa -> stato = IN_LOBBY;
    nuova_testa -> tid_giocatore = pthread_self();
    nuova_testa -> sd_giocatore = client_sd;
    nuova_testa -> next_node = testa_giocatori;

    testa_giocatori = nuova_testa;
    return testa_giocatori;
}
bool esiste_giocatore(const char *nome_giocatore)
{
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        if (strcmp(tmp -> nome, nome_giocatore) == 0) return true;
        tmp = tmp -> next_node;
    } 
    return false;
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
char* verifica_giocatore(const int client_sd)
{
    char *giocatore = (char *) malloc(MAXPLAYER*sizeof(char));
    if (giocatore == NULL)
    {
        send(client_sd, "errore", 6, 0);
        close(client_sd);
        cancella_nodo_tid(pthread_self());
        pthread_exit(NULL);
    }

    memset(giocatore, 0, MAXPLAYER);

    while (true)
    {
        if (send(client_sd, "Inserisci il tuo nome per registrarti (max 15 caratteri)", 56, 0) <= 0) 
        {
            close(client_sd);
            free(giocatore);
            cancella_nodo_tid(pthread_self());
            pthread_exit(NULL);
        }
        //si occupa il codice del client di verificare che i caratteri inviati siano al massimo 15
        if (recv(client_sd, giocatore, MAXPLAYER, 0) <= 0 )
        {
            close(client_sd);
            free(giocatore);
            cancella_nodo_tid(pthread_self());
            pthread_exit(NULL);
        }

        if(!(esiste_giocatore(giocatore))) break;
        memset(giocatore, 0, MAXPLAYER);

        if (send(client_sd, "Il nome selezionato è già utilizzato", 38, 0) <= 0)
        {
            close(client_sd);
            free(giocatore);
            cancella_nodo_tid(pthread_self());
            pthread_exit(NULL);
        }
    }

    if (send(client_sd, "Registrazione completata", 24, 0) <= 0)
    {
        close(client_sd);
        free(giocatore);
        cancella_nodo_tid(pthread_self());
        pthread_exit(NULL);
    }

    return giocatore;
}
struct nodo_giocatore* registra_giocatore(const int client_sd)
{
    char *nome_giocatore = verifica_giocatore(client_sd);
    struct nodo_giocatore *nodo = crea_giocatore_in_testa(nome_giocatore, client_sd);
    free(nome_giocatore);
    return nodo;
}
void cancella_giocatore(struct nodo_giocatore *nodo)
{
    if (nodo != NULL && testa_giocatori == nodo) //significa che si sta cercando di cancellare la testa
    {
        free(nodo);
        testa_giocatori = testa_giocatori -> next_node;
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
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di gestione partite

struct nodo_partita* crea_partita_in_testa(const char *nome_proprietario, const int sd_proprietario)
{
    struct nodo_partita *nuova_testa = (struct nodo_partita *) malloc(sizeof(struct nodo_partita));
    if (nuova_testa == NULL) return NULL;

    memset(nuova_testa, 0, sizeof(struct nodo_partita));
    strcpy(nuova_testa -> proprietario, nome_proprietario);
    nuova_testa -> sd_proprietario = sd_proprietario;
    nuova_testa -> stato = NUOVA_CREAZIONE;
    nuova_testa -> next_node = testa_partite;
    if(testa_partite != NULL && testa_partite -> stato == NUOVA_CREAZIONE) testa_partite -> stato = IN_ATTESA;

    testa_partite = nuova_testa;
    return testa_partite;
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
    struct nodo_partita *tmp = testa_partite;
    while (tmp != NULL)
    {
        if (tmp -> stato == IN_ATTESA || tmp -> stato == NUOVA_CREAZIONE) indice_partita++;
        if (indice_partita == indice) return tmp;
    }
    return NULL;
}
bool unione_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario)
{
    const int sd_proprietario = partita -> sd_proprietario;
    char buffer[MAXOUT];
    memset(buffer, 0, MAXOUT);
    strcat(buffer, nome_avversario); strcat(buffer, " vuole unirsi alla tua partita, accetti? [s/n]");

    char risposta = '\0'; //si occupa il codice client di verificare che l'input sia o "s" o "n"

    if(send(sd_avversario, "In attesa del proprietario...", 29, 0) <= 0) error_handler(sd_avversario);

    if(send(sd_proprietario, buffer, strlen(buffer), 0) <= 0) 
    {
        error_handler(partita -> sd_proprietario);
        return false;
    }
    if(recv(sd_proprietario, &risposta, 1, 0) <= 0)
    {
        error_handler(partita -> sd_proprietario);
        return false;
    }

    risposta = toupper(risposta); //case insensitive
    if(risposta == 'S')
    {
        strcpy(partita -> avversario, nome_avversario);
        partita -> sd_avversario = sd_avversario;
        if (send(sd_proprietario, "Richiesta accettata, inizia la partita", 38, 0) <= 0) error_handler(partita -> sd_proprietario);
        partita -> stato = IN_CORSO; //sblocca la funziona gioca_partita in busy wait
        return true;
    }
    return false;
}
void gioca_partita(struct nodo_partita *dati_partita)
{
    char buffer[MAXPARTITA];
    memset(buffer, 0, MAXPARTITA);

    const int sd_proprietario = dati_partita -> sd_proprietario;
    char nome_proprietario[MAXPLAYER];
    memset(nome_proprietario, 0, MAXPLAYER);
    strcpy(nome_proprietario, dati_partita -> proprietario);

    struct nodo_giocatore *proprietario = trova_giocatore_da_sd(sd_proprietario);
    proprietario -> stato = IN_PARTITA;
    if (send(sd_proprietario, "Partita creata, in attesa di un avversario...", 45, 0) <= 0) error_handler(sd_proprietario);
    segnala_cambiamento_partite();

    //il proprietario esce da questo ciclo quando la funzione di unione cambia lo stato della partita
    while (dati_partita -> stato != IN_CORSO) {} //busy wait DA TESTARE

    const int sd_avversario = dati_partita -> sd_avversario;
    char nome_avversario[MAXPLAYER];
    memset(nome_avversario, 0, MAXPLAYER);
    strcpy(nome_avversario, dati_partita -> avversario);

    struct nodo_giocatore *avversario = trova_giocatore_da_sd(sd_avversario);
    avversario -> stato = IN_PARTITA;

    while (true) //si esce da questo ciclo quando la richiesta di rivincita viene rifiutata
    {
        segnala_cambiamento_partite();

        char giocata = '\0';
        char esito = '0'; //il codice del client cambia il valore di questa variabile quando la partita finisce
        //'0' = ancora in corso '1' = vince proprietario, '2' = vince avversario, '3' = pareggio

        //inizia la partita
        while (esito == '0') //non è il terminatore \0 ma il carattere 0
        {
            //inizia il proprietario
            if (send(sd_proprietario, "Tocca a te", 10, 0) <= 0) error_handler(sd_proprietario);
            if (recv(sd_proprietario, &giocata, 1, 0) <= 0) error_handler(sd_proprietario);
            if (recv(sd_proprietario, &esito, 1, 0) <= 0) error_handler(sd_proprietario);
            if (send(sd_avversario, &giocata, 1, 0) <= 0) error_handler(sd_avversario);
            if (esito != '0') break;

            //turno dell'avversario
            if (send(sd_avversario, "Tocca a te", 10, 0) <= 0) error_handler(sd_avversario);
            if (recv(sd_avversario, &giocata, 1, 0) <= 0) error_handler(sd_avversario);
            if (recv(sd_avversario, &esito, 1, 0) <= 0) error_handler(sd_avversario);
            if (send(sd_proprietario, &giocata, 1, 0) <= 0) error_handler(sd_proprietario);
        }
        //si aggiornano i contatori dei giocatori
        switch (esito)
        {
            case '1':
                proprietario -> vittorie++;
                avversario -> sconfitte++;
            case '2': 
                proprietario -> sconfitte++;
                avversario -> vittorie++;
            default:
                proprietario -> pareggi++;
                avversario -> pareggi++;
        }
        dati_partita -> stato = TERMINATA;
        segnala_cambiamento_partite();

        //partita finita, rimane in stato terminata finchè la rivincita viene accettata o rifiutata
        char risposta = '\0';
        if (send(sd_proprietario, "Rivincita? [s/n]", 16, 0) <= 0) error_handler(sd_proprietario);
        if (recv(sd_proprietario, &risposta, 1, 0) <= 0) error_handler(sd_proprietario);
        risposta = toupper(risposta);
        
        if (risposta == 'N') if (send(sd_avversario, "Rivincita rifiutata", 19, 0) <= 0) error_handler(sd_avversario);
        if (send(sd_avversario, "Rivincita? [s/n]", 16, 0) <= 0) error_handler(sd_avversario);
        if (recv(sd_avversario, &risposta, 1, 0) <= 0) error_handler(sd_avversario);
        risposta = toupper(risposta);

        if(risposta == 'N') //si torna alla lobby
        {
            if (send(sd_proprietario, "Rivincita rifiutata", 19, 0) <= 0) error_handler(sd_proprietario);
            proprietario -> stato = IN_LOBBY;
            avversario -> stato = IN_LOBBY;
            break;
        }
    }
}
void funzione_lobby(const int sd_giocatore, struct nodo_giocatore *dati_giocatore)
{
    char inbuffer[MAXIN]; //contiene le "scelte" del giocatore
    char outbuffer[MAXOUT]; //contiene tutte le statistiche del giocatore formattate in un'unica stringa
    
    while (true)
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

        if (send(sd_giocatore, outbuffer, strlen(outbuffer), 0) <= 0) error_handler(sd_giocatore);
        invia_partite(sd_giocatore);
        if (recv(sd_giocatore, inbuffer, MAXIN, 0) <= 0) error_handler(sd_giocatore);

        inbuffer[0] = toupper(inbuffer[0]);
        if (strcmp(inbuffer, "Esci") == 0) break;
        else if (strcmp(inbuffer, "Crea") == 0) 
        {
            struct nodo_partita *nodo_partita = crea_partita_in_testa(dati_giocatore -> nome, sd_giocatore);
            gioca_partita(nodo_partita);
            cancella_partita(nodo_partita);
        }
        else 
        {
            struct nodo_partita *partita = trova_partita_da_indice(atoi(inbuffer));
            if (partita == NULL)
            {
                if (send(sd_giocatore, "Partita non trovata", 19, 0) <= 0) error_handler(sd_giocatore);
            }
            else
            {
                if (!unione_partita(partita, sd_giocatore, dati_giocatore -> nome)) //se il proprietario rifiuta
                    if (send(sd_giocatore, "Richiesta di unione rifiutata", 29, 0) <= 0) error_handler(sd_giocatore);

                while (dati_giocatore -> stato == IN_PARTITA) {} //NON FUNZIONA, bisogna trovare un modo per bloccare il thread fino alla fine della partita
            }
        }
    }
}
void cancella_partita(struct nodo_partita *nodo)
{
    if (nodo != NULL && testa_partite == nodo) //significa che si sta cercando di cancellare la testa
    {
        free(nodo);
        testa_partite = testa_partite -> next_node;
    }
    else if (nodo != NULL)
    {
        struct nodo_partita *tmp = testa_partite;
        while(tmp -> next_node != nodo && tmp != NULL)
        {
            tmp = tmp -> next_node;
        }
        tmp -> next_node = nodo -> next_node;
        free(nodo);
    }
}
void segnala_cambiamento_partite()
{ 
    const pthread_t tid_mittente = pthread_self();
    pthread_t tid_ricevente = 0;
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente && tmp -> stato == IN_LOBBY) pthread_kill(tid_ricevente, SIGUSR1);
        tmp = tmp -> next_node;
    }
}
void segnala_nuovo_giocatore()
{ 
    const pthread_t tid_mittente = pthread_self();
    pthread_t tid_ricevente = 0;
    struct nodo_giocatore *tmp = testa_giocatori;

    while (tmp != NULL)
    {
        tid_ricevente = tmp -> tid_giocatore;
        if (tid_ricevente != tid_mittente && tmp -> stato == IN_LOBBY) pthread_kill(tid_ricevente, SIGUSR2);
        tmp = tmp -> next_node;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni generali server

int inizializza_server() //crea la socket, si mette in ascolto e restituisce il socket descriptor
{
    int sd;
    struct sockaddr_in address;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
        perror("socket creation error"), exit(EXIT_FAILURE);

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
void error_handler(const int sd_giocatore)
{

    struct nodo_giocatore *giocatore = trova_giocatore_da_sd(sd_giocatore);
    const pthread_t tid = giocatore -> tid_giocatore;
    struct nodo_partita *partita = trova_partita_da_sd(sd_giocatore);

    if (partita != NULL) cancella_partita(partita);
    if (giocatore != NULL) pthread_kill(tid, SIGALRM);
}
void* thread_giocatore(void *sd)
{
    const int sd_giocatore = *((int *)sd); //cast del puntatore void a int e deferenziazione
    struct nodo_giocatore *giocatore = registra_giocatore(sd_giocatore);
    segnala_nuovo_giocatore();

    funzione_lobby(sd_giocatore, giocatore);

    close(sd_giocatore);    
    cancella_giocatore(giocatore);
    cancella_nodo_tid(pthread_self());
    pthread_exit(NULL);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di signal handling

void sigalrm_handler()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    close(giocatore -> sd_giocatore);
    cancella_giocatore(giocatore);
    cancella_nodo_tid(pthread_self());
    pthread_exit(NULL);
}
void handler_nuovo_giocatore()
{
    struct nodo_giocatore *tmp = testa_giocatori;

    char messaggio[MAXOUT];
    memset(messaggio, 0, MAXOUT);
    if (tmp != NULL) //controllo teoricamente inutile
    {
        strcat(messaggio, tmp -> nome); 
        strcat(messaggio, " è appena entrato in lobby!");

        struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
        const int sd = giocatore -> sd_giocatore;

        //l'handler ignora gli errori per essere il più veloce possibile
        send(sd, messaggio, strlen(messaggio), 0);
    }
}
void invia_partite()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    const int client_sd = giocatore -> sd_giocatore;
    struct nodo_partita *tmp = testa_partite;

    char outbuffer[MAXOUT];
    memset(outbuffer, 0, MAXOUT);

    unsigned int indice = 0; //conta le partite a cui è possibile aggiungersi
    char stringa_indice[3]; //l'indice verrà convertito in questa stringa
    memset(stringa_indice, 0, 3);

    char stato_partita[28];

    if(tmp == NULL) send(client_sd, "\nNon ci sono partite attive al momento, scrivi \"crea\" per crearne una o \"esci\" per uscire", 89, 0);
    //anche questa funzione non fa error checking in quanto signal handler
    else
    {
        while (tmp != NULL)
        {
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
                sprintf(stringa_indice, "%u", indice);
                strcat(outbuffer, "ID: "); strcat(outbuffer, stringa_indice);
            }
            send(client_sd, outbuffer, strlen(outbuffer), 0);
            send(client_sd, "\nUnisciti a una partita in attesa scrivendo il relativo ID, scrivi \"crea\" per crearne una o \"esci\" per uscire", 109, 0);

            tmp = tmp -> next_node;
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni per la creazione e distruzione dei thread

struct nodo_tid* crea_nodo_tid()
{
    struct nodo_tid *nuova_testa = (struct nodo_tid *) malloc(sizeof(struct nodo_tid));
    if (nuova_testa == NULL) return NULL;

    nuova_testa -> next_node = testa_thread;
    testa_thread = nuova_testa;
    return testa_thread;
}
void cancella_nodo_tid(const pthread_t tid)
{
    if (testa_thread != NULL && testa_thread -> tid == tid) //significa che si sta cercando di cancellare la testa
    {
        struct nodo_tid *canc = testa_thread;
        testa_thread = testa_thread -> next_node;
        free(canc);
    }
    else if (testa_thread != NULL)
    {
        struct nodo_tid *tmp = testa_thread;
        while(tmp -> next_node -> tid != tid && tmp != NULL) //in teoria è impossibile che tmp diventi null
        {
            tmp = tmp -> next_node;
        }
        struct nodo_tid *canc = tmp -> next_node;
        tmp -> next_node = canc-> next_node;
        free(canc);
    }
}