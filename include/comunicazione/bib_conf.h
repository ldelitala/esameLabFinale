#ifndef BIB_CONF_H
#define BIB_CONF_H

#include "../my_lib/readers_writers2.h"

/**
 * Library name: bib_conf.h
 * Author: Lorenzo Delitala
 * ------------------------
 * This library provides functions for both server and client so that multiple processes
 * can read and write data on the file bib.conf safely
 *
 */

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -1
#endif

#ifndef ERR_BUFFER_OVERFLOW
#define ERR_BUFFER_OVERFLOW 4365
#endif

#ifndef ERR_SCRITTURA
#define ERR_SCRITTURA 3566
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 108
#endif

#ifndef MAX_RIGA
#define MAX_RIGA 1024
#endif

//! FUNZIONI PER IL SERVER

/**
 * @brief Aggiunge un percorso di socket al file di configurazione.
 *
 * Questa funzione aggiunge in modo process-safe un nuovo percorso di socket al file `bib.conf`,
 * utilizzando semafori per la sincronizzazione. Assicura che il file sia accessibile da un solo
 * scrittore alla volta per prevenire corruzioni dei dati.
 *
 * @return int Restituisce SUCCESS se l'operazione è stata completata con successo, ERR_SYSTEM_CALL
 * se una chiamata di sistema è fallita, o ERR_SCRITTURA se c'è stato un errore nella scrittura del file.
 *
 * @note Questa funzione gestisce internamente tutte le necessarie sincronizzazioni e gli errori di
 * gestione del file, inclusa l'acquisizione e il rilascio dei semafori.
 */
int bib_addSocket(char *bib_path, char *socket_path, char *bib_name);

/**
 * @brief Rimuove un percorso di socket dal file di configurazione.
 *
 * Questa funzione crea un file temporaneo nel quale copia tutto il contenuto
 * del file `bib.conf` originale eccetto il percorso di socket specificato.
 * Sostituisce poi il file originale con questo file temporaneo in modo process-safe,
 * assicurando che l'operazione non interferisca con gli accessi concorrenti.
 *
 * @return int Restituisce SUCCESS se l'operazione viene completata con successo, ERR_SYSTEM_CALL
 * per fallimenti nelle chiamate di sistema, o ERR_BUFFER_OVERFLOW se il percorso temporaneo supera la lunghezza massima.
 *
 * @details Prima di sostituire il file originale, viene creato un backup per permettere il recupero in caso
 * di errori. La funzione gestisce l'acquisizione e il rilascio dei semafori per la sincronizzazione.
 */
int bib_removeSocket(char *bib_path, char *socket_path, char *build_directory_path);

//! FUNZIONI PER IL CLIENT

/**
 * @brief Legge il contenuto del file `bib.conf`.
 *
 * Questa funzione legge l'intero contenuto del file `bib.conf`,
 * restituendolo come una stringa allocata dinamicamente. Assicura l'accesso al file
 * in modo process-safe coordinandosi con altri lettori e scrittori attraverso i semafori.
 *
 * @return char* Un puntatore a una stringa allocata dinamicamente contenente il contenuto del file.
 * Restituisce NULL se si verifica un errore durante la lettura o l'allocazione della memoria.
 *
 * @note È responsabilità del chiamante liberare la memoria restituita. La funzione
 * gestisce la sincronizzazione e gli errori di accesso al file, inclusa l'acquisizione
 * e il rilascio dei semafori.
 */
char *bib_leggi(char *bib_path);

#endif