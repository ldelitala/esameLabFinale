#include "../../include/struttura_dati/arrayCampi.h"
#include <string.h>
#include <stdlib.h>

//! FUNZIONI PRIVATE

/**
 * @brief Confronta due elementi `valoreLibro`, considerando uguali se uno contiene l'altro.
 *
 * Confronta le stringhe `valoreCampo` di due elementi `valoreLibro`. Se `elemento2` contiene `elemento1`,
 * considera i due elementi uguali, altrimenti usa strcmp per il confronto diretto.
 *
 * @return int 0 se `elemento2` contiene `elemento1`, altrimenti il risultato di strcmp.
 */
int valoreLibro_confronta(const void *elemento1, const void *elemento2)
{
    const struct valoreLibro *elemento1Cast = (const struct valoreLibro *)elemento1;
    const struct valoreLibro *elemento2Cast = (const struct valoreLibro *)elemento2;

    if (strstr(elemento2Cast->valoreCampo, elemento1Cast->valoreCampo))
        return 0;

    return strcmp(elemento1Cast->valoreCampo, elemento2Cast->valoreCampo);
}

/**
 * @brief Libera un elemento `valoreLibro`.
 *
 * Questa è una funzione privata che libera la memoria allocata per la stringa `valoreCampo` di un elemento `valoreLibro`.
 * Imposta anche i puntatori `valoreCampo` e `libroAssociato` a NULL.
 */
void valoreLibro_free(const void *elemento)
{
    if (!elemento)
        return;

    struct valoreLibro *elementoCast = (struct valoreLibro *)elemento;

    free(elementoCast->valoreCampo);
    elementoCast->valoreCampo = NULL;
    elementoCast->libroAssociato = NULL;
}

/**
 * @brief Libera un elemento `campoAlbero` e il suo albero binario associato.
 *
 * Questa funzione privata libera la memoria allocata per la stringa `nomeCampo` di un elemento `campoAlbero` e
 * libera ricorsivamente l'intero albero binario di elementi `valoreLibro` ad esso associato.
 */
void campoAlbero_free(struct campoAlbero *elemento)
{
    if (elemento && elemento->nomeCampo)
    {
        free(elemento->nomeCampo);
        elemento->nomeCampo = NULL;
        bt_visitInOrder(&(elemento->alberoValori), valoreLibro_free);
        bt_freeTree(&(elemento->alberoValori));
    }
}

/**
 * @brief Aggiunge una coppia chiave-valore all'array dinamico specificato di elementi `campoAlbero`.
 *
 * Questa funzione inserisce una nuova coppia chiave-valore nell'array dinamico. Se la chiave (campo) esiste già, aggiunge il valore all'albero binario
 * della chiave esistente. Se la chiave non esiste, crea un nuovo elemento `campoAlbero`, inizializza un albero binario per esso
 * e inserisce il valore.
 *
 * @return int SUCCESS se l'operazione è riuscita, ERR_SYSTEM_CALL in caso di errore.
 */
int arrCampi_aggiungiCoppia(struct dynamic_array *arrayCampi, const char *campo, const char *valore, struct libro *puntatoreLibro)
{
    struct valoreLibro elementoDaInserire = {.valoreCampo = strdup(valore), .libroAssociato = puntatoreLibro};
    if (!(elementoDaInserire.valoreCampo))
    {
        perror("Errore in strdup per valoreCampo");
        return ERR_SYSTEM_CALL;
    }

    for (int index = 0; index < arrayCampi->da_inserted; ++index)
    {
        struct campoAlbero *corrente = (struct campoAlbero *)da_at(arrayCampi, index);
        if (strcmp(corrente->nomeCampo, campo) == 0)
        {
            if (bt_insert(&(corrente->alberoValori), &elementoDaInserire) != FAILURE)
                return SUCCESS;
            else
            {
                printf("Errore nell'inserimento nel binary tree\n");
                goto cleanup;
            }
        }
    }

    struct campoAlbero nuovoCampo = {.nomeCampo = strdup(campo), .alberoValori = bt_create(sizeof(struct valoreLibro), valoreLibro_confronta)};
    if (!(nuovoCampo.nomeCampo))
    {
        perror("Errore in strdup per nuovoCampo.nomeCampo");
        goto cleanup;
    }

    if (bt_insert(&(nuovoCampo.alberoValori), &elementoDaInserire) == FAILURE || da_append(arrayCampi, &nuovoCampo) == FAILURE)
    {
        printf("Errore nell'inserimento del nuovo campo o nell'appending all'array dinamico\n");
        campoAlbero_free(&nuovoCampo);
        goto cleanup;
    }

    return SUCCESS;

cleanup:
    valoreLibro_free(&elementoDaInserire);
    return ERR_SYSTEM_CALL;
}
//! FUNZIONI PUBBLICHE

int arrCampi_aggiungiLibro(struct dynamic_array *arrayCampi, struct libro *libro)
{
    char *stringa_libro = strdup(libro->lib_stringa),
         *campo_temp, *valore_temp;

    if (!stringa_libro)
    {
        perror("Errore in strdup per stringa_libro");
        return ERR_SYSTEM_CALL;
    }

    lib_estraiCoppia(stringa_libro, &campo_temp, &valore_temp);
    do
    {
        lib_formattaStringa(campo_temp);
        lib_formattaStringa(valore_temp);

        if (arrCampi_aggiungiCoppia(arrayCampi, campo_temp, valore_temp, libro) == ERR_SYSTEM_CALL)
        {
            free(stringa_libro);
            printf("Errore nell'aggiunta di una coppia campo-valore\n");
            return ERR_SYSTEM_CALL;
        }

    } while (lib_estraiCoppia(NULL, &campo_temp, &valore_temp)); // Continua estraendo la coppia successiva

    free(stringa_libro);
    return SUCCESS;
}

int arrCampi_generaLista(struct dynamic_array *arrayCampi, struct dynamic_array *lista_libri, const char *richiesta, pthread_mutex_t *mutex, pthread_cond_t *cond)
{
    char *primo_campo, *primo_valore,
        *copia_richiesta = strdup(richiesta);
    if (!copia_richiesta)
    {
        perror("Errore in strdup per copia_richiesta");
        return ERR_SYSTEM_CALL;
    }

    lib_estraiCoppia(copia_richiesta, &primo_campo, &primo_valore);

    struct campoAlbero *corrente = NULL;
    int index;

    // troviamo l'albero relativo al campo che corrisponde alla richiesta
    for (index = 0; index < arrayCampi->da_inserted; index++)
    {
        corrente = (struct campoAlbero *)da_at(arrayCampi, index);
        if (strcmp(corrente->nomeCampo, primo_campo) == 0)
            break;
    }
    if (index == arrayCampi->da_inserted)
        return 0;

    struct binary_tree_node *nodo_albero = (corrente->alberoValori).bt_root;

    struct valoreLibro da_cercare = (struct valoreLibro){.valoreCampo = primo_valore, .libroAssociato = NULL},
                       *trovato;

    while ((nodo_albero = bt_node_search(nodo_albero, &da_cercare, sizeof(struct valoreLibro), valoreLibro_confronta)) != NULL)
    {
        trovato = (struct valoreLibro *)nodo_albero->bt_node_element;
        int controllo = lib_controllaRichiestaThreadSafe(trovato->libroAssociato, richiesta, mutex, cond);
        if (controllo == ERR_SYSTEM_CALL || (controllo && da_append(lista_libri, &(trovato->libroAssociato)) == FAILURE))
        {
            printf("Errore durante la verifica della richiesta thread-safe o durante l'append alla lista dei libri\n");
            free(copia_richiesta);
            return ERR_SYSTEM_CALL;
        }
        nodo_albero = nodo_albero->bt_node_right;
    }

    free(copia_richiesta);

    return SUCCESS;
}

void arrCampi_free(struct dynamic_array *arrayCampi)
{
    if (!arrayCampi)
        return;
    for (int index = 0; index < arrayCampi->da_inserted; index++)
        campoAlbero_free((struct campoAlbero *)da_at(arrayCampi, index));
}