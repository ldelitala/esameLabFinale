#include "../../include/my_lib/readers_writers2.h"
#include <stdio.h>     // Standard input/output library, used for reading and writing operations on files and console.
#include <stdlib.h>    // Standard library providing functions for managing dynamic memory, such as malloc() and free().
#include <errno.h>     // Standard library that provides the "errno" variable for error handling.
#include <semaphore.h> // Library for semaphore management used for synchronization between processes.
#include <fcntl.h>     // Library providing additional options for file opening.
#include <sys/stat.h>  // Library for obtaining information about the status of files and directories.

struct semaphoreInfo
{
    sem_t *semaphore;
    const char *name;
};

//! INTERNAL FUNCTIONS

/**
 * @brief Opens or creates a single semaphore with the specified name and initial value.
 *
 * This function attempts to open a semaphore. If the semaphore does not exist, it is created
 * with the specified initial value. The function handles the opening or creation process and
 * reports any errors encountered.
 *
 * @param semaphore A pointer to a sem_t pointer where the opened or created semaphore will be stored.
 * @param name The name of the semaphore to be opened or created.
 * @param value The initial value for the semaphore if it is being created.
 *
 * @return Returns SUCCESS on successful opening or creation of the semaphore. Returns ERR_SYSTEM_CALL if an error occurs.
 */
int open_single_semaphore(sem_t **semaphore, const char *name, unsigned int value)
{
    if (semaphore)
    {
        *semaphore = sem_open(name, O_CREAT, S_IRWXU, value);
        if (*semaphore == SEM_FAILED)
        {
            perror("Error: unable to obtain the semaphore ");
            fprintf(stderr, "Semaphore name: %s\n", name);
            return ERR_SYSTEM_CALL;
        }
    }
    return SUCCESS;
}

/**
 * @brief Closes the specified semaphore and checks for errors during the closure.
 *
 * This function attempts to close the given semaphore. It checks if the closure operation
 * was successful and reports any errors that may occur during the process.
 *
 * @param sem The semaphore to be closed.
 * @param name The name of the semaphore, used for error reporting.
 *
 * @return Returns SUCCESS if the semaphore is successfully closed. Returns ERR_SYSTEM_CALL if an error occurs during closure.
 */
int close_and_check(sem_t *sem, const char *name)
{
    if (sem && sem != SEM_FAILED && sem_close(sem) == -1)
    {
        sem = SEM_FAILED;
        perror("Error: closing semaphore failed");
        fprintf(stderr, "Semaphore: %s\n", name);
        return ERR_SYSTEM_CALL;
    }
    return SUCCESS;
}

/**
 * @brief Unlinks a named semaphore.
 *
 * This function attempts to unlink (remove) a named semaphore. If the semaphore does not exist
 * (indicated by the ENOENT error), it is not considered an error. Any other failure in the unlinking
 * process is reported.
 *
 * @param name The name of the semaphore to be unlinked.
 *
 * @return Returns `SUCCESS` if the semaphore is successfully unlinked or does not exist.
 * Returns `ERR_SYSTEM_CALL` if an error occurs during the unlinking process.
 */
int unlink_single_semaphore(const char *name)
{
    if (sem_unlink(name) == -1 && errno != ENOENT)
    {
        perror("Error: sem_unlink call failed for semaphore ");
        fprintf(stderr, "Semaphore name: %s\n", name);
        return ERR_SYSTEM_CALL;
    }
    return SUCCESS;
}

//! FUNCTIONS FOR SEMAPHORES

int rw2_openSemaphores(sem_t **readers_var, sem_t **readers_mutex, sem_t **writers_var, sem_t **writers_mutex, sem_t **read_attempt_mutex, sem_t **resource_mutex)
{
    if (open_single_semaphore(readers_var, RW2_READERS_VAR, 0) == ERR_SYSTEM_CALL ||
        open_single_semaphore(writers_var, RW2_WRITERS_VAR, 0) == ERR_SYSTEM_CALL ||
        open_single_semaphore(readers_mutex, RW2_READERS_MUTEX, 1) == ERR_SYSTEM_CALL ||
        open_single_semaphore(writers_mutex, RW2_WRITERS_MUTEX, 1) == ERR_SYSTEM_CALL ||
        open_single_semaphore(read_attempt_mutex, RW2_READ_ATTEMPT_MUTEX, 1) == ERR_SYSTEM_CALL ||
        open_single_semaphore(resource_mutex, RW2_RESOURCE_MUTEX, 1) == ERR_SYSTEM_CALL)
    {
        rw2_closeSemaphores(readers_var ? *readers_var : NULL,
                            readers_mutex ? *readers_mutex : NULL,
                            writers_var ? *writers_var : NULL,
                            writers_mutex ? *writers_mutex : NULL,
                            read_attempt_mutex ? *read_attempt_mutex : NULL,
                            resource_mutex ? *resource_mutex : NULL);
        rw2_unlinkSemaphores();
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}

int rw2_closeSemaphores(sem_t *readers_var, sem_t *readers_mutex, sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex)
{
    struct semaphoreInfo semaphores[] = {
        {readers_var, RW2_READERS_VAR},
        {readers_mutex, RW2_READERS_MUTEX},
        {writers_var, RW2_WRITERS_VAR},
        {writers_mutex, RW2_WRITERS_MUTEX},
        {read_attempt_mutex, RW2_READ_ATTEMPT_MUTEX},
        {resource_mutex, RW2_RESOURCE_MUTEX}};

    int result = SUCCESS;
    for (int i = 0; i < sizeof(semaphores) / sizeof(semaphores[0]); i++)
    {
        if (close_and_check(semaphores[i].semaphore, semaphores[i].name) == ERR_SYSTEM_CALL)
            result = ERR_SYSTEM_CALL;
    }

    return result;
}

int rw2_unlinkSemaphores()
{
    const char *semaphoreNames[] = {
        RW2_READERS_VAR,
        RW2_WRITERS_VAR,
        RW2_READERS_MUTEX,
        RW2_WRITERS_MUTEX,
        RW2_READ_ATTEMPT_MUTEX,
        RW2_RESOURCE_MUTEX};

    int result = SUCCESS;
    for (int i = 0; i < sizeof(semaphoreNames) / sizeof(semaphoreNames[0]); i++)
    {
        if (unlink_single_semaphore(semaphoreNames[i]) == ERR_SYSTEM_CALL)
            result = ERR_SYSTEM_CALL;
    }

    return result;
}

//! Functions for Type 2 Writer-Reader Problem

// lock(writers_mutex)
// writers++
// if(writers==1)
//      lock(read_attempt_mutex)
// unlock(writers_mutex)
// lock(resource_mutex)
int rw2_writerAccess(sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex)
{
    int val;

    if (sem_wait(writers_mutex) == -1) // lock(writers_mutex)
        return ERR_SYSTEM_CALL;

    if (sem_post(writers_var) == -1) // writers++
        goto cleanup_writers_mutex;

    if ((sem_getvalue(writers_var, &val) == -1) ||            // if(writers==1)
        ((val == 1) && (sem_wait(read_attempt_mutex) == -1))) // lock(read_attempt_mutex)
        goto cleanup_writers_var_decrement;

    if ((sem_post(writers_mutex) == -1) || // unlock(writers_mutex)
        (sem_wait(resource_mutex) == -1))  // lock(resource_mutex)

        // Se sem_post fallisce qui, abbiamo un problema più serio
        // ma per consistenza, seguiamo lo stesso schema di gestione degli errori
        goto cleanup_read_attempt_mutex;

    return SUCCESS;

cleanup_read_attempt_mutex:
    if (val == 1)
        sem_post(read_attempt_mutex); // Best effort per rilasciare il lock

cleanup_writers_var_decrement:
    sem_wait(writers_var); // Tentativo di decrementare writers_var

cleanup_writers_mutex:
    sem_post(writers_mutex); // Sblocca writers_mutex

    return ERR_SYSTEM_CALL;
}

// unlock(resurce_mutex)
// lock(writers_mutex)
// writers--
// if(writers==0)
//     unlock(read_attempt_mutex)
// unlock(writers_mutex)
int rw2_writerExit(sem_t *writers_var, sem_t *writers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex)
{
    int val, error_occurred = 0;

    // unlock(resurce_mutex)
    if (sem_post(resource_mutex) == -1)
        // If unlocking resource_mutex fails, report and exit immediately.
        return ERR_SYSTEM_CALL;

    // lock(writers_mutex)
    if (sem_wait(writers_mutex) == -1)
        // If locking writers_mutex fails, nothing more can be safely done.
        return ERR_SYSTEM_CALL;

    // writers--
    if (sem_wait(writers_var) == -1)
        goto cleanup_writers_mutex;

    // if(writers==0)
    if (sem_getvalue(writers_var, &val) == -1)
        goto cleanup_increment_writers_var;

    //     unlock(read_attempt_mutex)
    if ((val == 0) && (sem_post(read_attempt_mutex) == -1))
        error_occurred = 1; // Mark error, but can't rollback this action, proceed to unlock writers_mutex

    if (sem_post(writers_mutex) == -1)
        // If unlocking writers_mutex fails, note the error but can't do much about it here.
        error_occurred = 1; // Mark error

    // If there was any error after the initial steps, return ERR_SYSTEM_CALL
    return (error_occurred) ? ERR_SYSTEM_CALL : SUCCESS;

cleanup_increment_writers_var:
    sem_post(writers_var); // Attempt to increment writers_var to rollback the decrement

cleanup_writers_mutex:
    sem_post(writers_mutex); // Unlock writers_mutex regardless of the state to ensure no deadlock

    return ERR_SYSTEM_CALL;
}

// lock(read_attemp_mutex)
// lock(readers_mutex)
// readers_var++
// if(readers_var==1)
//     lock(resource_mutex)
// unlock(readers_mutex)
// unlock(tentativo_lettura)
int rw2_readerAccess(sem_t *readers_var, sem_t *readers_mutex, sem_t *read_attempt_mutex, sem_t *resource_mutex)
{
    int val;

    // lock(read_attemp_mutex)
    if (sem_wait(read_attempt_mutex) == -1)
        return ERR_SYSTEM_CALL;

    // lock(readers_mutex)
    if (sem_wait(readers_mutex) == -1)
        goto cleanup_read_attempt_mutex;

    // readers_var++
    if (sem_post(readers_var) == -1)
        goto cleanup_readers_mutex;

    // if(readers_var==1)
    //     lock(resource_mutex)
    if ((sem_getvalue(readers_var, &val) == -1) ||
        ((val == 1) && (sem_wait(resource_mutex) == -1)))
        goto cleanup_decrement_readers_var;

    // unlock(readers_mutex)
    // unlock(tentativo_lettura)
    if (sem_post(readers_mutex) == -1 ||
        sem_post(read_attempt_mutex) == -1)
        // Se sem_post fallisce qui, abbiamo un problema più serio
        // ma per consistenza, seguiamo lo stesso schema di gestione degli errori
        goto cleanup_decrement_readers_var;

    return SUCCESS;

cleanup_decrement_readers_var:
    sem_wait(readers_var); // Attempt to decrement readers_var to rollback the increment

cleanup_readers_mutex:
    sem_post(readers_mutex); // Unlock readers_mutex

cleanup_read_attempt_mutex:
    sem_post(read_attempt_mutex); // Unlock read_attempt_mutex

    return ERR_SYSTEM_CALL;
}

// lock(readers_mutex)
// readers_var--
// if(readers_var==0)
//     unlock(resource_mutex)
// unlock(readers_mutex)
int rw2_readerExit(sem_t *readers_var, sem_t *readers_mutex, sem_t *resource_mutex)
{
    int val, error_occurred = 0;

    // lock(readers_mutex)
    if (sem_wait(readers_mutex) == -1)
        return ERR_SYSTEM_CALL;

    // readers_var--
    if (sem_wait(readers_var) == -1)
        goto cleanup_readers_mutex;

    // if(readers_var==0)
    //     unlock(resource_mutex)
    if (sem_getvalue(readers_var, &val) == -1)
        goto cleanup_increment_readers_var;

    if ((val == 0) && sem_post(resource_mutex) == -1)
        error_occurred = 1; // Note: At this point, cannot rollback the decrement of readers_var

    sem_post(readers_mutex);

    return (error_occurred) ? ERR_SYSTEM_CALL : SUCCESS;

cleanup_increment_readers_var:
    sem_post(readers_var); // Attempt to increment readers_var to rollback the decrement

cleanup_readers_mutex:
    sem_post(readers_mutex); // Unlock readers_mutex

    return ERR_SYSTEM_CALL;
}
