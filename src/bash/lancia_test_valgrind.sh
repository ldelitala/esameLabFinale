#!/bin/bash

# Salvataggio del primo e secondo argomento in variabili dedicate
server_path="$1"
client_path="$2"
bibaccess_path="$3"

# Verifica che sia stati forniti i percorsi
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

#creo variabili per le cartelle di logs
clients=logs/clients
servers=logs/servers


if [ ! -d  $clients ]; then
    mkdir -p $clients; \
fi
if [ ! -d  $servers ]; then
    mkdir -p $servers; \
fi

# Avvio dei server con valgrind
# valgrind --leak-check=full --show-leak-kinds=all -s $server_path $bib5 bib5 5 &
valgrind $server_path $bib1 bib1 1 > $servers/valgrind_$bib1.log 2>&1 &
valgrind_pid1=$!
valgrind $server_path $bib2 bib2 2 > $servers/valgrind_$bib2.log 2>&1 &
valgrind_pid2=$!
valgrind $server_path $bib3 bib3 3 > $servers/valgrind_$bib3.log 2>&1 &
valgrind_pid3=$!
valgrind $server_path $bib4 bib4 4 > $servers/valgrind_$bib4.log 2>&1 &
valgrind_pid4=$!
valgrind $server_path $bib5 bib5 5 > $servers/valgrind_$bib5.log 2>&1 &
valgrind_pid5=$!

sleep 10

# Ciclo per l'avvio dei client 8 volte con un'attesa di 1 secondo tra un ciclo e l'altro.
valgrind $client_path --luogo_pubblicazione="New York" > $clients/valgrind_newYork.log 2>&1 &
valgrind $client_path --anno="1998"  > $clients/valgrind_1998.log 2>&1 &
valgrind $client_path --autore="Pagli, Linda" -p > $clients/valgrind_pagli.log 2>&1 &
valgrind $client_path --autore="Di Ciccio, Antonio" --titolo="Manuale di architettura pisana" --editore="Palestro" --anno="1990" --volume="1345" --scaffale="A.west.2" -p  > $clients/valgrind_antonio.log 2>&1&
valgrind $client_path --autore="Bentley, Jon"  > $clients/valgrind_bentley.log 2>&1&
sleep 10

# Chiusura dei server
# Trova l'ID del processo di Valgrind che esegue il server e termina quel processo
kill -INT "$valgrind_pid1"
kill -INT "$valgrind_pid2"
kill -INT "$valgrind_pid3"
kill -INT "$valgrind_pid4"
kill -INT "$valgrind_pid5"

sleep 10

#lancio bibaccess
$bibaccess_path --query $bib1.log $bib2.log $bib3.log $bib4.log $bib5.log
$bibaccess_path --loan $bib1.log $bib2.log $bib3.log $bib4.log $bib5.log
