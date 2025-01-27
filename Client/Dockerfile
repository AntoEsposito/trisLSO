#L'immagine di partenza è linux ALpine
FROM alpine:latest
#Aggiorna i pacchetti e installa il gcc e build-base(per il makefile)
RUN apk update && apk add --no-cache gcc musl-dev build-base
#Imposta la directory di lavoro
WORKDIR /app
#Copia il codice del server nela cartella
COPY . .
#Compila con make
RUN make
#non c'e bisogno di esporre la porta se client e server sono nella stessa rete docker
#Comando per avviare il client
CMD ["./client"]