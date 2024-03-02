#ifndef STATIC_FIFO_H
#define STATIC_FIFO_H

#include <errno.h>

#include "dynamic_array.h"

/**
 * @param first_element Index of the first empty slot in the queue where a new element can be enqueued.
 * @param last_element Index of the last element that was enqueued in the queue. This is where the next
 * dequeue operation will retrieve the element from.
 * @param fifo_length The total capacity of the FIFO queue, indicating the maximum number of elements
 * it can hold at any given time.
 * @param queue Pointer to the dynamic array structure that actually stores the elements of the queue.
 *
 * @note The indices `first_element` and `last_element` are managed in a circular manner to utilize the
 * array efficiently, wrapping around when they reach the end of the allocated space. This design
 * enables the FIFO to function properly even when elements are continuously enqueued and dequeued.
 */
struct static_fifo
{
    size_t fifost_length;
    int fifost_first_element, fifost_last_element;
    struct dynamic_array *fifost_queue;
};

/**
 * Initializes a FIFO queue with specified length and element size.
 *
 * @return A struct static_fifo representing the initialized queue.
 * @warning From great power comes great responsibility. If allocation fails, the returned fifo.queue will be NULL.
 */
struct static_fifo fifost_create(size_t fifo_length, size_t element_size);

/**
 * Checks if the FIFO queue is empty.
 *
 * @return 1 if empty, 0 otherwise.
 */
int fifost_isEmpty(struct static_fifo *object);

/**
 * Checks if the FIFO queue is full.
 *
 * @return 1 if full, 0 otherwise.
 */
int fifost_isFull(struct static_fifo *object);

/**
 * Counts the number of elements in the FIFO queue.
 *
 * @return The number of elements in the queue.
 */
int fifost_count(struct static_fifo *object);

/**
 * Adds an element to a FIFO queue.
 *
 * @return 0 if the element is successfully added, 1 if the queue is full.
 *
 * @warning From great power comes great responsabity. The size of the element must match the size specified
 *          at queue initialization. Size mismatches can lead to undefined behavior.
 *
 * @note Memory of dequeued elements is not explicitly cleared for efficiency reasons. Access to these memory locations through fifo_dequeue is
 *       prevented by using internal variables to track the positions of valid elements. This ensures that while the memory for dequeued elements
 *       is not cleared, it is effectively inaccessible, safeguarding against unintended data access and maintaining the integrity of the queue operations.
 */
int fifost_enqueue(struct static_fifo *object, const void *element);

/**
 * Removes and retrieves the oldest element from a FIFO queue.
 *
 * @return 0 if the element is successfully dequeued and stored in `buffer`, 1 if the queue is empty.
 *
 * @warning Ensure that the memory allocated for `buffer` is sufficient to hold an element of the size specified during
 * the FIFO queue initialization. Providing a buffer with insufficient memory can lead to buffer overflow and undefined behavior.
 */
int fifost_dequeue(struct static_fifo *object, void *buffer);

/**
 * Peeks at the next element in the FIFO queue without removing it.
 *
 * @return Pointer to the next element, or NULL if the queue is empty.
 */
void *fifost_peek(struct static_fifo *object);

/**
 * Clears the FIFO queue without deallocating it.
 */
void fifost_clean(struct static_fifo *object);

/**
 * Destroys the FIFO queue and deallocates its memory.
 */
void fifost_destroy(struct static_fifo *object);

#endif