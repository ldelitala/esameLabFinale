
#define SUCCESS 0

#define FAILURE -1

#define ERR_SYSTEM_CALL_MSG "Errore: una chiamata di sistema è fallita durante l'esecuzione di str_d_genera"

#define FILE_RECORD "data/file_records/bib1.txt"

#define BUILD_DIR "build/"

#include "../include/struttura_dati.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

int test_chiediLibri(struct strutturaDati *struttura_dati, char *richiesta, int presta, pthread_mutex_t *libri_mutex, pthread_cond_t *libri_cond)
{
    char *libri_richiesti = NULL;
    int risultato_richiesta = str_d_chiediLibri(struttura_dati, &libri_richiesti, richiesta, presta, libri_mutex, libri_cond);
    if (risultato_richiesta == ERR_SYSTEM_CALL)
        return ERR_SYSTEM_CALL;

    if (risultato_richiesta == ERR_FORMATO_STR)
        printf("La richiesta \"%s\" non è nel formato corretto.\n", richiesta);

    if (!risultato_richiesta)
        printf("Nessun libro corrisponde alla richiesta:\n prestito=%d \n\"%s\"\n", presta, richiesta);
    else
    {
        printf("Sono stati trovati %d libri per la richiesta:\n prestito=%d\n \"%s\"\n\n%s\n", risultato_richiesta, presta, richiesta, libri_richiesti);
        free(libri_richiesti);
        libri_richiesti = NULL;
    }

    return SUCCESS;
}

int main(int argc, char *argv[])
{

    struct strutturaDati struttura_dati;

    int errore = str_d_genera(&struttura_dati, FILE_RECORD);
    if (errore == ERR_FORMATO_STR)
    {
        printf("La libreria contiene un libro che non è scritto nel formato corretto\n");
        return FAILURE;
    }
    if (errore == ERR_SYSTEM_CALL)
    {
        perror(ERR_SYSTEM_CALL_MSG);
        return FAILURE;
    }

    printf("struttura dati generata correttamente\n");

    pthread_mutex_t libri_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t libri_cond = PTHREAD_COND_INITIALIZER;

    char richiesta[10][100] = {"autore: Di Ciccio, Antonio;",
                               "editore:  Morgan Kaufmann; anno: 2011; nota:;",
                               "titolo: Manuale di architettura fiorentina;",
                               "autore: Steel, Graham;",
                               "autore: Pagli, Linda;",
                               "autore: Luccio, Fabrizio;",
                               "autore: Steel, Graham;  autore: Pagli, Linda;autore: Luccio, Fabrizio;",
                               "autore: Graham;  autore: Pagli, Linda;autore: Luccio, F;"};

    for (int i = 0; i < 8; i++)
    {
        if (test_chiediLibri(&struttura_dati, richiesta[i], 0, &libri_mutex, &libri_cond) == ERR_SYSTEM_CALL ||
            test_chiediLibri(&struttura_dati, richiesta[i], 1, &libri_mutex, &libri_cond) == ERR_SYSTEM_CALL)
        {
            perror(ERR_SYSTEM_CALL_MSG);
            goto failure;
        }
    }

    errore = str_d_aggiornaFileRecord(&struttura_dati, FILE_RECORD, BUILD_DIR);
    if (errore == ERR_SYSTEM_CALL)
    {
        perror(ERR_SYSTEM_CALL_MSG);
        goto failure;
    }
    if (errore == ERR_BUFFER_OVERFLOW || errore == ERR_SCRITTURA_FILE)
    {
        printf(errore == ERR_BUFFER_OVERFLOW ? ("La chiamata a str_d_aggiornaFileRecord ha riportato un errore di Buffer Overflow. "
                                                "Questo vuol dire che la dimensione del path di file record + build directory è troppo lunga.\n")
                                             : ("La chiamata a str_d_aggiornaFileRecord ha riportato un errore di Scrittura nel File. "
                                                "Questo vuol dire che un libro non è stato scritto del tutto ed la funzione si è interrotta"
                                                "prima di sovrascrivere il file record vecchio con quello aggiornato.\n"));
        goto failure;
    }
    printf("file record aggiornato correttamente\n");

    str_d_dealloca(&struttura_dati);
    printf("struttura dati deallocata con successo\n");

    return SUCCESS;

failure:
    str_d_dealloca(&struttura_dati);
    return FAILURE;
}