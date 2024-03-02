#ifndef CODA_CONDIVISA_H
#define CODA_CONDIVISA_H

#include <pthread.h>
#include <semaphore.h>

#include "protocollo_comunicazione.h"
#include "../my_lib/thread_shared_static_fifo.h"

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -1
#endif

/**
 * @struct elementoCoda
 * @brief Struttura per rappresentare un elemento da inserire in una coda di messaggi.
 *
 * La struttura `elementoCoda` è utilizzata per gestire le richieste dei clienti in un'applicazione
 * server. Ogni elemento della coda contiene un messaggio di richiesta e un identificatore del
 * file descriptor del client, permettendo al server di processare le richieste e rispondere al
 * client corretto.
 *
 * @param richiesta
 * Un `struct messaggio` che contiene i dettagli della richiesta inviata dal client.
 *
 * @param client_fd
 * File descriptor del socket associato al client che ha inviato la richiesta. Questo identificatore
 * viene utilizzato per inviare la risposta al client appropriato.
 */
struct elementoCoda
{
    struct messaggio richiesta;
    int client_fd;
};

/**
 * @struct coda_condivisa
 * @brief Struttura per rappresentare una coda condivisa tra thread.
 *
 * @param fifo
 * Una struttura `static_fifo` che funge da buffer per memorizzare gli elementi della coda.
 *
 * @param mutex
 * utilizzato per sincronizzare l'accesso alla coda tra thread differenti, assicurando
 * che solo un thread alla volta possa modificare la coda.
 *
 * @param spazio_libero
 * Un semaforo `sem_t` che tiene traccia dello spazio disponibile nella coda. Viene utilizzato per bloccare
 * i thread produttori quando la coda è piena.
 *
 * @param numero_elementi
 * Un semaforo `sem_t` che tiene traccia del numero di elementi presenti nella coda. Viene utilizzato per
 * bloccare i thread consumatori quando la coda è vuota.
 */
struct coda_condivisa
{
    struct static_fifo fifo;
    pthread_mutex_t mutex;
    sem_t spazio_libero, numero_elementi;
};

/**
 * Inizializza uno `struct coda_condivisa` con una dimensione specificata.
 *
 * @return Restituisce `SUCCESS` se la coda è stata creata e inizializzata con successo. Restituisce `ERR_SYSTEM_CALL`
 *         se si verifica un errore durante la creazione o l'inizializzazione dei componenti della coda, dopo aver eseguito
 *         la pulizia di tutte le risorse che erano state allocate con successo.
 */
int cc_crea(struct coda_condivisa *coda, size_t dimensioneCoda);

/**
 * Aggiunge un elemento alla coda condivisa in modo thread-safe.
 *
 * @return SUCCESS se aggiunto con successo, altrimenti ERR_SYSTEM_CALL.
 */
int cc_put(struct coda_condivisa *coda, const struct elementoCoda *elemento);

/**
 * Prende un elemento dalla coda condivisa in modo thread-safe.
 *
 * @return SUCCESS se aggiunto con successo, altrimenti ERR_SYSTEM_CALL.
 */
int cc_get(struct coda_condivisa *coda, struct elementoCoda *buffer);

void cc_destroy(struct coda_condivisa *coda);

#endif