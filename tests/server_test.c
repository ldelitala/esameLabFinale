#include "../include/socket_comunication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <pthread.h>

#define MSG_STOP 'S'

char socketServerPath[108] = "sockets/server_socket";
struct pollfd poll_fds[MAX_CLIENTS + 1];
struct coda_condivisa *ptrCoda = NULL;
pthread_t *workers = NULL;
size_t numeroWorkers = 0;

void chiudiWorkers()
{
    struct elementoCoda msgStop;
    msgStop.richiesta.type = MSG_STOP;

    for (int i = 0; i < numeroWorkers; i++)
    {
        cc_put(ptrCoda, &msgStop);
    }

    for (int i = 0; i < numeroWorkers; i++)
    {
        if (pthread_join(workers[i], NULL) == -1)
            printf("Errore durante la chiamata a pthread_join\nImpossibile chiudere i thread worker correttamente,\nsperiamo che il sistema operativo se la gestisca\n");
    }

    if (workers)
        free(workers);
}

void cleanupAndExit()
{
    shutdown(poll_fds[0].fd, SHUT_RDWR);
    close(poll_fds[0].fd);
    printf("distruggo socket\n");
    perror("");
    unlink(socketServerPath);
    perror("distrutta?");

    for (int i = 1; i < MAX_CLIENTS + 1; i++)
    {
        if (poll_fds[i].fd != -1)
        {
            shutdown(poll_fds[i].fd, SHUT_RDWR);
            close(poll_fds[i].fd);
        }
    }

    chiudiWorkers();
}

void signal_handler(int signal)
{
    cleanupAndExit();
}

void *worker(void *args)
{

    struct elementoCoda buffer;
    buffer.richiesta.data = NULL;
    while (1)
    {
        if (cc_get(ptrCoda, &buffer) == -1)
        {
            perror("Errore cc_get");
            break;
        }
        if (buffer.richiesta.type == MSG_STOP)
            break;

        int result = sockcom_server_trasmettiRisposta(buffer.client_fd, &(buffer.richiesta));
        if (result == ERR_SYSTEM_CALL)
        {
            perror("Errore chiamata a sockom_server_trasmettiRisposta fallita");
            shutdown(buffer.client_fd, SHUT_RDWR);
            close(buffer.client_fd);
            if ((buffer.richiesta.data))
                free((buffer.richiesta.data));
            break;
        }
        else if (result == ERR_COMUNICAZIONE)
        {
            shutdown(buffer.client_fd, SHUT_RDWR);
            close(buffer.client_fd);
        }

        if ((buffer.richiesta.data))
            free((buffer.richiesta.data));
        (buffer.richiesta.data) = NULL;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    strcat(socketServerPath, argv[1]);

    poll_fds[0].fd = sockcom_apriServer(socketServerPath);
    if (poll_fds[0].fd == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'apertura del socket server");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, signal_handler);

    struct coda_condivisa coda;
    ptrCoda = &coda;

    if (cc_crea(ptrCoda, 20) == ERR_SYSTEM_CALL)
    {
        perror("Errore nella creazione della coda");
        cleanupAndExit();
    }

    workers = (pthread_t *)malloc(sizeof(pthread_t) * 3);
    if (!workers)
    {
        perror("Malloc fallita");
        cleanupAndExit();
    }

    for (int i = 0; i < 3; i++)
    {
        pthread_create(workers + i, NULL, worker, NULL);
        numeroWorkers++;
    }

    if (sockcom_avviaServer(poll_fds, ptrCoda) == ERR_SYSTEM_CALL)
    {
        perror("il server ha avuto un problema");
        cleanupAndExit();
    }

    return 0;
}
