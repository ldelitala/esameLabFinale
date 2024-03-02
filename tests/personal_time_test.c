#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Assicurati di includere l'header della tua libreria qui
#include "../include/personal_time2.h"

int main()
{
    struct tm data1, data2, data3;
    char dataString[25];
    int diff;

    // Test pt_estraiData
    printf("Test pt_estraiData:\n");
    if (pt_estraiData(&data1, "01-01-202012:30:45") == SUCCESS)
    {

        printf("Data estratta correttamente: %04d-%02d-%02d %02d:%02d:%02d\n",
               data1.tm_year + 1900, data1.tm_mon + 1, data1.tm_mday,
               data1.tm_hour, data1.tm_min, data1.tm_sec);
    }
    else
    {
        printf("Errore nell'estrazione della data.\n");
    }

    // Test pt_currentTime
    printf("\nTest pt_currentTime:\n");
    if (pt_currentTime(&data2) == SUCCESS)
    {
        strftime(dataString, sizeof(dataString), "%Y-%m-%d %H:%M:%S", &data2);
        printf("Ora corrente 1: %s\n", dataString);
    }
    else
    {
        printf("Errore nell'ottenere l'ora corrente.\n");
    }

    // Test pt_currentTime
    printf("\nTest pt_currentTime:\n");
    if (pt_currentTime(&data3) == SUCCESS)
    {
        strftime(dataString, sizeof(dataString), "%Y-%m-%d %H:%M:%S", &data3);
        printf("Ora corrente: %s\n", dataString);
    }
    else
    {
        printf("Errore nell'ottenere l'ora corrente.\n");
    }

    // Test pt_secDiff
    printf("\nTest pt_secDiff:\n");
    diff = pt_secDiff(&data1, &data2);
    if (diff >= 0)
    {
        printf("Differenza in secondi: %d\n", diff);
    }
    else
    {
        printf("Errore nel calcolo della differenza in secondi.\n");
    }

    // Test pt_secDiff
    printf("\nTest pt_secDiff:\n");
    diff = pt_secDiff(&data2, &data3);
    if (diff >= 0)
    {
        printf("Differenza in secondi: %d\n", diff);
    }
    else
    {
        printf("Errore nel calcolo della differenza in secondi.\n");
    }

    // Test pt_creaStringaData
    printf("\nTest pt_creaStringaData:\n");
    if (pt_creaStringaData(dataString, sizeof(dataString), &data1) == SUCCESS)
    {
        printf("Data formattata: %s\n", dataString);
    }
    else
    {
        printf("Errore nella formattazione della data.\n");
    }

    return 0;
}
