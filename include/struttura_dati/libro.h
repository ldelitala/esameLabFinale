/**
 * @file libro.h
 * @brief Definizione della struttura `libro` e costanti associate per la gestione di una libreria di libri.
 *
 * Questo file header definisce la struttura `libro`, utilizzata per rappresentare i dettagli di un libro all'interno
 * del sistema. Inclusi anche sono i codici di errore standard e la dimensione predefinita per la gestione delle stringhe.
 */
#ifndef LIBRO_H
#define LIBRO_H

#include <pthread.h>
#include <stdint.h>
#include "personal_time.h"

#ifndef SUCCESS
#define SUCCESS 0
#endif

/**
 * in caso di allocazione statica di memoria per stringhe che devono salvarsi un campo od un valore,
 * questa è la dimensione usata. Il numero 156 è perché la dimensione media di una stringa libro è 156
 * e quindi mi sembrava un buon numero da usare come dimensione massima
 */
#ifndef SIZE_C_V
#define SIZE_C_V 156
#endif

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -1
#endif

#ifndef ERR_FORMATO_DATA
#define ERR_FORMATO_DATA -2
#endif

struct libro
{
    char *lib_stringa;
    __int8_t lib_inPrestito, lib_inUso;
    struct tm lib_dataPrestito;
};

//! FUNZIONI UTILI ANCHE ALLA STRUTTURA DATI

/**
 * @brief Estrae coppie chiave-valore da una stringa, iterativamente.
 *
 * Utilizza ":" per separare chiave e valore, e ";" per terminare una coppia. La prima chiamata utilizza la stringa target,
 * chiamate successive passano NULL per continuare dall'ultima posizione.
 *
 * @note Modifica la stringa inserendo '\0' al posto dei delimitatori. Non è thread-safe.
 * @param stringa La stringa sorgente o NULL per proseguire l'estrazione.
 * @return 1 per successo, 0 altrimenti.
 * @warning Passare stringhe non valide o modificare la stringa tra le chiamate può causare errori.
 */
int lib_estraiCoppia(char *stringa, char **campo, char **valore);

/**
 * @brief Normalizza una stringa rimuovendo spazi extra, convertendo in minuscolo e unificando spazi.
 *
 * Modifica la stringa in-place per:
 * - Rimuovere spazi iniziali e finali.
 * - Convertire tutti i caratteri in minuscolo.
 * - Sostituire spazi multipli con uno singolo.
 *
 * @note Modifica la stringa originale. Assicurarsi che sia modificabile e null-terminated.
 * @warning Passare stringhe non inizializzate o NULL può causare errori. La lunghezza della stringa
 *          può ridursi senza riallocare memoria.
 */
void lib_formattaStringa(char *stringa);

/**
 * @brief Controlla se una stringa rispetta il formato chiave:valore; atteso.
 *
 * Analizza la stringa fornita per verificare che contenga almeno una coppia chiave-valore formattata correttamente.
 * La verifica procede fino a che non vengono identificate tutte le coppie chiave-valore presenti nella stringa,
 * assicurandosi che non ci siano coppie incomplete o malformate.
 *
 * @return int Restituisce 1 se la stringa segue il formato atteso, 0 altrimenti.
 *
 * @warning Assicurarsi che la stringa passata non sia NULL e che sia adeguatamente terminata da un carattere nullo (\0) per evitare
 *          comportamenti indefiniti durante la ricerca dei caratteri ":" e ";".
 */
int lib_controllaFormatoCorretto(const char *stringa);

//! FUNZIONI PER I LIBRI

/**
 * @brief Inizializza una struttura libro da una stringa.
 *
 * Alloca e copia `str` in `dst->lib_stringa`, rimuove il carattere di nuova riga finale se presente, e
 * inizializza i dati di prestito. Assume che `str` sia nel formato corretto.
 *
 * @param dst Puntatore alla struttura `libro` da inizializzare.
 * @param str Stringa con i dati del libro.
 *
 * @return `SUCCESS` se l'operazione è riuscita, altrimenti `ERR_SYSTEM_CALL` per errori di allocazione memoria
 *         o `ERR_FORMATO_DATA` se il formato data di prestito è invalido.
 *
 * @warning La funzione non verifica il che il formato di `str` sia corretto. Da un grande potere derivano grandi responsabilità.
 */
int lib_crea(struct libro *dst, const char *str);

/**
 * @brief Genera una stringa contenente i dati di un libro, inclusi i dettagli del prestito se presente.
 *
 * Alloca una nuova stringa per contenere i dati del libro da `libro->lib_stringa`, aggiungendo informazioni di prestito
 * se applicabile. La stringa risultante include un carattere di nuova riga finale.
 *
 * @return Puntatore alla stringa allocata contenente i dati del libro e, se in prestito, la data di prestito
 *         formattata come "prestito: gg-mm-aaaa hh:mn:sc;". Ritorna NULL in caso di errore di allocazione
 *         memoria o se la conversione della data di prestito fallisce.
 * @warning Da grandi poteri derivano grandi responsabilità. È responsabilità del chiamante liberare la memoria della stringa restituita.
 */
char *lib_leggi(struct libro *libro);

/**
 * @brief Genera in modo thread-safe una stringa con i dati di un libro, inclusi i dettagli del prestito.
 *
 * Invoca `lib_leggi` per ottenere i dati del libro specificato, assicurandosi l'accesso esclusivo alla risorsa
 * mediante l'uso di mutex e variabili di condizione.
 *
 * @return Puntatore alla stringa allocata con i dati del libro e, se applicabile, dettagli del prestito.
 *         Ritorna NULL in caso di errore nell'allocazione della memoria o fallimento nelle funzioni di lock/unlock.
 * @note È responsabilità del chiamante liberare la memoria della stringa restituita.
 */
char *lib_leggiThreadSafe(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond);

/**
 * @brief Effettua in modo thread-safe il prestito di un libro.
 *
 * Tenta di marcare un libro come prestato, verificando prima lo stato del prestito. L'operazione è protetta
 * da mutex per garantire la sicurezza in un ambiente multithread. La data del prestito viene impostata al momento attuale.
 *
 * @return Ritorna `1` se il libro è stato prestato con successo, `0` se il libro era già in prestito,
 *         o `ERR_SYSTEM_CALL` in caso di errore nelle operazioni di lock, unlock, o nell'impostazione della data.
 * @note La funzione assicura che le risorse vengano rilasciate correttamente in caso di errore.
 */
int lib_prestaThreadSafe(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond);

/**
 * @brief Verifica in modo thread-safe se un libro soddisfa una richiesta specificata.
 *
 * Confronta le coppie campo-valore specificate in `richiesta` con i dati del libro, eseguendo l'operazione
 * in modo thread-safe. Utilizza la normalizzazione delle stringhe per garantire corrispondenze accurate.
 *
 * @param richiesta Stringa contenente coppie "campo:valore;"" da verificare nel libro.
 * @return Ritorna `1` se tutte le coppie campo-valore specificate in `richiesta` corrispondono a quelle nel libro,
 *         `0` se almeno una coppia non corrisponde, o `ERR_SYSTEM_CALL` in caso di errore di allocazione o lettura dei dati.
 * @note La funzione assicura la pulizia delle risorse in caso di errore o al termine del controllo.
 */
int lib_controllaRichiestaThreadSafe(struct libro *libro, const char *richiesta, pthread_mutex_t *mutex, pthread_cond_t *cond);

/**
 * @brief Libera le risorse allocate per una struttura libro.
 *
 * Dealloca la memoria utilizzata per i campi `lib_stringa` e `lib_dataPrestito` di un libro.
 *
 * @note La funzione verifica la validità del puntatore `libro` e dei suoi campi prima della deallocazione per prevenire dereferenziazioni di puntatori NULL.
 */
void lib_free(struct libro *libro);

#endif
