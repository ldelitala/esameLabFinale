#ifndef SOCKET_COMUNICATION_H
#define SOCKET_COMUNICATION_H

#include <pthread.h>
#include <semaphore.h>
#include <poll.h>

#include "coda_condivisa.h"

#ifndef MAX_CLIENTS
#define MAX_CLIENTS 40
#endif

#ifndef ERR_SYSTEM_CALL
#define ERR_SYSTEM_CALL -1
#endif

#ifndef ERR_COMUNICAZIONE
#define ERR_COMUNICAZIONE -2
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

/**
 * @brief Apre un socket server associato a un percorso UNIX univoco.
 *
 * Questa funzione crea un socket server, lo associa al percorso UNIX univoco specificato e inizia
 * ad ascoltare per connessioni in entrata. Se qualsiasi passaggio fallisce, esegue le operazioni di
 * pulizia necessarie e restituisce un codice di errore.
 *
 * @return Restituisce il file descriptor del socket server in caso di successo, ERR_SYSTEM_CALL in caso di fallimento.
 */
int sockcom_apriServer(char socket_path_univoco[108]);

/**
 * @brief Configura e avvia un server per gestire connessioni client asincrone con poll(), interrompibile via signal handler.
 *
 * @details
 * Passi principali:
 * - Imposta il server per accettare connessioni.
 * - Usa un ciclo infinito con `poll()` per monitorare eventi sui file descriptor.
 * - Accetta connessioni client, gestisce richieste e le inserisce in coda.
 * - Gestisce errori e pulizia risorse.
 *
 * Richiede configurazione pre-esecuzione del signal handler per garantire chiusura pulita dei file descriptor.
 *
 * @warning Da grandi poteri derivano grandi responsabilità. Necessaria configurazione del signal handler e
 *          dimensionamento corretto di `poll_fds` a `MAX_CLIENTS + 1`.
 *
 * @param poll_fds Inserire fd del server in poll_fds[0].fd. Gestisce fino a `MAX_CLIENTS`.
 * @return `ERR_SYSTEM_CALL` per errore, altrimenti esecuzione continua fino a interruzione.
 */
int sockcom_avviaServer(struct pollfd poll_fds[MAX_CLIENTS + 1], struct coda_condivisa *coda);

/**
 * @brief Trasmette una risposta a un client controllando la prontezza del fd con `poll()`.
 *
 * Verifica la disponibilità del fd del client per l'invio (POLLOUT) usando `poll()` e invia la risposta
 * (`type`, `length`, `data`) sequenzialmente.
 *
 * @return
 * - `SUCCESS` per invio riuscito.
 * - `ERR_SYSTEM_CALL` per errori di sistema nell'invio.
 * - `ERR_COMUNICAZIONE` per errori di comunicazione, come chiusura inaspettata del canale o fd non valido.
 *
 * @warning Da grandi poteri derivano grandi responsabilità. È compito del chiamante chiudere il fd del client
 *          e liberare la memoria. In caso di errore, usare shutdown prima di chiudere.
 */
int sockcom_server_trasmettiRisposta(int client_fd, struct messaggio *risposta);

/**
 * @brief Invia una richiesta a un server tramite socket UNIX.
 *
 * Stabilisce una connessione con il server specificato dal percorso del socket UNIX e invia una richiesta
 * formattata secondo la struttura `messaggio`. La funzione si occupa di aprire il socket, connettersi al server,
 * inviare la richiesta e poi chiudere il canale in scrittura.
 *
 * @return
 * - Il file descriptor del socket in caso di successo, permettendo al chiamante di ricevere una risposta se necessario.
 * - `ERR_SYSTEM_CALL` in caso di errore nella creazione del socket, nella connessione, nell'invio dei dati o nella chiusura
 *    del canale.
 *
 * @warning È compito del chiamante gestire la chiusura del socket e deallocare le risorse allocate per `richiesta` dopo l'uso.
 *          La funzione chiude il canale in scrittura dopo l'invio della richiesta ma lascia aperto il socket per eventuali
 *          letture della risposta dal server.
 *
 * @note se errno è settato su ECONNREFUSED la connessione è stata rifiutata
 */
int sockcom_client_mandaRichiesta(char socketServerPath[108], struct messaggio *richiesta);

/**
 * @brief Riceve una risposta dal server tramite un socket aperto.
 *
 * Legge la risposta inviata dal server su un socket specificato, allocando dinamicamente lo spazio necessario per i dati
 * ricevuti e popolando la struttura `messaggio` fornita. La funzione si aspetta di ricevere prima il tipo e la lunghezza
 * della risposta, seguiti dai dati effettivi.
 *
 * @return
 * - `SUCCESS` se la risposta viene ricevuta correttamente.
 * - `ERR_SYSTEM_CALL` se si verifica un errore di lettura dal socket.
 * - `ERR_COMUNICAZIONE` se la comunicazione viene interrotta inaspettatamente, ad esempio se il server chiude la connessione
 *    prima di inviare l'intera risposta.
 *
 * @warning Da grandi poteri derivano grandi responsabilità. Dopo l'utilizzo, il chiamante è responsabile della deallocazione
 *          della memoria allocata per `risposta->data` e della chiusura del file descriptor `sock_fd`. Questo assicura la
 *          corretta gestione delle risorse e previene perdite di memoria.
 */
int sockcom_client_riceviRisposta(int sock_fd, struct messaggio *risposta);

#endif