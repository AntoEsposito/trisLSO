#include "funzioni.h"

//funzioni di gestione giocatori

struct nodo_giocatore* crea_giocatore_in_testa(const char *nome_giocatore, const int client_sd)
{
    struct nodo_giocatore *nuova_testa = (struct nodo_giocatore *) malloc(sizeof(struct nodo_giocatore));
    if (nuova_testa == NULL)
    {
        send(client_sd, "errore", 6, 0); 
        close(client_sd);
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
    return nuova_testa;
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
        pthread_exit(NULL);
    }

    if (send(client_sd, "Inserisci il tuo nome per registrarti (max 15 caratteri)", 56, 0) <= 0) 
    {
        close(client_sd);
        free(giocatore);
        pthread_exit(NULL);
    }

    bool nome_trovato = true;
    while (nome_trovato)
    {
        memset(giocatore, 0, MAXPLAYER);
        
        //si occupa il codice del client di verificare che i caratteri inviati siano al massimo 15
        if (recv(client_sd, giocatore, MAXPLAYER, 0) <= 0 )
        {
            close(client_sd);
            free(giocatore);
            pthread_exit(NULL);
        }

        if(!esiste_giocatore(giocatore)) nome_trovato = false;

        if (nome_trovato && send(client_sd, "Il nome inserito è già utilizzato, prova un altro nome (max 15 caratteri)", 75, 0) <= 0)
        {
            close(client_sd);
            free(giocatore);
            pthread_exit(NULL);
        }
    }

    if (send(client_sd, "Registrazione completata", 24, 0) <= 0)
    {
        close(client_sd);
        free(giocatore);
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
    struct nodo_partita *tmp = testa_partite;
    while (tmp != NULL)
    {
        if (tmp -> stato == IN_ATTESA || tmp -> stato == NUOVA_CREAZIONE) indice_partita++;
        if (indice_partita == indice) return tmp;
    }
    return NULL;
}
bool accetta_partita(struct nodo_partita *partita, const int sd_avversario, const char *nome_avversario)
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
    if(recv(sd_proprietario, &risposta, 1, 0) <= 0)     //si potrebbe aggiungere un timer per mancata risposta
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

    //TODO addormentare il thread, verrà svegliato da funzione_lobby()

    const int sd_avversario = dati_partita -> sd_avversario;
    char nome_avversario[MAXPLAYER];
    memset(nome_avversario, 0, MAXPLAYER);
    strcpy(nome_avversario, dati_partita -> avversario);

    struct nodo_giocatore *avversario = trova_giocatore_da_sd(sd_avversario);
    avversario -> stato = IN_PARTITA;
    dati_partita -> stato = IN_CORSO; 

    bool rivincita_accettata = false;
    int round = 0;

    do
    {
        dati_partita -> stato = IN_CORSO;
        segnala_cambiamento_partite();
        round++;

        char giocata = '\0';
        char esito = '0'; //il codice del client cambia il valore di questa variabile quando la partita finisce
        //'0' = ancora in corso '1' = vince proprietario, '2' = vince avversario, '3' = pareggio
        //si potrebbero usare costanti locali per migliore leggibilità

        //inizia la partita
        do
        {
            if (round%2 != 0)
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
            else 
            {
                //inizia l'avversario
                if (send(sd_avversario, "Tocca a te", 10, 0) <= 0) error_handler(sd_avversario);
                if (recv(sd_avversario, &giocata, 1, 0) <= 0) error_handler(sd_avversario);
                if (recv(sd_avversario, &esito, 1, 0) <= 0) error_handler(sd_avversario);
                if (send(sd_proprietario, &giocata, 1, 0) <= 0) error_handler(sd_proprietario);
                if (esito != '0') break;

                //turno del proprietario
                if (send(sd_proprietario, "Tocca a te", 10, 0) <= 0) error_handler(sd_proprietario);
                if (recv(sd_proprietario, &giocata, 1, 0) <= 0) error_handler(sd_proprietario);
                if (recv(sd_proprietario, &esito, 1, 0) <= 0) error_handler(sd_proprietario);
                if (send(sd_avversario, &giocata, 1, 0) <= 0) error_handler(sd_avversario);
            }
        } while (esito == '0');

        //scambio dell'esito di vittoria nei round pari
        if (round%2 == 0)
        {
            if (esito == '1') esito = '2';
            else if (esito == '2') esito = '1';
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
        dati_partita -> stato = TERMINATA; //non credo importi segnalare a tutti i giocatori in lobby ogni rivincita
        segnala_cambiamento_partite();

        //partita finita, rimane in stato terminata finchè la rivincita viene accettata o rifiutata
        char risposta = '\0';

        if (send(sd_avversario, "Rivincita? [s/n]", 16, 0) <= 0) error_handler(sd_avversario);
        if (recv(sd_avversario, &risposta, 1, 0) <= 0) error_handler(sd_avversario);            //si potrebbe aggiungere un timer per mancata risposta
        risposta = toupper(risposta);
        
        if (risposta != 'S') {if (send(sd_proprietario, "Rivincita rifiutata", 19, 0) <= 0) error_handler(sd_proprietario);}
        else 
        {
            if (send(sd_proprietario, "Rivincita? [s/n]", 16, 0) <= 0) error_handler(sd_proprietario);  //problema: il proprietario rimane in attesa della risposta dell'avversario senza saperlo
            if (send(sd_avversario, "In attesa di risposta dall'avversario", 37, 0) <= 0) error_handler(sd_avversario);
            if (recv(sd_proprietario, &risposta, 1, 0) <= 0) error_handler(sd_proprietario);    //si potrebbe aggiungere un timer per mancata risposta
            risposta = toupper(risposta);
        }

        if(risposta != 'S') //si torna alla lobby
        {
            if (send(sd_avversario, "Rivincita rifiutata", 19, 0) <= 0) error_handler(sd_avversario);
            proprietario -> stato = IN_LOBBY;
            avversario -> stato = IN_LOBBY;
            //TODO svegliare thread avversario
            rivincita_accettata = false;
        }
        else
        {
            if (send(sd_avversario, "Rivincita accettata, pronti per il prossimo round", 49, 0) <= 0) error_handler(sd_avversario);
            if (send(sd_proprietario, "Rivincita accettata, pronti per il prossimo round", 49, 0) <= 0) error_handler(sd_proprietario);
        }
    } while (rivincita_accettata);
}
void funzione_lobby(const int sd_giocatore, struct nodo_giocatore *dati_giocatore)
{
    char inbuffer[MAXIN]; //contiene le "scelte" del giocatore
    char outbuffer[MAXOUT]; //contiene tutte le statistiche del giocatore formattate in un'unica stringa

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

        if (send(sd_giocatore, outbuffer, strlen(outbuffer), 0) <= 0) error_handler(sd_giocatore);
        invia_partite();
        if (recv(sd_giocatore, inbuffer, MAXIN, 0) <= 0) error_handler(sd_giocatore);

        inbuffer[0] = toupper(inbuffer[0]);
        if (strcmp(inbuffer, "Esci") == 0) connesso = false;
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
                if (!accetta_partita(partita, sd_giocatore, dati_giocatore -> nome))
                    {if (send(sd_giocatore, "Richiesta di unione rifiutata", 29, 0) <= 0) error_handler(sd_giocatore);}
                else 
                    ;//TODO svegliare il thread proprietario
                    ;//TODO addormentare questo thread, verrà svegliato da gioca_partita()
            }
        }
    } while (connesso);
}
void cancella_partita(struct nodo_partita *nodo)
{
    if (nodo != NULL && testa_partite == nodo) //significa che si sta cercando di cancellare la testa
    {
        testa_partite = testa_partite -> next_node;
        free(nodo);
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
    pthread_exit(NULL);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ funzioni di signal handling

void sigalrm_handler()
{
    struct nodo_giocatore *giocatore = trova_giocatore_da_tid(pthread_self());
    close(giocatore -> sd_giocatore);
    cancella_giocatore(giocatore);
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

        if (send(sd, messaggio, strlen(messaggio), 0) <= 0) error_handler(sd);
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

    if (tmp == NULL) {if (send(client_sd, "\nNon ci sono partite attive al momento, scrivi \"crea\" per crearne una o \"esci\" per uscire", 89, 0) <= 0) error_handler(client_sd);}
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
            if (send(client_sd, outbuffer, strlen(outbuffer), 0) <= 0) error_handler(client_sd);

            tmp = tmp -> next_node;
        }
        if (send(client_sd, "\nUnisciti a una partita in attesa scrivendo il relativo ID, scrivi \"crea\" per crearne una o \"esci\" per uscire", 109, 0) <= 0) error_handler(client_sd);
    }
}