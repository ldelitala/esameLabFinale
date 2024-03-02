/**
 * Library Name: reader-writer2
 * Author: LORENZO DELITALA
 * ----------------------------------
 * This library contains the necessary functions to handle a type 2 writer-reader problem,
 * where the writer has precedence over the reader. It includes semaphore operations
 * for access and exit of writers and readers in a concurrent environment.
 */

#include <semaphore.h>
#include <errno.h>

#ifndef READERS_WRITERS2_H
#define READERS_WRITERS2_H

// Semaphore names
#ifndef RW2_READERS_VAR
#define RW2_READERS_VAR "rw2_readers_var"
#endif

#ifndef RW2_WRITERS_VAR
#define RW2_WRITERS_VAR "rw2_writers_var"
#endif

#ifndef RW2_READERS_MUTEX
#define RW2_READERS_MUTEX "rw2_readers_mutex"
#endif

#ifndef RW2_WRITERS_MUTEX
#define RW2_WRITERS_MUTEX "rw2_writers_mutex"
#endif

#ifndef RW2_READ_ATTEMPT_MUTEX
#define RW2_READ_ATTEMPT_MUTEX "rw2_read_attempt_mutex"
#endif

#ifndef RW2_RESOURCE_MUTEX
#define RW2_RESOURCE_MUTEX "rw2_resource_mutex"
#endif

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

//! SEMAPHORE FUNCTIONS

/**
 * @brief Opens multiple semaphores for the type 2 writer-reader problem.
 *
 * This function initializes semaphores for controlling access and synchronization in a reader-writer
 * scenario. It uses predefined names for each semaphore (RW2_READERS_VAR, RW2_READERS_MUTEX, RW2_WRITERS_VAR,
 * RW2_WRITERS_MUTEX, RW2_READ_ATTEMPT_MUTEX, RW2_RESOURCE_MUTEX). Pass NULL to any semaphore that does not need
 * to be opened.
 *
 * @param readers_var Pointer to the semaphore controlling the number of readers.
 * @param readers_mutex Pointer to the mutex for managing readers' access.
 * @param writers_var Pointer to the semaphore controlling the number of writers.
 * @param writers_mutex Pointer to the mutex for managing writers' access.
 * @param read_attempt_mutex Pointer to the mutex for managing read attempts.
 * @param resource_mutex Pointer to the mutex for accessing the shared resource.
 *
 * @return SUCCESS on successful opening of all semaphores, ERR_SYSTEM_CALL on failure.
 */
int rw2_openSemaphores(sem_t **readers_var, sem_t **readers_mutex, sem_t **writers_var, sem_t **writers_mutex, sem_t **read_attempt_mutex, sem_t **resource_mutex);

/**
 * @brief Closes semaphores used in the type 2 writer-reader problem. This function closes each semaphore
 * involved in the synchronization of readers and writers. It ensures each semaphore is properly closed using
 * the `close_and_check` function. If a semaphore fails to close, the function reports the error and terminates,
 * returning an error code. Pass NULL to any semaphore that does not need
 * to be closed.
 *
 * @param readers_var Pointer to the semaphore controlling the number of readers.
 * @param readers_mutex Pointer to the mutex for managing readers' access.
 * @param writers_var Pointer to the semaphore controlling the number of writers.
 * @param writers_mutex Pointer to the mutex for managing writers' access.
 * @param read_attempt_mutex Pointer to the mutex for managing read attempts.
 * @param resource_mutex Pointer to the mutex for accessing the shared resource.
 *
 * @return `SUCCESS` if all semaphores are successfully closed, `ERR_SYSTEM_CALL` if any semaphore closure fails.
 */
int rw2_closeSemaphores(sem_t *readers_var, sem_t *readers_mutex, sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex);

/**
 * @brief Unlinks semaphores used in the type 2 writer-reader problem.
 * It leverages the `unlink_single_semaphore` function for each semaphore identified by its predefined
 * name (RW2_READERS_VAR, RW2_READERS_MUTEX, RW2_WRITERS_VAR, RW2_WRITERS_MUTEX, RW2_READ_ATTEMPT_MUTEX, RW2_RESOURCE_MUTEX).
 * The function ensures each semaphore is unlinked correctly and reports errors if any unlink operation fails.
 *
 * @return 0 if all semaphores are successfully unlinked, -1 if any semaphore unlinking fails.
 */
int rw2_unlinkSemaphores();

//! Functions for Type 2 Writer-Reader Problem

/**
 * @brief Manages writer access in a readers-writers type 2 problem using semaphores.
 *
 * It uses semaphores to coordinate access, ensuring that writers
 * have exclusive and priority access to the resource when they are writing. The function locks and
 * unlocks the necessary semaphores to manage writer access. If any semaphore operation
 * fails, the function prints an error.
 *
 * @param writers_var Semaphore that tracks the number of writers.
 * @param writers_mutex Semaphore to ensure mutual exclusion when modifying writers_var.
 * @param read_attempt_mutex Semaphore to prevent readers from attempting to access
 *        the resource while a writer is waiting or writing.
 * @param resource_mutex Semaphore to control access to the shared resource.
 *
 * @return Returns `SUCCESS` on successful coordination of writer access, and `ERR_SYSTEM_CALL` on error.
 */
int rw2_writerAccess(sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex);

/**
 * @brief Manages writer exit in a readers-writers type 2 problem using semaphores.
 *
 * It adjusts the relevant semaphore values to indicate
 * that the writer has finished its operations and releases the resource. This involves
 * unlocking the resource mutex and decrementing the writers count. If the writer is the last
 * one, it also signals that readers can attempt to access the resource. The function ensures
 * proper semaphore operations for safe exit of a writer. In case of a semaphore operation
 * failure, it prints an error.
 *
 * @param writers_var Semaphore that tracks the number of writers.
 * @param writers_mutex Semaphore to ensure mutual exclusion when modifying writers_var.
 * @param read_attempt_mutex Semaphore to allow readers to attempt access
 *        to the resource when no writers are present.
 * @param resource_mutex Semaphore to control access to the shared resource.
 *
 * @return Returns `SUCCESS` on successful release of writer access, and ERR_SYSTEM_CALL` on error.
 */
int rw2_writerExit(sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex);

/**
 * @brief Manages reader access in a readers-writers type 2 problem using semaphores.
 *
 * It coordinates the access using semaphores to ensure that multiple readers can read
 * simultaneously, but not while a writer is writing.
 * If the reader is the first one, it locks the resource mutex. Any failure in semaphore
 * operations results in an error return value.
 *
 * @param readers_var Semaphore that tracks the number of active readers.
 * @param readers_mutex Semaphore to ensure mutual exclusion when modifying readers_var.
 * @param read_attempt_mutex Semaphore to prevent readers from attempting to access
 *        the resource while a writer is waiting.
 * @param resource_mutex Semaphore to control access to the shared resource.
 *
 * @return Returns `SUCCESS` on successful coordination of reader access, and `ERR_SYSTEM_CALL` on error.
 */
int rw2_readerAccess(sem_t *readers_var, sem_t *readers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex);

/**
 * @brief Manages reader exit in a readers-writers type 2 problem using semaphores.
 *
 * It manages the semaphore values to decrement the
 * reader count and, if necessary, signals that the resource is available for writing. The
 * function ensures mutual exclusion when modifying the reader count and releases the resource
 * mutex if the exiting reader is the last one. If any semaphore operation fails, the function
 * returns an error.
 *
 * @param readers_var Semaphore that tracks the number of active readers.
 * @param readers_mutex Semaphore to ensure mutual exclusion when modifying the readers count.
 * @param resource_mutex Semaphore to control access to the shared resource, especially for writers.
 *
 * @return Returns `SUCCESS` on successful release of reader access, and `ERR_SYSTEM_CALL` on error.
 */
int rw2_readerExit(sem_t *readers_var, sem_t *readers_mutex, sem_t *resource_mutex);

#endif
