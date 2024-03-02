#ifndef ARRAYCAMPI_H
#define ARRAYCAMPI_H

#include "libro.h"
#include "arrayCampi.h"
#include "../my_lib/dynamic_array.h"
#include "../my_lib/binary_tree.h"

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL 876
#endif

/**
 * @struct campoAlbero
 * @brief Struttura per mappare un campo a un albero di valori.
 *
 * Questa struttura è utilizzata per associare un nome di campo a un albero binario di ricerca che memorizza
 * i valori specifici per quel campo. Ogni nodo dell'albero rappresenta un valore unico per il campo, consentendo
 * una ricerca efficiente dei valori associati.
 *
 * @param nomeCampo
 * Nome del campo. Questa stringa identifica il campo di cui l'albero di valori mantiene traccia.
 *
 * @param alberoValori
 * Albero binario di ricerca che contiene i valori associati al campo. Ogni valore è unico all'interno dell'albero,
 * e l'albero permette operazioni di ricerca, inserimento e cancellazione efficienti per gestire i valori del campo.
 */
struct campoAlbero
{
    char *nomeCampo;
    struct binary_tree alberoValori;
};

/**
 * @struct valoreLibro
 * @brief Struttura per mappare un valore di un campo a un libro specifico.
 *
 * Questa struttura collega un valore specifico di un campo a un libro, permettendo di associare
 * direttamente un valore (ad esempio, un autore o un genere) a un libro nella collezione. Viene utilizzata
 * principalmente all'interno degli alberi binari di ricerca per memorizzare e recuperare rapidamente i libri
 * basati sui valori dei loro campi.
 *
 * @param valoreCampo
 * Il valore specifico del campo. Questa stringa rappresenta il valore del campo (ad esempio, il nome di un autore)
 * a cui il libro è associato.
 *
 * @param libroAssociato
 * Puntatore a un libro. Questo puntatore riferisce a una struttura libro che contiene tutti i dettagli
 * del libro associato a questo valore del campo.
 */
struct valoreLibro
{
    char *valoreCampo;
    struct libro *libroAssociato;
};

/**
 * @brief Aggiunge un libro all'array dinamico specificato di elementi `campoAlbero`.
 *
 * Questa funzione analizza la rappresentazione stringa di un libro, estrae coppie chiave-valore e le aggiunge
 * all'array dinamico.
 *
 * @return int SUCCESS se l'operazione è riuscita, ERR_SYSTEM_CALL in caso di errore.
 */
int arrCampi_aggiungiLibro(struct dynamic_array *arrayCampi, struct libro *libro);

/**
 * @brief Genera una lista di libri che soddisfano una data richiesta, verificando ogni libro completamente.
 *
 * Analizza la richiesta per identificare un campo e un valore iniziale di ricerca, trova i libri potenzialmente corrispondenti
 * nell'array di `campoAlbero` basandosi su quel singolo campo e valore. Per ogni libro trovato, effettua un ulteriore controllo
 * confrontando l'intero libro con la richiesta completa per assicurarsi che soddisfi tutti i criteri specificati.
 * I libri che passano questo controllo aggiuntivo vengono aggiunti a `lista_libri`.
 *
 *  @return int SUCCESS se l'operazione è riuscita, ERR_SYSTEM_CALL in caso di errore.
 */
int arrCampi_generaLista(struct dynamic_array *arrayCampi, struct dynamic_array *lista_libri, const char *richiesta, pthread_mutex_t *mutex, pthread_cond_t *cond);

/**
 * @brief Libera l'intero array dinamico di elementi `campoAlbero`.
 *
 * Questa funzione itera sull'array dinamico e libera ogni elemento `campoAlbero` insieme al suo albero binario
 * di elementi `valoreLibro` associato.
 */
void arrCampi_free(struct dynamic_array *arrayCampi);

#endif