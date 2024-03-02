#include "../../include/comunicazione/bib_conf.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/**
 * @brief Verifica se una stringa è vuota.
 *
 * Una stringa si considera vuota se contiene solo caratteri di spazio
 * bianco (' ' o '\n'). Questa funzione itera attraverso la stringa
 * per verificare la presenza di qualsiasi carattere diverso da questi spazi.
 *
 * @return int Restituisce 1 se la stringa è vuota, 0 altrimenti.
 */
int isStringEmpty(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ' && str[i] != '\n')
        {
            // Se trovi qualsiasi carattere diverso da ' ' e '\n', la stringa non è vuota
            return 0;
        }
    }

    // Se sei arrivato a questo punto, la stringa è vuota
    return 1;
}

// Funzione per copiare un file
int copy_file(const char *src, const char *dest)
{
    FILE *fsrc, *fdest;
    char buffer[1024];
    size_t bytesRead;

    // Apertura file sorgente in modalità lettura binaria
    fsrc = fopen(src, "rb");
    if (fsrc == NULL)
    {
        perror("Errore nell'apertura del file sorgente");
        return ERR_SYSTEM_CALL;
    }

    // Apertura file destinazione in modalità scrittura binaria
    fdest = fopen(dest, "wb");
    if (fdest == NULL)
    {
        perror("Errore nell'apertura del file destinazione");
        fclose(fsrc); // Chiude il file sorgente se non si può aprire il destinatario
        return ERR_SYSTEM_CALL;
    }

    // Legge dal file sorgente e scrive nel file destinazione
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0)
    {
        fwrite(buffer, 1, bytesRead, fdest);
    }

    // Chiusura dei file
    fclose(fsrc);
    fclose(fdest);

    return SUCCESS; // Successo
}

//! FUNZIONI PER IL SERVER

int bib_addSocket(char *bib_path, char *socket_path, char *bib_name)
{
    sem_t *writers_var, *writers_mutex, *read_attempt_mutex, *resource_mutex;

    if ((rw2_unlinkSemaphores() == ERR_SYSTEM_CALL) ||
        (rw2_openSemaphores(NULL, NULL, &writers_var, &writers_mutex, &read_attempt_mutex, &resource_mutex) == ERR_SYSTEM_CALL))
    {
        perror("Errore nell'inizializzazione dei semafori");
        return ERR_SYSTEM_CALL;
    }

    if (rw2_writerAccess(writers_var, writers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'accesso da scrittore");
        goto close_and_unlink_semaphores;
    }

    // apriamo il file
    FILE *file_conf = fopen(bib_path, "a");
    if (!file_conf)
    {
        perror("Errore nell'apertura del file di configurazione");
        goto safe_exit;
    }

    // Aggiungiamo il socket al file
    int caratteri_da_scrivere = snprintf(NULL, 0, "%s:%s\n", bib_name, socket_path);
    int caratteri_scritti = fprintf(file_conf, "%s:%s\n", bib_name, socket_path); // scriviamo nel file

    // controlliamo il risultato dell'operazione
    if (caratteri_scritti < caratteri_da_scrivere)
    {
        printf("Errore nella scrittura nel file di configurazione. Caratteri previsti: %d, scritti: %d\n", caratteri_da_scrivere, caratteri_scritti);
        fclose(file_conf);
        rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
        rw2_closeSemaphores(NULL, NULL, writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
        rw2_unlinkSemaphores();
        return (caratteri_scritti < 0) ? ERR_SYSTEM_CALL : ERR_SCRITTURA;
    }

    if (fclose(file_conf) == EOF)
    {
        perror("Errore nella chiusura del file di configurazione");
        goto safe_exit;
    }

    if (rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'uscita da scrittore");
        goto close_and_unlink_semaphores;
    }

    return SUCCESS;

safe_exit:
    rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex);

close_and_unlink_semaphores:
    rw2_closeSemaphores(NULL, NULL, writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
    rw2_unlinkSemaphores();
    return ERR_SYSTEM_CALL;
}

int bib_removeSocket(char *bib_path, char *socket_path, char *build_directory_path)
{
    sem_t *writers_var, *writers_mutex, *read_attempt_mutex, *resource_mutex;

    if ((rw2_openSemaphores(NULL, NULL, &writers_var, &writers_mutex, &read_attempt_mutex, &resource_mutex) == ERR_SYSTEM_CALL))
    {
        perror("Errore nell'apertura dei semafori");
        return ERR_SYSTEM_CALL;
    }

    if (rw2_writerAccess(writers_var, writers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'accesso da scrittore ai semafori");
        goto close_and_unlink_semaphores;
    }

    // creiamo path per il file temporaneo
    srand((unsigned int)time(NULL));

    char temp_path[MAX_PATH],
        *format = "%sbib_%hhd.conf";
    int8_t random = (int8_t)rand() % 127;

    int temp_len = snprintf(temp_path, MAX_PATH, format, build_directory_path, random);
    if (temp_len < 0 || temp_len > MAX_PATH)
    {
        perror("Errore nella generazione del path temporaneo");
        rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
        rw2_closeSemaphores(NULL, NULL, writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
        rw2_unlinkSemaphores();
        return (temp_len < 0) ? ERR_SYSTEM_CALL : ERR_BUFFER_OVERFLOW;
    }

    FILE *temp_file = fopen(temp_path, "w"),
         *bib_file = fopen(bib_path, "r");
    if (!temp_file || !bib_file)
    {
        perror("Errore nell'apertura dei file");
        goto safe_exit;
    }

    char buffer[MAX_PATH];
    while (!feof(bib_file) && fgets(buffer, sizeof(buffer), bib_file))
    {
        if ((!strstr(buffer, socket_path)) && (!isStringEmpty(buffer)) && fputs(buffer, temp_file) == EOF)
        {
            perror("Errore nella scrittura nel file temporaneo");
            goto fclose;
        }
    }
    if (ferror(bib_file) || !feof(bib_file))
    {
        perror("Errore nella lettura del file bib.conf");
        goto fclose;
    }

    int size, bibIsEmpty = 0;
    if ((fseek(temp_file, 0, SEEK_END) == -1) || (size = ftell(temp_file), size == -1))
    {
        perror("Errore nel controllo della dimensione del file temporaneo");
        goto fclose;
    }
    bibIsEmpty = !size;

    if ((fclose(bib_file) == EOF) || (fclose(temp_file) == EOF))
    {
        perror("Errore nella chiusura dei file");
        goto safe_exit;
    }

    char backup_path[MAX_PATH];
    snprintf(backup_path, MAX_PATH, "%sbib.bak", build_directory_path);

    if (copy_file(bib_path, backup_path) == ERR_SYSTEM_CALL)
    {
        perror("Errore nel creare il backup del file bib.conf");
        goto safe_exit;
    }

    if (remove(bib_path) == -1 || rename(temp_path, bib_path) == -1)
    {
        perror("Errore nel rinominare o rimuovere il file");
        rename(backup_path, bib_path); // Restore from backup
        goto safe_exit;
    }

    remove(backup_path);

    if ((rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL) ||
        (rw2_closeSemaphores(NULL, NULL, writers_var, writers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL) ||
        (bibIsEmpty && (rw2_unlinkSemaphores() == ERR_SYSTEM_CALL)))
    {
        perror("Errore nella chiusura e unlink dei semafori");
        goto close_and_unlink_semaphores;
    }

    return SUCCESS;

fclose:
    if (temp_file)
        fclose(temp_file);
    if (bib_file)
        fclose(bib_file);
    remove(temp_path);

safe_exit:
    rw2_writerExit(writers_var, writers_mutex, read_attempt_mutex, resource_mutex);

close_and_unlink_semaphores:
    rw2_closeSemaphores(NULL, NULL, writers_var, writers_mutex, read_attempt_mutex, resource_mutex);
    rw2_unlinkSemaphores();
    return ERR_SYSTEM_CALL;
}

//! FUNZIONI PER IL CLIENT

char *bib_leggi(char *bib_path)
{
    sem_t *readers_var, *readers_mutex, *read_attempt_mutex, *resource_mutex;
    FILE *bib_file = NULL;
    char *bib_data = NULL;

    if ((rw2_openSemaphores(&readers_var, &readers_mutex, NULL, NULL, &read_attempt_mutex, &resource_mutex) == ERR_SYSTEM_CALL))
    {
        perror("Errore nell'apertura dei semafori per la lettura");
        return NULL;
    }

    if (rw2_readerAccess(readers_var, readers_mutex, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'ottenere l'accesso di lettura");
        goto close_and_unlink_semaphores;
    }

    bib_file = fopen(bib_path, "r");
    if (!bib_file)
    {
        perror("Errore nell'apertura del file di configurazione");
        goto safe_exit;
    }

    bib_data = (char *)malloc(sizeof(char));
    if (!bib_data)
    {
        fprintf(stderr, "Errore nell'allocazione iniziale della memoria per i dati del file\n");
        goto safe_exit;
    }
    bib_data[0] = '\0';

    char buffer[MAX_RIGA];

    while (fgets(buffer, MAX_RIGA, bib_file))
    {
        bib_data = realloc(bib_data, strlen(bib_data) + strlen(buffer) + 1);
        if (!bib_data)
        {
            fprintf(stderr, "Errore nella riallocazione della memoria per i dati del file\n");
            free(bib_data); // Liberiamo la memoria precedentemente allocata
            goto safe_exit;
        }
        strcat(bib_data, buffer);
    }
    if (ferror(bib_file))
    {
        perror("Errore nella lettura del file");
        goto safe_exit;
    }

    if (fclose(bib_file) == EOF)
    {
        perror("Errore nella chiusura del file");
        free(bib_data);
        goto close_and_unlink_semaphores;
    }

    if ((rw2_readerExit(readers_var, readers_mutex, resource_mutex) == ERR_SYSTEM_CALL) ||
        (rw2_closeSemaphores(readers_var, readers_mutex, NULL, NULL, read_attempt_mutex, resource_mutex) == ERR_SYSTEM_CALL) ||
        (rw2_unlinkSemaphores() == ERR_SYSTEM_CALL))
    {
        perror("Errore nel rilascio delle risorse o nella chiusura dei semafori");
        free(bib_data);
        return NULL;
    }

    return bib_data; // success

safe_exit:
    if (bib_data)
        free(bib_data);
    if (bib_file)
        fclose(bib_file);
    rw2_readerExit(readers_var, readers_mutex, resource_mutex);

close_and_unlink_semaphores:
    rw2_closeSemaphores(readers_var, readers_mutex, NULL, NULL, read_attempt_mutex, resource_mutex);
    rw2_unlinkSemaphores();
    return NULL;
}
