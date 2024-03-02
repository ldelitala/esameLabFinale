#include "../../include/comunicazione/coda_condivisa.h"
#include <stdio.h>

int cc_crea(struct coda_condivisa *coda, size_t dimensioneCoda)
{
    (*coda) = (struct coda_condivisa){.fifo = fifost_create(dimensioneCoda, sizeof(struct elementoCoda)),
                                      .mutex = PTHREAD_MUTEX_INITIALIZER};
    if (!((coda->fifo).fifost_queue))
    {
        perror("Creazione coda condivisa fallita");
        goto distruggi_mutex;
    }

    if (sem_init(&(coda->numero_elementi), 0, 0) == -1)
    {
        perror("Errore init semaforo");
        goto distruggi_coda;
    }

    if (sem_init(&(coda->spazio_libero), 0, dimensioneCoda) == -1)
    {
        perror("Errore init semaforo");
        goto distruggi_numero_elementi;
    }

    return SUCCESS;

distruggi_numero_elementi:
    sem_destroy(&(coda->numero_elementi));

distruggi_coda:
    fifost_destroy(&(coda->fifo));

distruggi_mutex:
    pthread_mutex_destroy(&(coda->mutex));

    return ERR_SYSTEM_CALL;
}

int cc_put(struct coda_condivisa *coda, const struct elementoCoda *elemento)
{
    int error = fifost_threadSafePut(&(coda->fifo), elemento, &(coda->mutex), &(coda->spazio_libero), &(coda->numero_elementi));
    return (error == SUCCESS) ? SUCCESS : ERR_SYSTEM_CALL;
}

int cc_get(struct coda_condivisa *coda, struct elementoCoda *buffer)
{
    int error = fifost_threadSafeGet(&(coda->fifo), buffer, &(coda->mutex), &(coda->spazio_libero), &(coda->numero_elementi));
    return (error == SUCCESS) ? SUCCESS : ERR_SYSTEM_CALL;
}

void cc_destroy(struct coda_condivisa *coda)
{
    fifost_destroy(&(coda->fifo));

    sem_destroy(&(coda->numero_elementi));
    sem_destroy(&(coda->spazio_libero));

    pthread_mutex_destroy(&(coda->mutex));
}