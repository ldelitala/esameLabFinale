#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/socket.h>
#include <pthread.h>

#include "../../include/comunicazione/protocollo_comunicazione.h"
#include "../../include/comunicazione/socket_comunication.h"
#include "../../include/comunicazione/bib_conf.h"

#include "../../include/struttura_dati/struttura_dati.h"

#define MSG_STOP 'S'       ///< Codice di messaggio per fermare i worker.
#define MAX_PATH 108       ///< Lunghezza massima del percorso dei file.
#define DIMENSIONE_CODA 20 ///< Dimensione della coda di richieste.
#define SUCCESS 0
#define FAILURE -1

char socketServerPath[MAX_PATH],
    fileRecordPath[MAX_PATH];
struct pollfd poll_fds[MAX_CLIENTS + 1];
struct strutturaDati *ptrStr_d = NULL;
struct coda_condivisa *ptrCoda = NULL;
pthread_t *workers = NULL;
int numeroWorkers = 0;
FILE *log_file = NULL;
pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_libri = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_libri = PTHREAD_COND_INITIALIZER;

void signal_handler(int signal);

/**
 * @return 1 se la stringa rappresenta un intero positivo, 0 altrimenti.
 */
int isStrPositiveInteger(const char *str);

/**
 * Legge gli argomenti della linea di comando e li elabora.
 *
 * @return SUCCESS in caso di successo, FAILURE altrimenti.
 */
int leggiArgomenti(int argc, char **argv, char *name_bib, char *file_record_path, int *numero_worker_richiesti);

/**
 * @param name_bib Nome della biblioteca, usato per generare il nome del file di log.
 * @return SUCCESS in caso di successo, FAILURE altrimenti.
 */
int apri_file_log(char name_bib[MAX_PATH]);

/**
 * @param type Tipo di operazione (prestito o query).
 * @param numero_libri Numero di libri coinvolti nell'operazione.
 * @param risposta_data Dati della risposta.
 * @return SUCCESS in caso di successo, FAILURE altrimenti.
 */
int log_add_op(FILE **log_file, pthread_mutex_t *mutex, char type, int numero_libri, char *risposta_data);

/**
 * Chiude i worker threads e pulisce le risorse.
 */
void chiudiWorkers();

/**
 * Pulisce le risorse e termina l'esecuzione.
 *
 * @param exit_status Codice di uscita del programma.
 */
void cleanupAndExit(int exit_status);

/**
 * Funzione eseguita dai worker threads.
 *
 * @param args Argomenti del thread (non utilizzati).
 * @return NULL.
 */
void *worker(void *args);

int main(int argc, char *argv[])
{
    char nomeBib[MAX_PATH];
    struct strutturaDati struttura_dati;
    struct coda_condivisa coda;
    int error;
    pid_t pid;

    //*LEGGO GLI ARGOMENTI
    if (leggiArgomenti(argc, argv, nomeBib, fileRecordPath, &numeroWorkers) == FAILURE)
        exit(EXIT_FAILURE);

    //*INIZIALIZZO GLI FD DI POLL_FDS A -1
    for (int i = 0; i < MAX_CLIENTS + 1; i++)
    {
        poll_fds[i].fd = -1;
    }

    //*APRO IL FILE DI LOG
    if (apri_file_log(nomeBib) == FAILURE)
        exit(EXIT_FAILURE);

    //*GENERO LA STRUTTURA DATI
    error = str_d_genera(&struttura_dati, fileRecordPath);
    switch (error)
    {
    case ERR_SYSTEM_CALL:
        printf("Chiamata a str_d_genera fallita\n");
        break;

    case ERR_FORMATO_STR:
        printf("Una stringa di un libro non è del formato corretto\n");
        break;

    case ERR_FORMATO_DATA:
        printf("Un stringa di un libro contiene una data non valida\n");
        break;
    }

    if (error != SUCCESS)
        exit(EXIT_FAILURE);

    ptrStr_d = &struttura_dati;

    //*CREO LA CODA CONDIVISA ED I WORKERS
    if (cc_crea(&coda, DIMENSIONE_CODA) == ERR_SYSTEM_CALL)
    {
        perror("Errore nella creazione della coda");
        cleanupAndExit(EXIT_FAILURE);
    }
    ptrCoda = &coda;

    workers = (pthread_t *)malloc(sizeof(pthread_t) * numeroWorkers);
    if (!workers)
    {
        perror("Malloc fallita");
        cleanupAndExit(EXIT_FAILURE);
    }

    for (int i = 0; i < numeroWorkers; i++)
    {
        if ((error = pthread_create(workers + i, NULL, worker, NULL)))
        {
            printf("pthread_create fallita: %s", strerror(error));
            cleanupAndExit(EXIT_FAILURE);
        }
    }

    //*APRO IL SERVER
    pid = getpid();
    if (snprintf(socketServerPath, MAX_PATH, "%ssocketServer_%d", SOCKET_DIR, pid) < 0)
    {
        perror("snprintf per socketServerPath fallita");
        cleanupAndExit(EXIT_FAILURE);
    }

    poll_fds[0].fd = sockcom_apriServer(socketServerPath);
    if (poll_fds[0].fd == ERR_SYSTEM_CALL)
    {
        printf("Errore nell'apertura del socket server");
        cleanupAndExit(EXIT_FAILURE);
    }

    //*AGGIUNGO IL SERVER AL FILE DI BIB.CONF
    error = bib_addSocket(BIB_CONF_PATH, socketServerPath, nomeBib);
    if (error == ERR_SYSTEM_CALL)
    {
        printf("Chiamata a bib_addSocket fallita\n");
        cleanupAndExit(EXIT_FAILURE);
    }

    //*IMPOSTO LA SIGNAL
    if (signal(SIGINT, signal_handler) == SIG_ERR || signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        perror("tentativo di impostare la signal fallito");
        cleanupAndExit(EXIT_FAILURE);
    }

    //*AVVIO IL SERVER
    if (sockcom_avviaServer(poll_fds, ptrCoda) == ERR_SYSTEM_CALL)
    {
        perror("il server ha avuto un problema");
        cleanupAndExit(EXIT_FAILURE);
    }

    return 0;
}

void *worker(void *args)
{
    struct elementoCoda buffCoda;
    buffCoda.richiesta.data = NULL;
    struct messaggio risposta = {.data = NULL};
    int presta, result, libri_letti;
    char *buffStr = NULL;

    while (1)
    {
        if (cc_get(ptrCoda, &buffCoda) == -1)
        {
            perror("Errore cc_get");
            break;
        }
        if (buffCoda.richiesta.type == MSG_STOP)
            break;

        presta = (buffCoda.richiesta.type == MSG_LOAN) ? 1 : 0;
        libri_letti = str_d_chiediLibri(ptrStr_d, &buffStr, buffCoda.richiesta.data, presta, &mutex_libri, &cond_libri);

        switch (libri_letti)
        {
        case ERR_SYSTEM_CALL:
            risposta = (struct messaggio){
                .type = MSG_ERROR,
                .length = strlen(STR_ERR_SYSCALL) + 1,
                .data = STR_ERR_SYSCALL};
            break;

        case ERR_FORMATO_STR:
            risposta = (struct messaggio){
                .type = MSG_ERROR,
                .length = strlen(STR_ERR_FRMT_RIC) + 1,
                .data = STR_ERR_FRMT_RIC};
            break;

        case 0:
            risposta = (struct messaggio){
                .type = MSG_NO,
                .length = 0,
                .data = NULL};

            log_add_op(&log_file, &mutex_log, buffCoda.richiesta.type, libri_letti, risposta.data);
            break;

        default:
            risposta = (struct messaggio){
                .type = MSG_RECORD,
                .length = strlen(buffStr) + 1,
                .data = buffStr};
            log_add_op(&log_file, &mutex_log, buffCoda.richiesta.type, libri_letti, risposta.data);
            break;
        }

        result = sockcom_server_trasmettiRisposta(buffCoda.client_fd, &risposta);
        if (result == ERR_SYSTEM_CALL)
        {
            printf("Errore chiamata a sockom_server_trasmettiRisposta fallita\n");
            if (shutdown(buffCoda.client_fd, SHUT_RDWR) == -1)
                perror("Errore shutdown");
        }
        else if (result == ERR_COMUNICAZIONE)
        {
            printf("C'è stato un errore nella comunicazione con il client\n");
            if (shutdown(buffCoda.client_fd, SHUT_RDWR) == -1)
                perror("Errore shutdown");
        }

        if (buffCoda.client_fd != -1)
        {
            if (close(buffCoda.client_fd) == -1)
                perror("Errore close");
        }

        if ((buffCoda.richiesta.data))
        {
            free((buffCoda.richiesta.data));
            (buffCoda.richiesta.data) = NULL;
        }
        if (buffStr)
        {
            free(buffStr);
            buffStr = NULL;
        }
    }

    if (buffStr)
        free(buffStr);
    if ((buffCoda.richiesta.data))
        free((buffCoda.richiesta.data));
    return NULL;
}

void chiudiWorkers()
{
    struct elementoCoda msgStop = {
        .client_fd = -1,
        .richiesta = (struct messaggio){.type = MSG_STOP, .length = 0, .data = NULL}};

    for (int i = 0; i < numeroWorkers; i++)
    {
        if (cc_put(ptrCoda, &msgStop) == ERR_SYSTEM_CALL)
        {
            perror("Errore put msg stop");
        }
    }

    for (int i = 0; i < numeroWorkers; i++)
    {
        if (pthread_join(workers[i], NULL) == -1)
            printf("Errore durante la chiamata a pthread_join\nImpossibile chiudere i thread worker correttamente,\nsperiamo che il sistema operativo se la gestisca\n");
    }

    if (workers)
        free(workers);
}

void cleanupAndExit(int exit_status)
{
    int error;
    if ((error = pthread_mutex_destroy(&mutex_libri)) != 0)
        printf("Errore distruggendo mutex_libri: %s\n", strerror(error));

    if ((error = pthread_cond_destroy(&cond_libri)) != 0)
        printf("Errore distruggendo cond_libri: %s\n", strerror(error));

    if (ptrStr_d)
    {

        if (str_d_aggiornaFileRecord(ptrStr_d, fileRecordPath, BUILD_DIR) != SUCCESS)
            printf("Impossibile aggiornare il file record.\n");

        str_d_dealloca(ptrStr_d);
    }
    if (bib_removeSocket(BIB_CONF_PATH, socketServerPath, BUILD_DIR) == ERR_SYSTEM_CALL)
        printf("Impossibile rimuovere la socket dal file di configurazione\n");

    if (poll_fds[0].fd != -1)
    {
        if (shutdown(poll_fds[0].fd, SHUT_RDWR) == -1)
            perror("Errore chiudendo il socket del server (shutdown)");

        if (close(poll_fds[0].fd) == -1)
            perror("Errore chiudendo il socket del server (close)");

        if (unlink(socketServerPath) == -1)
            perror("Errore rimuovendo il socket del server");
    }

    for (int i = 1; i < MAX_CLIENTS + 1; i++)
    {
        if (poll_fds[i].fd != -1)
        {
            if (shutdown(poll_fds[i].fd, SHUT_RDWR) == -1)
                perror("Errore chiudendo socket client (shutdown)");

            if (close(poll_fds[i].fd) == -1)
                perror("Errore chiudendo socket client (close)");
        }
    }

    chiudiWorkers();

    if (log_file && fclose(log_file) == EOF)
        perror("Errore chiudendo log_file");

    if (ptrCoda)
        cc_destroy(ptrCoda);

    exit(exit_status);
}

int apri_file_log(char name_bib[MAX_PATH])
{
    char file_log_path[MAX_PATH];

    if (strlen(LOGS_DIR) + strlen(name_bib) + 4 >= MAX_PATH)
    {
        printf("Errore: rischio di buffer overflow in file_log_path\n");
        return FAILURE;
    }

    strcpy(file_log_path, LOGS_DIR);
    strcat(file_log_path, name_bib);
    strcat(file_log_path, ".log"); // aggiungo il .log

    log_file = fopen(file_log_path, "w");
    if (!log_file)
    {
        perror("Errore: apertura file log fallita\n");
        return FAILURE;
    }

    return SUCCESS;
}

int log_add_op(FILE **log_file, pthread_mutex_t *mutex, char type, int numero_libri, char *risposta_data)
{
    if (pthread_mutex_lock(mutex))
    {
        perror("Errore: impossibile eseguire operazione su mutex");
        return FAILURE;
    }

    int caratteri_da_scrivere = snprintf(NULL, 0, "%s %d\n\n",
                                         (type == MSG_LOAN) ? "LOAN" : "QUERY", numero_libri),

        caratteri_scritti = fprintf(*log_file, "%s %d\n\n",
                                    (type == MSG_LOAN) ? "LOAN" : "QUERY", numero_libri); // scriviamo nel file

    if (caratteri_scritti < caratteri_da_scrivere)
    {
        printf("Errore nella scrittura del file log. Caratteri previsti: %d, scritti: %d\n"
               "Errore di sistema: %s\n",
               caratteri_da_scrivere, caratteri_scritti, strerror(errno));
        return FAILURE;
    }

    if (numero_libri != 0)
    {
        caratteri_da_scrivere = strlen(risposta_data) + 2,

        caratteri_scritti = fprintf(*log_file, "%s\n\n", risposta_data); // scriviamo nel file

        if (caratteri_scritti < caratteri_da_scrivere)
        {
            printf("Errore nella scrittura del file log. Caratteri previsti: %d, scritti: %d\n"
                   "Errore di sistema: %s\n",
                   caratteri_da_scrivere, caratteri_scritti, strerror(errno));
            return FAILURE;
        }
    }

    if (pthread_mutex_unlock(mutex))
    {
        perror("Errore: impossibile eseguire operazione su mutex");
        return FAILURE;
    }
    return SUCCESS;
}

int leggiArgomenti(int argc, char **argv, char *name_bib, char *file_record_path, int *numero_worker_richiesti)
{
    if (argc < 4)
    {
        printf("Errore: parametri mancanti\n Il comando deve essere del tipo:\n $ bibserver name_bib file_record W");
        return FAILURE;
    }

    if (strlen(argv[1]) >= MAX_PATH || strlen(argv[2]) >= MAX_PATH - strlen(FILE_RECORDS_DIR) - 4)
    {
        printf("Errore: nome o path troppo lungo\n Il comando deve essere del tipo:\n $ bibserver name_bib file_record W \n");
        return FAILURE;
    }

    // li copio
    strcpy(name_bib, argv[1]);
    snprintf(file_record_path, MAX_PATH, "%s%s.txt", FILE_RECORDS_DIR, argv[2]);

    // controllo che il terzo argomento sia un intero positivo
    if (!isStrPositiveInteger(argv[3]))
    {
        printf("Errore: W deve essere un numero intero positivo\n Il comando deve essere del tipo:\n $ bibserver name_bib file_record W \n");
        return FAILURE;
    }

    *numero_worker_richiesti = atoi(argv[3]);

    return SUCCESS;
}

int isStrPositiveInteger(const char *str)
{
    if (str == NULL || *str == '\0' || *str == '-')
        return 0;

    if (*str == '+')
        str++;

    while (*str != '\0')
    {
        // Se trova un carattere non numerico, ritorna 0
        if (*str < '0' || *str > '9')
            return 0;
        str++;
    }

    // Tutti i caratteri sono cifre valide
    return 1;
}

void signal_handler(int signal)
{
    cleanupAndExit(EXIT_SUCCESS);
}
