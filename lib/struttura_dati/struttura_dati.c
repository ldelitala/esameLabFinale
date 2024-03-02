#include "../../include/struttura_dati/struttura_dati.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//! FUNZIONI PRIVATE

/**
 * @brief Legge o presta una lista di libri.
 *
 * Itera attraverso un array dinamico di libri, prestando o leggendo ciascun libro basandosi sul parametro `presta`.
 * Aggrega le stringhe dei libri letti o prestati in un'unica stringa di risposta. Ritorna NULL in caso di fallimento dell'allocazione
 * di memoria o se la lettura/prestito fallisce.
 *
 * @param libri_prestati_o_letti Puntatore a un intero per tenere traccia del numero di libri letti o prestati.
 * @param presta Flag che indica se prestare i libri (1) o solo leggerli (0).
 * @return char* Stringa aggregata contenente i libri letti o prestati, o NULL in caso di errore di allocazione di memoria
 * o fallimento nel prestito/lettura dei libri.
 */
char *leggi_o_presta_lista(struct dynamic_array *lista_libri, int *libri_prestati_o_letti, const int presta, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    struct libro **corrente = NULL;
    char *stringa_libro = NULL, *risposta = NULL;
    *libri_prestati_o_letti = 0;

    risposta = (char *)malloc(sizeof(char));
    if (!risposta)
    {
        perror("Errore di allocazione memoria per risposta");
        return NULL;
    }
    *risposta = '\0';

    for (int index = 0; index < lista_libri->da_inserted; index++)
    {
        corrente = (struct libro **)da_at(lista_libri, index);

        if (presta)
        {
            int prestato = lib_prestaThreadSafe(*corrente, mutex, cond);
            if (prestato == ERR_SYSTEM_CALL)
            {
                free(risposta);
                perror("Fallimento nel prestito del libro in modo thread-safe");
                return NULL;
            }
            if (!prestato)
                continue;
        }

        stringa_libro = lib_leggiThreadSafe(*corrente, mutex, cond);
        if (!stringa_libro)
        {
            free(risposta);
            perror("Fallimento nella lettura del libro in modo thread-safe");
            return NULL;
        }

        char *temp_risposta = realloc(risposta, strlen(risposta) + strlen(stringa_libro) + 1);
        if (!temp_risposta)
        {
            free(stringa_libro);
            free(risposta);
            perror("Errore di reallocazione per aggiornare la risposta");
            return NULL;
        }
        risposta = temp_risposta;

        strcat(risposta, stringa_libro);
        free(stringa_libro);
        (*libri_prestati_o_letti)++;
    }

    return risposta;
}

//! FUNZIONI PUBBLICHE

int str_d_genera(struct strutturaDati *struttura_dati, const char *file_record)
{
    struttura_dati->str_d_arrayCampi = da_create(sizeof(struct campoAlbero), 10);
    if (!((struttura_dati->str_d_arrayCampi).da_ptrArray))
    {
        perror("Errore nella creazione dell'array dei campi");
        return ERR_SYSTEM_CALL;
    }

    struttura_dati->str_d_ptrLibri = da_create(sizeof(struct libro *), 10);
    if (!((struttura_dati->str_d_ptrLibri).da_ptrArray))
    {
        da_destroy(&(struttura_dati->str_d_arrayCampi));
        perror("Errore nella creazione dell'array dei puntatori ai libri");
        return ERR_SYSTEM_CALL;
    }

    FILE *biblioteca = fopen(file_record, "r");
    if (!biblioteca)
    {
        perror("Impossibile aprire il file dei record");
        str_d_dealloca(struttura_dati);
        return ERR_SYSTEM_CALL;
    }

    char buffer[MAX_RIGA];
    int errore = 0;

    while (fgets(buffer, MAX_RIGA, biblioteca))
    {
        if (strlen(buffer) < 3)
            continue;

        if (!lib_controllaFormatoCorretto(buffer))
        {
            printf("Formato record non corretto nel file\n");
            str_d_dealloca(struttura_dati);
            return ERR_FORMATO_STR;
        }

        struct libro *temp = (struct libro *)malloc(sizeof(struct libro));
        if (!temp || (errore = lib_crea(temp, buffer)) != SUCCESS)
        {
            free(temp);
            printf("Errore nella creazione del libro da buffer\n");
            str_d_dealloca(struttura_dati);
            return (errore == ERR_FORMATO_DATA) ? ERR_FORMATO_DATA : ERR_SYSTEM_CALL;
        }

        if (arrCampi_aggiungiLibro(&(struttura_dati->str_d_arrayCampi), temp) == ERR_SYSTEM_CALL ||
            da_append(&(struttura_dati->str_d_ptrLibri), &temp) == FAILURE)
        {
            free(temp);
            perror("Errore nell'aggiunta del libro agli array dinamici");
            str_d_dealloca(struttura_dati);
            return ERR_SYSTEM_CALL;
        }
    }

    if (fclose(biblioteca) == EOF)
    {
        perror("Errore nella chiusura del file biblioteca");
        str_d_dealloca(struttura_dati);
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}

int str_d_chiediLibri(struct strutturaDati *struttura_dati, char **dst, char *richiesta, const int presta, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    lib_formattaStringa(richiesta);

    if (!lib_controllaFormatoCorretto(richiesta))
        return ERR_FORMATO_STR;

    struct dynamic_array lista_libri_richiesti = da_create(sizeof(struct libro *), 10);
    if (!(lista_libri_richiesti.da_ptrArray))
    {
        perror("Errore nella creazione dell'array per i libri richiesti");
        return ERR_SYSTEM_CALL;
    }

    if (arrCampi_generaLista(&(struttura_dati->str_d_arrayCampi), &lista_libri_richiesti, richiesta, mutex, cond) == ERR_SYSTEM_CALL)
    {
        da_destroy(&lista_libri_richiesti);
        perror("Errore nella generazione della lista dei libri richiesti");
        return ERR_SYSTEM_CALL;
    }

    if (lista_libri_richiesti.da_inserted == 0)
    {
        da_destroy(&lista_libri_richiesti);
        return 0; // nessun libro trovato
    }
    int libri_prestati_o_letti;
    *dst = leggi_o_presta_lista(&lista_libri_richiesti, &libri_prestati_o_letti, presta, mutex, cond);
    if (!(*dst))
    {
        da_destroy(&lista_libri_richiesti);
        perror("Errore nel prestito o lettura della lista dei libri");
        return ERR_SYSTEM_CALL;
    }

    da_destroy(&lista_libri_richiesti);
    return libri_prestati_o_letti;
}

int str_d_aggiornaFileRecord(struct strutturaDati *struttura_dati, const char *file_record, const char *build_directory)
{
    char temp[MAX_PATH];
    char *format = "%stemp_%d.txt";
    pid_t random = getpid();

    int temp_len = snprintf(temp, MAX_PATH, format, build_directory, random);
    if (temp_len < 0)
    {
        perror("Errore nella formattazione del nome del file temporaneo");
        return ERR_SYSTEM_CALL;
    }
    else if (temp_len > MAX_PATH)
    {
        printf("Overflow del buffer nel nome del file temporaneo\n");
        return ERR_BUFFER_OVERFLOW;
    }

    FILE *bib_temp = fopen(temp, "w");
    if (!bib_temp)
    {
        perror("Impossibile creare il file temporaneo");
        return ERR_SYSTEM_CALL;
    }
    char *stringa_libro;

    for (int index = 0; index < (struttura_dati->str_d_ptrLibri).da_inserted; index++)
    {
        struct libro **corrente = (struct libro **)da_at(&(struttura_dati->str_d_ptrLibri), index);
        stringa_libro = lib_leggi(*corrente);
        if (!stringa_libro)
        {
            fclose(bib_temp);
            perror("Errore nella lettura del libro per aggiornamento file");
            return ERR_SYSTEM_CALL;
        }
        if (fprintf(bib_temp, "%s", stringa_libro) < strlen(stringa_libro))
        {
            free(stringa_libro);
            fclose(bib_temp);
            perror("Errore nella scrittura del libro nel file temporaneo");
            return ERR_SCRITTURA_FILE;
        }

        free(stringa_libro);
    }

    if (fclose(bib_temp) == EOF || remove(file_record) == -1 || rename(temp, file_record) == -1)
    {
        perror("Errore nell'aggiornamento del file record");
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}

void str_d_dealloca(struct strutturaDati *struttura_dati)
{
    if (!struttura_dati)
        return;

    arrCampi_free(&(struttura_dati->str_d_arrayCampi));

    // liberiamo i libri
    struct libro **corrente = NULL;
    for (int index = 0; index < (struttura_dati->str_d_ptrLibri).da_inserted; index++)
    {
        corrente = (struct libro **)da_at(&(struttura_dati->str_d_ptrLibri), index);
        lib_free(*corrente);
        free(*corrente);
    }

    da_destroy(&(struttura_dati->str_d_ptrLibri));
    da_destroy(&(struttura_dati->str_d_arrayCampi));
}
