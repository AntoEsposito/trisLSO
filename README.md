ISTRUZIONI PER AVVIARE IL PROGETTO:

1) Avere il docker client installato ed in esecuzione
(Sistemi Unix-like) eseguire "sudo usermod -aG docker proprio_nome_utente" per essere sicuri che il proprio utente appartenga al gruppo docker

2) Assicurarsi che la cartella che contiene il progetto si chiami esattamente "tris-lso"

3) Aprire un terminale ed impostare la directory del progetto come work directory (cioè "posizionandosi" all'interno della cartella tramite il comando cd)
Nel caso di Windows il terminale deve obbligatoriamente essere PowerShell

4) (Sistemi Unix-like) eseguire il comando "chmod +x ./avviounix.sh" per dare i permessi di esecuzione allo script
Eseguire anche "sudo chown -R proprio_nome_utente:proprio_nome_gruppo ." in caso root risultasse come proprietario e gruppo della cartella

5) Infine, a seconda del proprio sistema operativo, avviare il progetto eseguendo uno dei 2 script di avvio presenti nella cartella
(cioè scrivendo nel terminale ./avviounix.sh o ./avviowindows.ps1)

Attenzione perchè nel caso di Windows e macOS, il Sistema Operativo potrebbe bloccare l'esecuzione di script provenienti da fonti anonime, in questo caso:
1) Windows: eseguire lo script scrivendo nel terminale "powershell -ExecutionPolicy Bypass -File "./avviowindows.ps1""
2) macOS: rimuovere l'attributo di quarantena scrivendo nel terminale "xattr -d com.apple.quarantine ./avviounix.sh", e poi avviare lo script (punto 5)

Nel caso non si potesse comunque avviare lo script per qualsiasi problema, scrivere manualmente nel terminale "docker-compose up --scale client=n", per avviare il server
ed un numero n di client. Quando tutti i container saranno in esecuzione, aprire un terminale per ogni client e scrivere "docker attach tris-lso-client-i", in modo da
collegare manualmente il terminale al client desiderato.