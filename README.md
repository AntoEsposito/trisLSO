ISTRUZIONI PER AVVIARE IL PROGETTO:

1) Avere il docker client in esecuzione
2) Assicurarsi che la cartella che contiene il progetto si chiami esattamente "tris-lso"
3) Impostare la directory del progetto come work directory
4) Controllare che lo script di avvio abbia i permessi di esecuzione
5) Infine, a seconda del proprio sistema oprerativo, avviare il progetto eseguendo uno dei 2 script di avvio presenti nella cartella

Nel caso non si potesse avviare lo script per qualsiasi problema, scrivere manualmente nel terminale "docker compose up --scale client=n", per avviare il server
ed un numero n di client. Quando tutti i container saranno in esecuzione, aprire un terminale per ogni client e scrivere "docker attach tris-lso-client-i", in modo da
collegare manualmente il terminale al client desiderato.