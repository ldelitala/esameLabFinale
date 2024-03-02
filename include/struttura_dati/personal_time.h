#ifndef PERSONAL_TIME_H
#define PERSONAL_TIME_H

#include <stdio.h>
#include <time.h>

// Definizioni dei codici di errore
#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef ERR_FORMATO
#define ERR_FORMATO -1
#endif

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -2
#endif

#ifndef ERR_BUFFER_OVERFLOW
#define ERR_BUFFER_OVERFLOW -3
#endif

// Dichiarazione delle funzioni
int pt_estraiData(struct tm *dst, const char *src);
int pt_currentTime(struct tm *dst);
int pt_secDiff(struct tm *time1, struct tm *time2);
int pt_creaStringaData(char *dst, size_t size, const struct tm *src);

#endif // PERSONAL_TIME_H
