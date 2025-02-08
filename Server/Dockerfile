#L'immagine di partenza è linux Ubuntu
FROM ubuntu:latest
#Aggiorna i pacchetti e installa il gcc e il pacchetto make
RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*
#Imposta la directory di lavoro
WORKDIR /app
#Copia il codice del server nella directory
COPY . .
#Compila il codice
RUN make
#non c'e bisogno di esporre la porta se client e server sono nella stessa rete docker
#Comando per avviare il server
CMD ["./server"]