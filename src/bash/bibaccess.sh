#!/bin/bash

# Inizializzazione delle variabili per contare le richieste o i prestiti
temp=0
total=0
is_query=0

# Funzione per processare le richieste di tipo MSG_QUERY
process_query() {
    while read -r line; do
        # Estrai il numero dopo "QUERY" e sommalo a total
        number=$(echo "$line" | sed -n 's/QUERY[[:space:]]*\([0-9]\+\)/\1/p')
        if [[ -n $number ]]; then
            temp=$((temp + number))
        fi
    done < "$1"
}


# Funzione per processare le richieste di tipo MSG_LOAN
process_loan() {
    while read -r line; do
        # Estrai il numero dopo "LOAN" e sommalo a temp
        number=$(echo "$line" | sed -n 's/LOAN[[:space:]]*\([0-9]\+\)/\1/p')
        if [[ -n $number ]]; then
            temp=$((temp + number))
        fi
    done < "$1"
}

# Verifica il numero di argomenti e l'opzione specificata
if [[ $# -ge 2 && ($1 == "--query" || $1 == "--loan") ]]; then
    # Imposta la variabile di controllo in base all'opzione
    if [[ $1 == "--query" ]]; then
        is_query=1
    fi
    
    # Rimuovi l'opzione dalla lista degli argomenti
    shift
    
    # Esegui l'elaborazione dei file di log
    for log_file in "$@"; do
        
        file_path="logs/$log_file"
        
        # Estrai il nome del file
        file_name=$(basename "$log_file")
        
        # Processa le richieste in base al valore della variabile di controllo
        if [[ $is_query -eq 1 ]]; then
            process_query "$file_path"
        else
            process_loan "$file_path"
        fi
        
        # Stampa il risultato per il file di log corrente
        echo "$file_name $((temp))"
        
        #aggiungo il valore al totale
        total=$((total + temp))
        
        # Resetta le variabili per il prossimo file
        temp=0
        
    done
    # Stampa il tempe complessivo delle richieste o dei prestiti
    if [[ $is_query -eq 1 ]]; then
        echo "QUERY $((total))"
    else
        echo "LOAN $((total))"
    fi
    
else
    # Stampa messaggio di errore in caso di argomenti non validi
    echo "Utilizzo: $0 --query file1.log ... fileN.log"
    echo "         $0 --loan file1.log ... fileN.log"
fi
