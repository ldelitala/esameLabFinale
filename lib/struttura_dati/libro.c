#include "../../include/struttura_dati/libro.h"
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

//! FUNZIONI PRIVATE

//* FUNZIONI DI ACCESSO AL LIBRO
/**
 * @brief Acquisisce l'accesso ad un libro in modo thread-safe.
 *
 * @param cond Puntatore alla variabile di condizione utilizzata per attendere che il libro diventi disponibile.
 * @return int Restituisce `SUCCESS` se l'accesso al libro è acquisito con successo; `ERR_SYSTEM_CALL` se falliscono le chiamate di sistema.
 *
 * @note È cruciale assicurarsi che `pthread_mutex_unlock` venga chiamato prima di ritornare per evitare deadlock.
 * @warning Da grandi poteri derivano grandi responsabilità. L'uso improprio dei mutex e delle variabili di condizione può portare
 *          a deadlock, inversione di priorità e altre problematiche di concorrenza. Assicurarsi che i primitivi di sincronizzazione
 *          siano utilizzati correttamente e che tutte le vie d'uscita dalla funzione rilascino le risorse acquisite.
 */
int accedi_libro(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    // lock(mutex)
    // while(libro->in_uso==1)
    //   cond_wait(cond,mutex)
    // libro->in_uso=1
    // unlock(mutex)

    if (pthread_mutex_lock(mutex))
    {
        perror("Errore lock mutex in accedi_libro");
        return ERR_SYSTEM_CALL;
    }

    while (libro->lib_inUso == 1)
    {
        if (pthread_cond_wait(cond, mutex))
        {
            pthread_mutex_unlock(mutex);
            perror("Errore wait cond in accedi_libro");
            return ERR_SYSTEM_CALL;
        }
    }
    libro->lib_inUso = 1;

    if (pthread_mutex_unlock(mutex))
    {
        perror("Errore unlock mutex in accedi_libro");
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}

/**
 * @brief Rilascia l'accesso ad un libro in modo thread-safe.
 *
 * Questa funzione rilascia l'accesso a una risorsa libro precedentemente acquisita, impostando il campo `in_uso`
 * della struttura `libro` a 0. Utilizza un mutex per garantire l'esclusione mutua durante l'aggiornamento dello stato del libro
 * e segnala a un altro thread in attesa (se presente) che il libro è ora disponibile utilizzando una variabile di condizione.
 *
 * @return int Restituisce `SUCCESS` se il libro è stato rilasciato con successo; `ERR_SYSTEM_CALL` se falliscono le chiamate di sistema.
 *
 * @note Questa funzione dovrebbe essere chiamata dopo aver completato l'accesso al libro per permettere ad altri thread
 *       di accedere alla risorsa in modo ordinato e senza conflitti.
 * @warning Da grandi poteri derivano grandi responsabilità. È importante assicurarsi che tutte le operazioni su mutex e variabili di condizione
 *          siano eseguite correttamente per prevenire deadlock e garantire l'integrità dei dati tra i thread.
 */
int esci_libro(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond)
{

    // lock(mutex)
    // libro->in_uso=0
    // cond_signal(cond)
    // unlock(mutex)

    if (pthread_mutex_lock(mutex))
    {
        perror("Errore lock mutex in esci_libro");
        return ERR_SYSTEM_CALL;
    }

    libro->lib_inUso = 0;

    if (pthread_cond_signal(cond))
    {
        pthread_mutex_unlock(mutex);
        perror("Errore signal cond in esci_libro");
        return ERR_SYSTEM_CALL;
    }

    if (pthread_mutex_unlock(mutex))
    {
        perror("Errore unlock mutex in esci_libro");
        return ERR_SYSTEM_CALL;
    }

    return SUCCESS;
}

//*FUNZIONI PER IL PRESTITO
/**
 * @brief Controlla lo stato del prestito di un libro e determina se il periodo di prestito è scaduto.
 *
 * Questa funzione verifica se un libro, rappresentato dalla struttura `libro`, è attualmente in prestito.
 * Se il libro non è in prestito, la funzione ritorna immediatamente. Altrimenti, calcola la differenza
 * tra la data corrente e la data di inizio del prestito (`data_prestito`). Se questa differenza supera
 * 30 secondi, il prestito viene considerato scaduto, il libro viene marcato come non in prestito
 * (`prestito` impostato a 0), e la funzione ritorna 0. Se il calcolo della differenza di tempo fallisce,
 * ritorna un codice di errore.
 *
 * @return int Restituisce 0 se il libro non è in prestito o se il prestito è scaduto, 1 se il libro è ancora in prestito,
 *         o `ERR_SYSTEM_CALL` in caso di fallimento delle chiamate di sistema durante il controllo.
 *
 * @warning Gestire accuratamente i valori di ritorno e gli stati di errore per evitare comportamenti imprevisti o inconsistenze nei dati.
 */
int controllo_prestito(struct libro *libro)
{
    int error;

    if (libro->lib_inPrestito == 0)
        return 0;

    struct tm now;
    if (pt_currentTime(&now) == ERR_SYSTEM_CALL)
    {
        perror("Errore ottenimento tempo corrente in controllo_prestito");
        return ERR_SYSTEM_CALL;
    }

    error = pt_secDiff(&(libro->lib_dataPrestito), &now);
    if ((error) > 30)
    {
        libro->lib_inPrestito = 0;
        return 0;
    }
    else if (error == ERR_SYSTEM_CALL)
    {
        perror("Errore differenza secondi in controllo_prestito");
        return ERR_SYSTEM_CALL;
    }

    return 1;
}

/**
 * @brief Rimuove il campo "prestito" dalla stringa di dettagli di un libro e aggiorna le informazioni di prestito
 *        nello struct data `lib_dataPrestito`.
 *
 * Questa funzione cerca il campo "prestito" all'interno della stringa di dettagli `lib_stringa` di un libro.
 * Se il campo viene trovato, viene rimosso dalla stringa e le informazioni relative vengono utilizzate per aggiornare
 * la data di prestito e lo stato di prestito del libro. La funzione gestisce la memoria dinamicamente per l'estrazione
 * e la manipolazione dei dati necessari.
 *
 * @return int Restituisce `SUCCESS` se il campo "prestito" viene rimosso con successo e le informazioni di prestito aggiornate correttamente.
 *         Restituisce `ERR_FORMATO_DATA` se il formato della data estratta non è valido.
 *         Restituisce `ERR_SYSTEM_CALL` in caso di fallimento delle chiamate di sistema, come errori di allocazione della memoria.
 */
int rimuoviCampoPrestito(struct libro *libro)
{
    char *campoPrestito = strstr(libro->lib_stringa, "prestito:");
    if (campoPrestito)
    {
        char *puntoEVirgola = strchr(campoPrestito, ';');
        size_t len_valore = puntoEVirgola - campoPrestito - strlen("prestito:") + 1;

        char *valore = (char *)malloc(len_valore);
        if (!valore)
        {
            perror("Errore allocazione memoria in rimuoviCampoPrestito");
            return ERR_SYSTEM_CALL;
        }

        strncpy(valore, campoPrestito + strlen("prestito:"), len_valore);
        valore[len_valore - 1] = '\0';

        lib_formattaStringa(valore);

        int result = pt_estraiData(&(libro->lib_dataPrestito), valore);
        if (result == ERR_FORMATO || result == ERR_SYSTEM_CALL)
        {
            free(valore);
            perror("Errore estrazione data in rimuoviCampoPrestito");
            return (result == ERR_FORMATO) ? ERR_FORMATO_DATA : ERR_SYSTEM_CALL;
        }

        libro->lib_inPrestito = 1;

        free(valore);
        memmove(campoPrestito, puntoEVirgola + 1, strlen(puntoEVirgola + 1) + 1);

        libro->lib_stringa = realloc(libro->lib_stringa, strlen(libro->lib_stringa) + 1);
        if (!(libro->lib_stringa))
        {
            perror("Errore reallocazione memoria in rimuoviCampoPrestito");
            return ERR_SYSTEM_CALL;
        }
    }
    else
        libro->lib_inPrestito = 0;

    return SUCCESS;
}

//* FUNZIONI PER LA VISUALIZZAZIONE DELLA STRINGA LIBRO

// Funzione per verificare se un carattere è speciale.
int isSpecialChar(char c)
{
    return c == ':' || c == ',' || c == '.' || c == ';' || c == '!';
}

// Funzione per rimuovere spazi multipli e spazi prima di caratteri speciali.
void formattaPerVisualizzazione(char *str)
{
    char *temp = str;

    // Rimuovi spazi vuoti iniziali
    while (isspace((unsigned char)*temp))
        temp++;

    memmove(str, temp, strlen(temp) + 1);

    // Rimuovi spazi vuoti finali
    size_t lunghezza = strlen(str);

    while (lunghezza > 0 && isspace((unsigned char)str[lunghezza - 1]))
        lunghezza--;

    str[lunghezza] = '\0';

    // rimuovi gli spazi vuoti multipli o gli spazi vuoti prima di un carattere speciale
    int i, j = 0,
           spaceFound = 0;

    for (i = 0; str[i]; i++)
    {
        // Se il carattere corrente è uno spazio
        if (isspace((unsigned char)str[i]))
        {
            if (spaceFound)
            {
                // Se abbiamo già trovato uno spazio, ignoriamo gli spazi successivi.
                continue;
            }
            if (isSpecialChar(str[i + 1]))
            {
                // Se lo spazio è seguito da un carattere speciale, lo ignoriamo.
                continue;
            }
            spaceFound = 1;
        }
        else
        {
            spaceFound = 0;
        }
        // Copiamo il carattere nella nuova posizione.
        str[j++] = str[i];
    }
    // Aggiungiamo il terminatore di stringa.
    str[j] = '\0';
}

//! FUNZIONI PUBLICCHE

int lib_estraiCoppia(char *stringa, char **campo, char **valore)
{
    if (((*campo) = strtok(stringa, ":")) &&
        ((*valore) = strtok(NULL, ";")))
        return 1;
    return 0;
}

void lib_formattaStringa(char *stringa)
{
    char *temp = stringa;
    // Rimuovi spazi vuoti iniziali
    while (isspace((unsigned char)*temp))
    {
        temp++;
    }
    memmove(stringa, temp, strlen(temp) + 1);

    // Rimuovi spazi vuoti finali
    size_t lunghezza = strlen(stringa);
    while (lunghezza > 0 && isspace((unsigned char)stringa[lunghezza - 1]))
    {
        lunghezza--;
    }
    stringa[lunghezza] = '\0';

    // Converte i caratteri in minuscolo
    for (size_t i = 0; stringa[i]; i++)
    {
        stringa[i] = tolower((unsigned char)stringa[i]);
    }

    // Rimuovi gli spazi
    size_t j = 0;
    for (size_t i = 0; stringa[i]; i++)
    {
        if (!isspace((unsigned char)stringa[i]))
            stringa[j++] = stringa[i];
    }
    stringa[j] = '\0';
}

int lib_controllaFormatoCorretto(const char *stringa)
{
    char *duePunti, *puntoVirgola;

    // ci dev'essere almeno un campo
    if (((duePunti = strstr(stringa, ":")) == NULL) || ((puntoVirgola = strstr(duePunti, ";")) == NULL))
        return 0;

    while (((duePunti = strstr(puntoVirgola, ":")) != NULL) && ((puntoVirgola = strstr(duePunti, ";")) != NULL))
    {
        // continuo a scorrere la stringa
    }
    if (duePunti && !puntoVirgola)
        return 0; // manca l'ultimo valore

    return 1;
}

int lib_crea(struct libro *dst, const char *str)
{
    *dst = (struct libro){
        .lib_stringa = (char *)malloc(strlen(str) + 1),
        .lib_inUso = 0,
        .lib_inPrestito = 0};

    if (!(dst->lib_stringa))
    {
        perror("Errore di allocazione per lib_stringa");
        return ERR_SYSTEM_CALL;
    }

    strcpy(dst->lib_stringa, str);

    if ((dst->lib_stringa)[strlen(dst->lib_stringa) - 1] == '\n')
        (dst->lib_stringa)[strlen(dst->lib_stringa) - 1] = '\0';

    formattaPerVisualizzazione(dst->lib_stringa);

    int op = rimuoviCampoPrestito(dst);
    if (op == ERR_FORMATO_DATA || op == ERR_SYSTEM_CALL)
    {
        free(dst->lib_stringa);
        perror("Errore nella rimozione del campo prestito");
        return op;
    }

    return SUCCESS;
}

char *lib_leggi(struct libro *libro)
{
    char *result = (char *)malloc(strlen(libro->lib_stringa) + 2);
    if (!result)
    {
        perror("Errore di allocazione per result in lib_leggi");
        return NULL;
    }

    strcpy(result, libro->lib_stringa);
    int prestito = controllo_prestito(libro);
    if (prestito == ERR_SYSTEM_CALL)
    {
        free(result);
        perror("Errore di sistema in controllo_prestito");
        return NULL;
    }
    if (prestito)
    {
        result = realloc(result, strlen(libro->lib_stringa) + strlen(" prestito: gg-mm-aaaa hh:mn:sc;\n") + 1);
        if (!result)
        {
            perror("Errore di reallocazione per result in lib_leggi");
            return NULL;
        }

        char valore[20] = "gg-mm-aaaa hh:mn:sc";
        if (pt_creaStringaData(valore, sizeof(valore), &(libro->lib_dataPrestito)) == ERR_SYSTEM_CALL)
        {
            free(result);
            perror("Errore nella creazione della stringa data");
            return NULL;
        }

        strcat(result, " prestito: ");
        strcat(result, valore);
        strcat(result, ";");
    }
    strcat(result, "\n");

    return result;
}

char *lib_leggiThreadSafe(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    char *result;

    if (accedi_libro(libro, mutex, cond) == ERR_SYSTEM_CALL)
    {
        perror("Errore di accesso thread-safe al libro");
        return NULL;
    }

    result = lib_leggi(libro);

    if (esci_libro(libro, mutex, cond) == ERR_SYSTEM_CALL)
    {
        if (result)
            free(result);
        perror("Errore nell'uscita thread-safe dal libro");
        return NULL;
    }

    return result;
}

int lib_prestaThreadSafe(struct libro *libro, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    if (accedi_libro(libro, mutex, cond) == ERR_SYSTEM_CALL)
    {
        perror("Errore di accesso thread-safe al libro per prestito");
        return ERR_SYSTEM_CALL;
    }

    int statoPrestito = controllo_prestito(libro);
    if (statoPrestito == ERR_SYSTEM_CALL)
    {
        esci_libro(libro, mutex, cond);
        perror("Errore di sistema nel controllo del prestito");
        return ERR_SYSTEM_CALL;
    }

    if (statoPrestito == 1)
    {
        if (esci_libro(libro, mutex, cond) == ERR_SYSTEM_CALL)
        {
            perror("Errore nell'uscita thread-safe dal libro dopo controllo prestito");
            return ERR_SYSTEM_CALL;
        }
        return 0; // libro già in prestito
    }

    if (pt_currentTime(&(libro->lib_dataPrestito)) == ERR_SYSTEM_CALL)
    {
        esci_libro(libro, mutex, cond);
        perror("Errore nell'impostazione della data corrente per il prestito");
        return ERR_SYSTEM_CALL;
    }
    libro->lib_inPrestito = 1;

    if (esci_libro(libro, mutex, cond) == ERR_SYSTEM_CALL)
    {
        perror("Errore nell'uscita thread-safe dal libro dopo prestito");
        return ERR_SYSTEM_CALL;
    }

    return 1;
}

int lib_controllaRichiestaThreadSafe(struct libro *libro, const char *richiesta, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    char *copia_ric, *stringa_libro;

    copia_ric = strdup(richiesta);
    if (!copia_ric)
    {
        perror("Errore di duplicazione della richiesta");
        return ERR_SYSTEM_CALL;
    }

    stringa_libro = lib_leggiThreadSafe(libro, mutex, cond);
    if (!stringa_libro)
    {
        free(copia_ric);
        perror("Errore nella lettura thread-safe del libro");
        return ERR_SYSTEM_CALL;
    }

    lib_formattaStringa(stringa_libro);

    char *campo_temp, *valore_temp;

    lib_estraiCoppia(copia_ric, &campo_temp, &valore_temp);

    do
    {
        lib_formattaStringa(campo_temp);
        lib_formattaStringa(valore_temp);

        char *ricorrenza_campo = stringa_libro, *ricorrenza_valore, *puntoEVirgola;
        int trovato = 0;
        while ((ricorrenza_campo = strstr(ricorrenza_campo, campo_temp)))
        {
            trovato = (ricorrenza_valore = strstr(ricorrenza_campo, valore_temp)) &&
                      (puntoEVirgola = strchr(ricorrenza_campo, ';')) &&
                      (ricorrenza_valore < puntoEVirgola);

            if (trovato)
                break;

            ricorrenza_campo++;
        }

        if (!trovato)
        {
            free(copia_ric);
            free(stringa_libro);
            return 0;
        }

    } while (lib_estraiCoppia(NULL, &campo_temp, &valore_temp));

    free(copia_ric);
    free(stringa_libro);
    return 1;
}

void lib_free(struct libro *libro)
{
    if (!libro)
        return;

    if (libro->lib_stringa)
    {
        free(libro->lib_stringa);
        libro->lib_stringa = NULL;
    }
}
