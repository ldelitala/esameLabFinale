#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/comunicazione/socket_comunication.h"
#include "../../include/comunicazione/bib_conf.h"
#include "../../include/comunicazione/protocollo_comunicazione.h"

/**
 * Estrae il nome della biblioteca e il percorso del server da una stringa.
 *
 * @return 1 se l'estrazione ha successo, 0 altrimenti.
 */
int estrai_nome_e_path(char *stringa, char **nomeBib, char **serverPath);

/**
 * Elabora gli argomenti della linea di comando per costruire la richiesta.
 *
 * @return Puntatore alla stringa della richiesta, NULL in caso di fallimento.
 */
char *lettura_argomenti(int argc, char **argv);

/**
 * Aggiunge una coppia campo-valore alla stringa della richiesta.
 *
 * @param stringa Stringa della richiesta.
 * @return Puntatore alla nuova stringa della richiesta, NULL in caso di fallimento.
 */
char *aggiungi_coppia(char *stringa, char *campo, char *valore);

int main(int argc, char *argv[])
{
    char *richiesta,
        *bibFile,
        *temp_nomeBib, *temp_serverPath;
    struct messaggio daInviare;

    //* CONTROLLO ARGOMENTI

    // controlliamo se è una richiesta di prestito
    if (strcmp(argv[argc - 1], "-p") == 0)
    {
        daInviare.type = MSG_LOAN;
        argc--;
    }
    else
        daInviare.type = MSG_QUERY;

    // se era l'unico argomento allora manca la coppia campo:valore
    if (argc <= 1)
    {
        printf("La chiamata a bibclient deve contenere almeno una coppia campo-valore nel formato:\n"
               "./bibclient --campo=\"valore\" [-p]\n dove -p è un campo opzionale che, se presente, "
               "richiede il prestito di tutti i libri trovati.\n");
        exit(EXIT_FAILURE);
    }

    richiesta = lettura_argomenti(argc, argv);
    if (!richiesta)
    {
        printf("Creazione della richiesta fallita\n");
        exit(EXIT_FAILURE);
    }

    daInviare.data = richiesta;
    daInviare.length = strlen(richiesta) + 1;

    bibFile = bib_leggi(BIB_CONF_PATH);
    if (!bibFile)
    {
        printf("chiamata a bib_leggi fallita\n");
        exit(EXIT_FAILURE);
    }

    if (!estrai_nome_e_path(bibFile, &temp_nomeBib, &temp_serverPath))
    {
        printf("Nessun server trovato\n");
        exit(EXIT_SUCCESS);
    }

    do
    {
        printf("\nMANDO LA RICHIESTA ALLA BIBBLIOTECCA: %s\n", temp_nomeBib);

        errno = 0;
        int sock_fd = sockcom_client_mandaRichiesta(temp_serverPath, &daInviare);
        if (errno == ECONNREFUSED)
        {
            printf("È stato impossibile connettersi alla biblioteca \"%s\"\n", temp_nomeBib);
            continue;
        }
        else if (sock_fd == ERR_SYSTEM_CALL)
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

        switch (risposta.type)
        {
        case MSG_NO:
            printf("\nNon è stato trovato alcun libro corrispondente alla richiesta\n");
            break;
        case MSG_ERROR:
            printf("\nIl server ha riportato il seguente errore: ");
        default:
            printf("\n%s", risposta.data);
            break;
        }

        if (risposta.data)
            free(risposta.data);
        if (sock_fd != -1)
            close(sock_fd);

    } while (estrai_nome_e_path(NULL, &temp_nomeBib, &temp_serverPath));

    exit(EXIT_SUCCESS);
}

char *lettura_argomenti(int argc, char **argv)
{

    char *equal_sign, *arg, *campo, *valore,
        *richiesta = (char *)malloc(sizeof(char));
    if (!richiesta)
    {
        perror("Chiamata a malloc fallita per la variabile \"richiesta\"");
        return NULL;
    }
    *richiesta = '\0';

    for (int i = 1; i < argc; i++)
    {
        arg = argv[i];
        if (
            (arg[0] == '-' && arg[1] == '-') &&                         // controllo che sia un opzione lunga
            ((equal_sign = strchr(arg, '=')) && equal_sign != arg + 2)) // e trovo l'uguale
        {
            campo = arg + 2;
            *equal_sign = '\0';
            valore = equal_sign + 1;

            richiesta = aggiungi_coppia(richiesta, campo, valore);
            if (!richiesta)
            {
                printf("Errore: chiamata ad aggiungi coppia fallita\n");
                return NULL;
            }
        }
        else
        {
            printf("Tutte le coppie campo:valore devono essere del formato --campo=\"valore\"\n");
            return NULL;
        }
    }
    return richiesta;
}

char *aggiungi_coppia(char *stringa, char *campo, char *valore)
{
    //"%s %s: %s;" il 5 è per garantire spazio ai caratteri in più
    size_t new_size = strlen(stringa) + strlen(campo) + strlen(valore) + 5;

    stringa = realloc(stringa, new_size);
    if (!stringa)
    {
        perror("Errore: chiamata a realloc fallita per la variabile \"stringa\"");
        return NULL;
    }

    strcat(stringa, " ");
    strcat(stringa, campo);
    strcat(stringa, ": ");
    strcat(stringa, valore);
    strcat(stringa, ";");

    return stringa;
}

int estrai_nome_e_path(char *stringa, char **nomeBib, char **serverPath)
{
    if (((*nomeBib) = strtok(stringa, ":")) &&
        ((*serverPath) = strtok(NULL, "\n")))
        return 1;
    return 0;
}
