#define _XOPEN_SOURCE
#include "../../include/struttura_dati/personal_time.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

// Definizioni dei codici di errore
#define SUCCESS 0
#define ERR_FORMATO -1
#define ERR_SYSTEM_CALL -2
#define ERR_BUFFER_OVERFLOW -3

int pt_estraiData(struct tm *dst, const char *src)
{
    memset(dst, 0, sizeof(struct tm)); // Pulisce la struttura tm
    if (strptime(src, "%d-%m-%Y%H:%M:%S", dst) == NULL)
        return ERR_FORMATO;

    // Valida i campi della data
    if (dst->tm_mday > 31 ||
        dst->tm_mon > 11 ||           // tm_mon è 0-based
        dst->tm_year + 1900 < 2000 || // tm_year è l'anno meno 1900
        dst->tm_sec >= 60 ||
        dst->tm_min >= 60 ||
        dst->tm_hour >= 24)
        return ERR_FORMATO;

    return SUCCESS;
}

int pt_currentTime(struct tm *dst)
{
    time_t t;
    struct tm *now;

    time(&t);
    now = localtime(&t);
    if (!now)
    {
        perror("Errore in localtime: impossibile ottenere l'ora locale");
        return ERR_SYSTEM_CALL;
    }

    memcpy(dst, now, sizeof(struct tm)); // Copia i valori in dst

    return SUCCESS;
}

int pt_secDiff(struct tm *time1, struct tm *time2)
{
    time_t t1 = mktime(time1), t2 = mktime(time2);
    if (t1 == (time_t)-1 || t2 == (time_t)-1)
    {
        perror("Errore in mktime: impossibile convertire la struttura tm in time_t");
        return ERR_SYSTEM_CALL;
    }

    double diff = difftime(t2, t1);

    return (int)diff;
}

int pt_creaStringaData(char *dst, size_t size, const struct tm *src)
{
    if (size < 20 * sizeof(char))
    {
        printf("Errore di overflow del buffer: la dimensione del buffer di destinazione è insufficiente\n");
        return ERR_BUFFER_OVERFLOW;
    }

    if (strftime(dst, size, "%d-%m-%Y %H:%M:%S", src) == 0)
    {
        printf("Errore nella formattazione della stringa data\n");
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}
