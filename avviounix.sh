#!/bin/bash

# Chiedi all'utente il numero di client da avviare
echo "Inserisci il numero di client da avviare:"
read n_client

# Verifica che l'input sia un numero intero maggiore o uguale a 1
if [ "$n_client" -lt 1 ]; then
    echo "Riprova, inserisci un numero valido di client"
    exit 1
fi

# Avvia docker-compose con build e scaling per i client
if [[ "$OSTYPE" == "darwin" ]]; then
    # macOS: usa AppleScript per aprire una nuova finestra di Terminale
    osascript -e "tell application \"Terminal\" to do script \"docker-compose up --build --scale client=$n_client\""
else
    # Linux
    gnome-terminal -- bash -c "docker-compose up --build --scale client=$n_client; exec bash"
fi

# Attendi che tutti i container attesi siano in esecuzione
all_containers_running="false"
while [ "$all_containers_running" = "false" ]; do
    all_containers_running="true"
    for (( i=1; i<=n_client; i++ )); do
        container_name="tris-lso-client-$i"
        if ! docker ps --format "{{.Names}}" | grep -q "$container_name"; then
            all_containers_running="false"
            break
        fi
    done
    if [ "$all_containers_running" = "false" ]; then
        sleep 2 #aspetta 2 secondi prima di un nuovo conrollo
    fi
done

# Avvia una finestra terminale per ciascun client, collegandosi al rispettivo container
for (( i=1; i<=n_client; i++ )); do
    container_name="tris-lso-client-$i"

    if [[ "$OSTYPE" == "darwin" ]]; then
        # macOS: usa AppleScript per aprire una nuova finestra di Terminale
        osascript -e "tell application \"Terminal\" to do script \"docker attach '$container_name'\""
    else
        # Linux
        gnome-terminal -- bash -c "docker attach '$container_name'; exec bash"
    fi
done