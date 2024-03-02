#include "../../include/comunicazione/socket_comunication.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

int sockcom_apriServer(char socket_path_univoco[108])
{
    struct sockaddr_un server_addr = {.sun_family = AF_UNIX};
    strcpy(server_addr.sun_path, socket_path_univoco);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Errore creazione socket");
        return ERR_SYSTEM_CALL;
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Errore nella bind");
        close(server_fd);
        return ERR_SYSTEM_CALL;
    }

    char abs_socket_path[4096];
    // troviamo il percorso assoluto per esguire l'unlink in caso di errori
    if (realpath(socket_path_univoco, abs_socket_path) == NULL)
    {
        perror("Errore nella ricerca del percorso assoluto");
        return ERR_SYSTEM_CALL;
    }

    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("Errore listen fallita");
        close(server_fd);
        unlink(abs_socket_path);
        return ERR_SYSTEM_CALL;
    }

    return server_fd;
}

//-poll_fds[0].fd deve essere il file descriptor del server
int sockcom_avviaServer(struct pollfd poll_fds[MAX_CLIENTS + 1], struct coda_condivisa *coda)
{
    struct elementoCoda daInviare;

    poll_fds[0].events = POLLIN;
    for (int i = 1; i < MAX_CLIENTS + 1; i++)
        poll_fds[i].fd = -1;

    while (1)
    {
        int num_events = poll(poll_fds, MAX_CLIENTS + 1, -1);
        if (num_events == -1 && errno != EINTR)
        {
            perror("Errore poll");
            break;
        }

        if (poll_fds[0].revents & POLLIN)
        {
            int fd_libero;
            for (fd_libero = 1; fd_libero < MAX_CLIENTS + 1 && poll_fds[fd_libero].fd != -1; fd_libero++)
            {
                // trovo fd libero
            }

            if (fd_libero < MAX_CLIENTS + 1)
            {
                int client_fd = accept(poll_fds[0].fd, NULL, NULL);
                if (client_fd == -1)
                {
                    perror("Errore con accept");
                    continue;
                }
                poll_fds[fd_libero].fd = client_fd;
                poll_fds[fd_libero].events = POLLIN;
            }
        }

        for (int j = 1; j < MAX_CLIENTS + 1; j++)
        {
            if (poll_fds[j].fd == -1)
                continue;

            if (poll_fds[j].revents & (POLLERR | POLLHUP | POLLNVAL))
            {

                if (poll_fds[j].revents & POLLERR)
                    printf("La poll ha riportato un problema\n");

                if (poll_fds[j].revents & POLLHUP)
                    printf("Il cliente ha chiuso\n");

                if (poll_fds[j].revents & POLLNVAL)
                    printf("File descriptor invalido\n");

                close(poll_fds[j].fd);
                poll_fds[j].fd = -1;
                continue;
            }

            if (poll_fds[j].revents & POLLIN)
            {
                daInviare.richiesta.data = NULL;

                ssize_t bytes_read = read(poll_fds[j].fd, &(daInviare.richiesta.type), sizeof(char));
                if (bytes_read == -1)
                {
                    perror("Read di type fallita");
                    goto error_exit;
                }
                else if (bytes_read == 0)
                {
                    printf("Client ha chiuso la comunicazione in anticipo\n");
                    goto client_ha_chiuso;
                }

                bytes_read = read(poll_fds[j].fd, &(daInviare.richiesta.length), sizeof(int32_t));
                if (bytes_read == -1)
                {
                    perror("Read di length fallita");
                    goto error_exit;
                }
                else if (bytes_read == 0)
                {
                    printf("Client ha chiuso la comunicazione in anticipo\n");
                    goto client_ha_chiuso;
                }

                size_t lenData = sizeof(char) * daInviare.richiesta.length;
                daInviare.richiesta.data = (char *)malloc(lenData);
                if (!(daInviare.richiesta.data))
                {
                    perror("Malloc fallita");
                    goto error_exit;
                }

                bytes_read = 0;
                while (bytes_read < lenData)
                {
                    ssize_t result = read(poll_fds[j].fd, daInviare.richiesta.data + bytes_read, lenData - bytes_read);
                    if (result == -1)
                    {
                        perror("Read di data fallita");
                        free(daInviare.richiesta.data);
                        goto error_exit;
                    }
                    else if (result == 0)
                    {
                        printf("Client ha chiuso la comunicazione in anticipo\n");
                        free(daInviare.richiesta.data);
                        goto client_ha_chiuso;
                    }

                    bytes_read += result;
                }

                daInviare.client_fd = poll_fds[j].fd;

                if (cc_put(coda, &daInviare) == ERR_SYSTEM_CALL)
                {
                    perror("cc_put fallita");
                    free(daInviare.richiesta.data);
                    goto error_exit;
                }

                poll_fds[j].fd = -1;
                continue;

            client_ha_chiuso:
                if (poll_fds[j].fd != -1)
                {
                    close(poll_fds[j].fd);
                    poll_fds[j].fd = -1;
                }
                continue;
            }
        }
    }

error_exit:
    for (int i = 1; i < MAX_CLIENTS + 1; i++)
    {
        if (poll_fds[i].fd != -1)
        {
            shutdown(poll_fds[i].fd, SHUT_RDWR);
            close(poll_fds[i].fd);
            poll_fds[i].fd = -1;
        }
    }
    return ERR_SYSTEM_CALL;
}

// se la funzione riporta un qualsiasi errore, il chiamante deve chiudere il file descriptor del client
// e liberare la memoria di risposta.data
int sockcom_server_trasmettiRisposta(int client_fd, struct messaggio *risposta)
{
    struct pollfd client = {.fd = client_fd, .events = POLLOUT};

    int num_events = poll(&client, 1, -1);
    if (num_events == -1 && errno != EINTR)
    {
        perror("Errore poll");
        return ERR_SYSTEM_CALL;
    }

    if (client.revents & (POLLERR | POLLHUP | POLLNVAL))
    {
        if (client.revents & POLLERR)
            printf("La poll ha riportato un problema\n");

        if (client.revents & POLLHUP)
            printf("Il cliente ha chiuso\n");

        if (client.revents & POLLNVAL)
            printf("File descriptor invalido\n");

        return ERR_COMUNICAZIONE;
    }

    if (client.revents & POLLOUT)
    {
        ssize_t bytes_written = write(client.fd, &(risposta->type), sizeof(char));
        if (bytes_written == -1)
        {
            perror("write di type fallita");
            return ERR_SYSTEM_CALL;
        }

        bytes_written = write(client.fd, &(risposta->length), sizeof(int32_t));
        if (bytes_written == -1)
        {
            perror("write di length fallita");
            return ERR_SYSTEM_CALL;
        }

        size_t lenData = sizeof(char) * risposta->length;
        bytes_written = 0;
        while (bytes_written < lenData)
        {
            ssize_t result = write(client.fd, risposta->data + bytes_written, lenData - bytes_written);
            if (result == -1)
            {
                perror("write di data fallita");
                return ERR_SYSTEM_CALL;
            }
            bytes_written += result;
        }

        char buffer = 'r';
        ssize_t bytes_read = read(client.fd, &buffer, sizeof(char));
        if (bytes_read == -1)
        {
            printf("Il client ha chiuso la comunicazione\n");
            return ERR_SYSTEM_CALL;
        }
        else if (bytes_read != 0)
        {
            printf("Alcuni dati inviati dal client sono stati persi,"
                   "chiudiamo la connessione e cancelliamo la richiesta per sicurezza\n");
            return ERR_SYSTEM_CALL;
        }

        if (shutdown(client.fd, SHUT_RDWR) == -1)
        {
            perror("shutdown fallita");
            return ERR_SYSTEM_CALL;
        }

        return SUCCESS;
    }

    printf("La poll è uscita con revents!= POLLOUT | POLLERR | POLLHUP | POLLNVAL\n");
    return ERR_COMUNICAZIONE;
}

int sockcom_client_mandaRichiesta(char socketServerPath[108], struct messaggio *richiesta)
{
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("creazione socket fallita");
        return ERR_SYSTEM_CALL;
    }

    struct sockaddr_un server_addr = {.sun_family = AF_UNIX};
    strcpy(server_addr.sun_path, socketServerPath);

    int result = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
    if (result == -1)
    {
        perror("connessione rifiutata");
        goto error_exit;
    }

    ssize_t bytes_written = write(sock_fd, &(richiesta->type), sizeof(char));
    if (bytes_written == -1)
    {
        perror("Write di type fallita");
        goto error_exit;
    }

    bytes_written = write(sock_fd, &(richiesta->length), sizeof(int32_t));
    if (bytes_written == -1)
    {
        perror("Write di type fallita");
        goto error_exit;
    }

    size_t lenData = sizeof(char) * richiesta->length;
    bytes_written = 0;
    while (bytes_written < lenData)
    {
        ssize_t result = write(sock_fd, richiesta->data + bytes_written, lenData - bytes_written);
        if (result == -1)
        {
            perror("write di data fallita");
            goto error_exit;
        }
        bytes_written += result;
    }

    if (shutdown(sock_fd, SHUT_WR) == -1)
        perror("chiusura in scrittura fallita");

    return sock_fd;

error_exit:
    if (sock_fd != -1)
    {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }
    return ERR_SYSTEM_CALL;
}

// chiamante deve deallocare risposta e chiudere sock_fd
int sockcom_client_riceviRisposta(int sock_fd, struct messaggio *risposta)
{
    ssize_t bytes_read = read(sock_fd, &(risposta->type), sizeof(char));
    if (bytes_read == -1)
    {
        perror("Read di type fallita");
        return ERR_SYSTEM_CALL;
    }
    else if (bytes_read == 0)
    {
        printf("Server ha chiuso la comunicazione in anticipo\n");
        return ERR_COMUNICAZIONE;
    }

    bytes_read = read(sock_fd, &(risposta->length), sizeof(int32_t));
    if (bytes_read == -1)
    {
        perror("Read di length fallita");
        return ERR_SYSTEM_CALL;
    }
    else if (bytes_read == 0)
    {
        printf("Server ha chiuso la comunicazione in anticipo\n");
        return ERR_COMUNICAZIONE;
    }

    if (risposta->length == 0)
    {
        risposta->data = NULL;
        goto success_exit;
    }

    size_t lenData = sizeof(char) * risposta->length;
    risposta->data = (char *)malloc(lenData);
    if (!risposta->data)
    {
        perror("malloc fallita");
        return ERR_SYSTEM_CALL;
    }

    bytes_read = 0;
    while (bytes_read < lenData)
    {
        ssize_t result = read(sock_fd, risposta->data + bytes_read, lenData - bytes_read);
        if (result == -1)
        {
            perror("Read di data fallita");
            free(risposta->data);
            return ERR_SYSTEM_CALL;
        }
        else if (result == 0)
        {
            printf("Server ha chiuso la comunicazione in anticipo\n");
            free(risposta->data);
            return ERR_SYSTEM_CALL;
        };

        bytes_read += result;
    }

success_exit:
    if (shutdown(sock_fd, SHUT_RD) == -1)
        perror("Shutdown fallita");

    return SUCCESS;
}