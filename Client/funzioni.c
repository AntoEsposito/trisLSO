#include "funzioni.h"

void* thread_fun()
{
    char inbuffer[MAXLETTORE];
    memset(inbuffer, 0, MAXLETTORE);

    //il thread principale crea un thread scrittore detatched
    pthread_t tid_scrittore;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid_scrittore, &attr, fun_scrittore, NULL);

    printf("TRIS-LSO\n");
    //il thread scrittore viene ucciso per giocare le partite per evitare conflitti con le send
    while (recv(sd, inbuffer, MAXLETTORE, 0) > 0)
    {
        if (strcmp(inbuffer, "*** Inizia la partita come proprietario ***\n") == 0) 
        {
            pthread_kill(tid_scrittore, SIGUSR1);
            gioca_partite(inbuffer, PROPRIETARIO);
            pthread_create(&tid_scrittore, &attr, fun_scrittore, NULL);
        }
        else if (strcmp(inbuffer, "*** Inizia la partita come avversario ***\n") == 0) 
        {
            pthread_kill(tid_scrittore, SIGUSR1);
            gioca_partite(inbuffer, AVVERSARIO);
            pthread_create(&tid_scrittore, &attr, fun_scrittore, NULL);
        }
        else printf("%s", inbuffer);
        memset(inbuffer, 0, MAXLETTORE);
    }
    pthread_kill(tid_scrittore, SIGUSR1);
    close(sd);
    pthread_exit(NULL);
}

void* fun_scrittore()
{
    char outbuffer[MAXSCRITTORE];

    do
    {
        memset(outbuffer, 0, MAXSCRITTORE);
        //strnlen è più sicura di strlen per stringhe che potrebbero non terminare con \0 come in questo caso
        if (fgets(outbuffer, MAXSCRITTORE, stdin) != NULL && outbuffer[strnlen(outbuffer, MAXSCRITTORE)-1] == '\n') //massimo 15 caratteri nel buffer escluso \n
        {
            if (outbuffer[0] != '\n') send(sd, outbuffer, strlen(outbuffer)-1, 0);
            else 
            {
                outbuffer[0] = '\0'; //evita che il giocatore invii un input vuoto
                printf("Inserisci un input valido\n");
            }
        }
        else //sono stati scritti più di 15 caratteri
        {
            printf("Puoi scrivere al massimo 15 caratteri\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF); //svuota lo standard input (fflush non funziona)
        }
    } while (strcmp(outbuffer, "esci\n") != 0);
    close(sd);
    printf("Uscita\n");
    exit(EXIT_SUCCESS);
}


void gioca_partite(char *inbuffer, const enum tipo_giocatore tipo)
{
    unsigned int round = 0;
    do
    {
        memset(griglia, 0, 9);
        round++;
        unsigned short int n_giocate = 0;
        char esito = '0';
        char e_flag = NOERROR; //il server manda 1 in caso di errore
        if (tipo == PROPRIETARIO ) printf("\nRichiesta accettata, inizia la partita!\n");
        if (tipo == AVVERSARIO ) printf("\nIl proprietario ha accettato la richiesta, inizia la partita!\n");
        printf("\nRound %u\n", round);
        if (tipo == AVVERSARIO ) printf("\nTurno dell'avversario\n");
        do
        {
            memset(inbuffer, 0, MAXLETTORE);

            //controllo che serve a distinguere chi comincia per primo
            if ((tipo == PROPRIETARIO && round%2 == 1) || (tipo == AVVERSARIO && round%2 == 0))
            {   //ogni player vede egli stesso come O e l'avversario come X
                if (n_giocate == 0) //primo turno
                {
                    stampa_griglia();
                    printf("Tocca a te\n");
                    esito = invia_giocata(&n_giocate);
                    printf("Turno dell'avversario\n");
                }
                else //dal secondo turno in poi deve prima ricevere la giocata dell'avversario e poi iniziare il suo turno
                { //viene controllato il flag di errore prima di ricevere la giocata dell'avversario
                    if (recv(sd, &e_flag, 1, 0) <= 0) error_handler(); 
                    if (e_flag == ERROR) {esito = 1; break;}
                    //non c'è errore, riceve la giocata dell'avversario
                    if ((esito = ricevi_giocata(&n_giocate)) != '0') break;
                    printf("Tocca a te\n");
                    if ((esito = invia_giocata(&n_giocate)) != '0') break;
                    printf("Turno dell'avversario\n");
                }
            }
            else
            {
                if (recv(sd, &e_flag, 1, 0) <= 0) error_handler(); 
                if (e_flag == ERROR) {esito = 1; break;}
                //non c'è errore, riceve la giocata dell'avversario
                if ((esito = ricevi_giocata(&n_giocate)) != '0') break;
                printf("Tocca a te\n");
                if ((esito = invia_giocata(&n_giocate)) != '0') break;
                printf("Turno dell'avversario\n");
            }
        } while (esito == '0');
        if (e_flag == ERROR) {printf("L'avversario si è disconnesso, vittoria a tavolino\n"); break;}

    } while (rivincita(tipo));
}

char invia_giocata(unsigned short int *n_giocate)
{ 
    int c; //variabile ausiliaria per pulire lo stdin
    char giocata[2] = {'\0', '\0'};
    int num_giocata = 0;
    bool giocata_valida = false;
    char esito = '0';

    do
    {
        printf("Scrivi un numero da 1 a 9 per indicare dove posizionare la O\n");
        giocata[0] = getchar();
        if (giocata [0] != '\n') //controllo per un invio nullo
        {
            num_giocata = atoi(giocata);
            giocata_valida = controllo_giocata(num_giocata);
            while ((c = getchar()) != '\n' && c != EOF);  // Svuota lo stdin per sicurezza
        }
        if (!giocata_valida) printf("Giocata non valida\n"); 
    } while(!giocata_valida);

    //giocata valida, può essere inviata al server
    send(sd, giocata, 1, 0);

    //inserisce la giocata nella propria griglia locale e invia l'esito al server
    inserisci_O(num_giocata);
    (*n_giocate)++;
    if (system("clear") < 0) perror("errore"), exit(EXIT_FAILURE);
    stampa_griglia();

    //ha senso controllare l'esito solo se sono state fatte almeno 5 giocate
    if (*(n_giocate) >= 5) esito = controllo_esito(n_giocate);
    send(sd, &esito, 1, 0);
    return esito;
}

char ricevi_giocata(unsigned short int *n_giocate)
{
    char giocata[2] = {'\0', '\0'};
    char num_giocata = 0;
    char esito = '0';

    //da per scontato che la giocata sia valida
    if (recv(sd, giocata, 1, 0) <= 0) error_handler();
    num_giocata = atoi(giocata);

    inserisci_X(num_giocata);
    (*n_giocate)++;
    if (system("clear") < 0) perror("errore"), exit(EXIT_FAILURE);
    stampa_griglia();

    esito = controllo_esito(n_giocate);
    //non invia l'esito perchè se ne occupa chi invia la giocata
    return esito; 
}

bool rivincita(const enum tipo_giocatore tipo)
{
    char buffer[MAXLETTORE];
    memset(buffer, 0, MAXLETTORE);
    char risposta;
    int c;
    bool risposta_valida = false;

    if (tipo == PROPRIETARIO) printf("L'avversario sta scegliendo se vuole o meno la rivincita\n");

    //Avversario riceve richiesta di rivincita, proprietario riceve risposta dell'avversario
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) error_handler();
    printf("%s", buffer);

    do //verifica che l'input sia valido
    {
        if (strcmp(buffer, "Rivincita rifiutata dall'avversario, ritorno in lobby\n") == 0) return false; //questo messaggio può riceverlo solo il proprietario
        else memset(buffer, 0, MAXLETTORE);

        risposta = getchar();
        if (risposta != '\n') //il giocatore ha premuto invio senza dare input
        {
            while ((c = getchar()) != '\n' && c != EOF);  // Svuota lo stdin per sicurezza
            risposta = toupper(risposta);
            if (risposta != 'S' && risposta != 'N') printf("Scrivi una risposta valida\n"); 
            else risposta_valida = true;
        }
    } while(!risposta_valida);

    //input corretto, lo invia al server (nel caso del proprietario potrebbe non essercene bisogno)
    send(sd, &risposta, 1, 0 );

    if (tipo == AVVERSARIO) //all'avversario viene chiesto di attendere il proprietario (se ha scelto la rivincita)
    {
        if (risposta == 'N')
        {
            printf("Rivincita rifiutata\n");
            return false;
        }
        if (recv(sd, buffer, MAXLETTORE, 0) <= 0) error_handler();
        printf("%s", buffer);
        if (strcmp(buffer, "Rivincita rifiutata\n") == 0) return false;
        memset(buffer, 0, MAXLETTORE);
    }
    //riceve e stampa feedback positivo o negativo
    if (recv(sd, buffer, MAXLETTORE, 0) <= 0) error_handler();
    printf("%s", buffer);
    if (tipo == AVVERSARIO) { if (strcmp(buffer, "Rivincita rifiutata dal proprietario\n") == 0) return false; }
    else if (strcmp(buffer, "Ritorno in lobby\n") == 0) return false;
    return true;
}


char controllo_esito(const unsigned short int *n_giocate)
{
    char esito = '0';
    //se la funzione trova un tris di O restituisce 1, tris di X restituisce 2, pareggio restituisce 3
    // Controlla le righe
    for (int i = 0; i < 3; i++) 
    {
        if (griglia[i][0] != 0 && griglia[i][0] == griglia[i][1] && griglia[i][1] == griglia[i][2])
        {
            if (griglia[i][0] == 'O') { esito = '1'; printf("Hai vinto!\n"); }
            else { esito = '2'; printf("Vince l'avversario\n"); }
        }
    }
    // Controlla le colonne
    for (int i = 0; i < 3; i++) 
    {
        if (griglia[0][i] != 0 && griglia[0][i] == griglia[1][i] && griglia[1][i] == griglia[2][i])
        {
            if (griglia[0][i] == 'O') { esito = '1'; printf("Hai vinto!\n"); }
            else { esito = '2'; printf("Vince l'avversario\n"); }
        }
    }
    // Controlla la diagonale principale
    if (griglia[0][0] != 0 && griglia[0][0] == griglia[1][1] && griglia[1][1] == griglia[2][2])
    {
        if (griglia[0][0] == 'O') { esito = '1'; printf("Hai vinto!\n"); }
        else { esito = '2'; printf("Vince l'avversario\n"); }
    }
    // Controlla la diagonale secondaria
    if (griglia [0][2] != 0 && griglia[0][2] == griglia[1][1] && griglia[1][1] == griglia[2][0])
    {
        if(griglia[0][2] == 'O') { esito = '1'; printf("Hai vinto!\n"); }
        else { esito = '2'; printf("Vince l'avversario\n"); }
    }
    //controlla un eventuale pareggio 
    if (esito == '0' && *n_giocate == 9) { esito = '3'; printf("Pareggio\n"); }
    return esito;
}

bool controllo_giocata(const int giocata)
{
    if (giocata < 1) return false;
    if (giocata > 9) return false;

    //controlla se la casella selezionata è gia occupata
    int indice = giocata - 1;
    const unsigned short int i_colonna = indice%3;
    const unsigned short int i_riga = indice/3;
    if (griglia[i_riga][i_colonna] != '\0') return false;
    else return true;
}

void inserisci_O(const unsigned short int giocata)
{
    int indice = giocata - 1;
    const unsigned short int i_colonna = indice%3;
    const unsigned short int i_riga = indice/3;
    griglia[i_riga][i_colonna] = 'O';
}

void inserisci_X(const unsigned short int giocata)
{
    int indice = giocata - 1;
    const unsigned short int i_colonna = indice%3;
    const unsigned short int i_riga = indice/3;
    griglia[i_riga][i_colonna] = 'X';
}

void stampa_griglia()
{
    char c;

    for(int i=0; i<3; i++) 
    {
        if (griglia[0][i] == '\0') c = ' ';
        else c = griglia[0][i];
        printf("|   %c   |", c);
    }
    printf("\n");
    for(int i=0; i<3; i++) 
    {
        if (griglia[1][i] == '\0') c = ' ';
        else c = griglia[1][i];
        printf("|   %c   |", c);
    }
    printf("\n");
    for(int i=0; i<3; i++) 
    {
        if (griglia[2][i] == '\0') c = ' ';
        else c = griglia[2][i];
        printf("|   %c   |", c);
    }
    printf("\n");
}


void inizializza_socket()
{
    struct sockaddr_in ser_add;
    socklen_t lenght = sizeof(struct sockaddr_in);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket creation error"), exit(EXIT_FAILURE);

    const int opt = 1;
    //opzione reuseaddr per evitare problemi coi riavvii
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
        perror("set REUSEADDR option error"), exit(EXIT_FAILURE);

    struct timeval timer;
    timer.tv_sec = 120;  // Timer di 120 secondi
    timer.tv_usec = 0;

    if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer)) < 0 || setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0)
        perror("set socket timer error"), exit(EXIT_FAILURE);

    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
        perror("tcp nodelay error"), exit(EXIT_FAILURE);

    memset(&ser_add, 0, sizeof(ser_add));
    ser_add.sin_family = AF_INET;
    ser_add.sin_port = htons(8080);
    ser_add.sin_addr.s_addr = inet_addr("127.0.0.1");

    //non c'è bisogno di inizializzare manualmente la porte client e fare il bind

    if (connect(sd, (struct sockaddr *)&ser_add, lenght) < 0)
        perror("connect error"), exit(EXIT_FAILURE);
}


void error_handler()
{
    printf("errore\n");
    close(sd);
    exit(EXIT_FAILURE);
}

void SIGUSR1_handler()
{
    pthread_exit(NULL);
}

void SIGTERM_handler()
{
    close(sd);
    exit(EXIT_SUCCESS);
}