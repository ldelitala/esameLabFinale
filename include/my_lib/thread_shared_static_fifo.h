#ifndef THREAD_SHARED_STATIC_FIFO_H
#define THREAD_SHARED_STATIC_FIFO_H

#include "static_fifo.h"
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FREE_SPACE_FAILURE
#define FREE_SPACE_FAILURE 1
#endif

#ifndef NUM_ELEMENTS_FAILURE
#define NUM_ELEMENTS_FAILURE 2
#endif

#ifndef MUTEX_FAILURE
#define MUTEX_FAILURE 3
#endif

#ifndef SEM_COORDINATION_FAILURE
#define SEM_COORDINATION_FAILURE 4
#endif

/**
 * Adds an element to the FIFO queue in a thread-safe manner.
 * This function uses a semaphore to wait for free space in the queue, locks a mutex to ensure exclusive access
 * to the queue during the operation, and then signals another semaphore to indicate that a new element has been added.
 *
 * @param free_space A semaphore that tracks the available space in the queue. This function waits on this semaphore
 *        to ensure there is space before adding an element.
 * @param num_elements A semaphore that tracks the number of elements in the queue. This function posts to this semaphore
 *        after successfully adding an element to signal that the queue size has increased.
 *
 * @return Returns SUCCESS if the element is successfully added. Returns FREE_SPACE_FAILURE if waiting for free space fails,
 *         MUTEX_FAILURE if acquiring or releasing the mutex fails, NUM_ELEMENTS_FAILURE if post of num_elements fails, and
 *         SEM_COORDINATION_FAILURE if adding the element to the queue fails due to full capacity, indicating a potential semaphore
 *         coordination issue.
 *
 * @warning From great power comes great responsabity. The size of the element must match the size specified
 *          at queue initialization. Size mismatches can lead to undefined behavior.
 *
 * @note Memory of dequeued elements is not explicitly cleared for efficiency reasons. Access to these memory locations through fifo_dequeue is
 *       prevented by using internal variables to track the positions of valid elements. This ensures that while the memory for dequeued elements
 *       is not cleared, it is effectively inaccessible, safeguarding against unintended data access and maintaining the integrity of the queue operations.
 */
int fifost_threadSafePut(struct static_fifo *object, const void *element, pthread_mutex_t *mutex, sem_t *free_space, sem_t *num_elements);

/**
 * Retrieves and removes an element from the FIFO queue in a thread-safe manner.
 * This function uses a semaphore to wait for available elements in the queue, locks a mutex to ensure exclusive access
 * during the operation, and then signals another semaphore to indicate that space has been freed up.
 *
 * @param free_space A semaphore that tracks the available space in the queue. This function posts to this semaphore
 *        after successfully removing an element to signal that the queue space has increased.
 * @param num_elements A semaphore that tracks the number of elements in the queue. This function waits on this semaphore
 *        to ensure there is an element to remove.
 *
 * @return Returns SUCCESS if the element is successfully removed, NUM_ELEMENTS_FAILURE if waiting for an element fails,
 *         MUTEX_FAILURE if acquiring or releasing the mutex fails,FREE_SPACE_FAILURE if post of free_space fails, and
 *         SEM_COORDINATION_FAILURE if removing the element from the queue fails due to an empty queue, indicating a potential
 *         semaphore coordination issue.
 *
 * @warning Ensure that the memory allocated for `buffer` is sufficient to hold an element of the size specified during
 *          the FIFO queue initialization. Providing a buffer with insufficient memory can lead to buffer overflow and
 *          undefined behavior.
 */
int fifost_threadSafeGet(struct static_fifo *object, void *buffer, pthread_mutex_t *mutex, sem_t *free_space, sem_t *num_elements);

#endif