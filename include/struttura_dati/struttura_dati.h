#ifndef STRUTTURA_DATI_H
#define STRUTTURA_DATI_H

#include "../my_lib/dynamic_array.h"
#include "libro.h"
#include "arrayCampi.h"


#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL 294
#endif

#ifndef ERR_FORMATO_STR
#define ERR_FORMATO_STR 9325
#endif

#ifndef ERR_FORMATO_DATA
#define ERR_FORMATO_DATA 2456
#endif

#ifndef ERR_BUFFER_OVERFLOW
#define ERR_BUFFER_OVERFLOW 934
#endif

#ifndef ERR_SCRITTURA_FILE
#define ERR_SCRITTURA_FILE 775
#endif

#ifndef MAX_RIGA
#define MAX_RIGA 1024
#endif

#ifndef MAX_PATH
#define MAX_PATH 108
#endif

/**
 * @struct strutturaDati
 * @brief Struttura principale per la gestione dei libri e dei loro campi.
 *
 * Questa struttura dati centralizza la gestione di una collezione di libri, associando i vari campi dei libri
 * (come autore, titolo, genere) a strutture dati dinamiche per un accesso e una ricerca efficienti. La struttura
 * utilizza array dinamici per mantenere l'organizzazione e la gestione di campi e libri.
 *
 * @param str_d_arrayCampi
 * Array dinamico di `elemento_array_campo`. Ogni elemento dell'array rappresenta un campo diverso (es. autore, titolo)
 * e il corrispondente albero di valori che organizza i libri secondo quel campo.
 *
 * @param str_d_ptrLibri
 * Array dinamico di puntatori a libri. Questo array memorizza i puntatori a tutte le strutture libro gestite
 * dalla struttura dati, consentendo un accesso rapido e diretto ai libri senza necessità di attraversare gli alberi di valori.
 */
struct strutturaDati
{
    struct dynamic_array str_d_arrayCampi;
    struct dynamic_array str_d_ptrLibri;
};

/**
 * @brief Genera la struttura dati da un file di record.
 *
 * Legge un file di record, popola la struttura dati con libri, campi e valori estratti dal record.
 *
 * @param file_record Percorso del file di record.
 * @return int `ERR_SYSTEM_CALL` se ci sono errori di sistema, `ERR_FORMATO_STR` se una stringa di un libro
 *         non è del formato corretto, `ERR_FORMATO_DATA` se un libro contiene un prestito con una data
 *         in un formato non valido, altrimenti `SUCCESS`.
 */
int str_d_genera(struct strutturaDati *strutturaDati, const char *file_record);

/**
 * @brief Gestisce la richiesta di libri in base a una query fornita.
 *
 * Filtra i libri nella struttura dati in base alla query fornita, leggendo o prestando i libri corrispondenti.
 * Ritorna il numero di libri letti o prestati.
 *
 * @param dst Puntatore alla stringa di destinazione dove aggregare i risultati.
 * @param richiesta Query di ricerca dei libri.
 * @param presta Flag che indica se prestare i libri (1) o solo leggerli (0).
 * @return int Numero di libri letti o prestati, ERR_FORMATO_STR se la richiesta non è del formato corretto, ERR_SYSTEM_CALL per errori di sistema, o 0 se nessun libro corrisponde.
 */
int str_d_chiediLibri(struct strutturaDati *strutturaDati, char **dst, char *richiesta, const int presta, pthread_mutex_t *mutex, pthread_cond_t *cond);

/**
 * @brief Aggiorna il file di record con i libri attuali nella struttura dati.
 *
 * Crea un file temporaneo per scrivere i libri aggiornati, quindi sostituisce il file di record esistente.
 *
 * @param file_record Percorso del file di record da aggiornare.
 * @param build_directory Directory dove creare il file temporaneo, deve terminare con "/".
 * @return int ERR_SYSTEM_CALL per errori di sistema, ERR_BUFFER_OVERFLOW per eccesso di lunghezza del percorso del file,
 *         ERR_SCRITTURA_FILE se un libro non è stato scritto completamente, altrimenti SUCCESS.
 */
int str_d_aggiornaFileRecord(struct strutturaDati *strutturaDati, const char *file_record, const char *build_directory);

/**
 * @brief Dealloca la struttura dati e tutti i libri in essa contenuti.
 *
 * Libera la memoria occupata dalla struttura dati, inclusi i libri e i campi associati. Non ci sono valori di ritorno
 * specificati per gli errori in questa funzione poiché si assume che completi sempre con successo la liberazione delle risorse.
 */
void str_d_dealloca(struct strutturaDati *strutturaDati);

#endif
