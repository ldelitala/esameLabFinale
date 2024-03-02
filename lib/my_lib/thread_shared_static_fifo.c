#include "../../include/my_lib/thread_shared_static_fifo.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int fifost_threadSafePut(struct static_fifo *object, const void *element, pthread_mutex_t *mutex, sem_t *free_space, sem_t *num_elements)
{
    int error;

    if (sem_wait(free_space) == -1)
    {
        perror("Errore nell'attesa di spazio libero (sem_wait su free_space)");
        return FREE_SPACE_FAILURE;
    }

    error = pthread_mutex_lock(mutex);
    if (error)
    {
        printf("Errore nella lock del mutex: %s\n", strerror(error));
        sem_post(free_space); // Release the semaphore if mutex lock fails to maintain correct semaphore count
        return MUTEX_FAILURE;
    }

    if (fifost_enqueue(object, element) == 1)
    {
        pthread_mutex_unlock(mutex); // Avoiding sem_post(free_space) here because the fifo queue is actually full
        return SEM_COORDINATION_FAILURE;
    }

    error = pthread_mutex_unlock(mutex);
    if (error)
    {
        printf("Errore nella unlock del mutex: %s\n", strerror(error));
        sem_post(num_elements); // If unlocking fails, it still increments num_elements to avoid losing the signal of added element
        return MUTEX_FAILURE;
    }

    if (sem_post(num_elements) == -1)
    {
        perror("Errore nel segnalare un nuovo elemento (sem_post su num_elements)");
        return NUM_ELEMENTS_FAILURE;
    }

    return SUCCESS;
}

int fifost_threadSafeGet(struct static_fifo *queue, void *buffer, pthread_mutex_t *mutex, sem_t *free_space, sem_t *num_elements)
{
    int error;

    if (sem_wait(num_elements) == -1)
    {
        perror("Errore nell'attesa di un elemento (sem_wait su num_elements)");
        return NUM_ELEMENTS_FAILURE;
    }

    error = pthread_mutex_lock(mutex);
    if (error)
    {
        printf("Errore nella lock del mutex: %s\n", strerror(error));
        sem_post(num_elements); // Release the semaphore if mutex lock fails to maintain correct semaphore count
        return MUTEX_FAILURE;
    }

    if (fifost_dequeue(queue, buffer) == 1)
    {
        pthread_mutex_unlock(mutex); // No need to adjust num_elements here, as the fifo queue is actually empty
        return SEM_COORDINATION_FAILURE;
    }

    error = pthread_mutex_unlock(mutex);
    if (error)
    {
        printf("Errore nella unlock del mutex: %s\n", strerror(error));
        sem_post(free_space); // If unlocking fails, it still increments num_elements to avoid losing the signal of added element
        return MUTEX_FAILURE;
    }

    if (sem_post(free_space) == -1)
    {
        perror("Errore nel segnalare spazio disponibile (sem_post su free_space)");
        return FREE_SPACE_FAILURE;
    }

    return SUCCESS;
}
