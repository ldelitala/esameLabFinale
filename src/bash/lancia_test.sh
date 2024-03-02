#!/bin/bash

# Salvataggio del primo e secondo argomento in variabili dedicate
server_path="$1"
client_path="$2"
bibaccess_path="$3"

# Verifica che sia stati forniti entrambi i percorsi
if [ -z "$server_path" ] || [ -z "$client_path" ] || [ -z "$bibaccess_path" ]; then
    echo "Errore: devi fornire sia il percorso del server, sia quello del client che quello di bibaccess."
    echo "Uso: $0 <percorso_server> <percorso_client> <percorso bibaccess>"
    exit 1
fi

#creo variabili per i nomi delle biblioteche
bib1=ANDREA
bib2=MARCO
bib3=CASMUS
bib4=CELESTINO
bib5=SASHA



# Avvio dei server 
$server_path $bib1 bib1 1 &
$server_path $bib2 bib2 2  &
$server_path $bib3 bib3 3 &
$server_path $bib4 bib4 4 &
$server_path $bib5 bib5 5  &

sleep 1

# Ciclo per l'avvio dei client 8 volte con un'attesa di 1 secondo tra un ciclo e l'altro.
for i in {1..8}; do
    $client_path --luogo_pubblicazione="New York" &
    $client_path --anno="1998"  &
    $client_path --autore="Pagli, Linda" -p  &
    $client_path --autore="Di Ciccio, Antonio" --titolo="Manuale di architettura pisana" --editore="Palestro" --anno="1990" --volume="1345" --scaffale="A.west.2" -p  &
    $client_path --autore="Bentley, Jon"  &
    sleep 1
done

# Chiusura dei server
pkill -u $(whoami) -INT $(basename $server_path)

sleep 10

#lancio bibaccess
$bibaccess_path --query $bib1.log $bib2.log $bib3.log $bib4.log $bib5.log
$bibaccess_path --loan $bib1.log $bib2.log $bib3.log $bib4.log $bib5.log
