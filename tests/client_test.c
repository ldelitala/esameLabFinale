#include "../include/socket_comunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    char socketServerPath[5][108] = {
        "sockets/server_socket1",
        "sockets/server_socket2",
        "sockets/server_socket3",
        "sockets/server_socket4",
        "sockets/server_socket5"};

    struct messaggio richiesta = {
        .type = 'C',
        .length = strlen("Ciao, server!") + 1,
        .data = "Ciao, server!"};

    for (int i = 0; i < 5; i++)
    {
        int sock_fd = sockcom_client_mandaRichiesta(socketServerPath[i], &richiesta);
        if (sock_fd == ERR_SYSTEM_CALL)
        {
            perror("Errore nella connessione o invio al server");
            exit(EXIT_FAILURE);
        }

        struct messaggio risposta;
        if (sockcom_client_riceviRisposta(sock_fd, &risposta) != SUCCESS)
        {
            perror("Errore nella ricezione della risposta dal server");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        printf("risposta: %s\n", risposta.data);
        free(risposta.data);
        close(sock_fd); // Assicurati di chiudere il socket quando hai finito
    }
    
    return 0;
}