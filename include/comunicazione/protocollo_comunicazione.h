#ifndef PROTOCOLLO_COMUNICAZIONE_H
#define PROTOCOLLO_COMUNICAZIONE_H

// client-server
#define MSG_QUERY 'Q'
#define MSG_LOAN 'L'
#define MSG_RECORD 'R'
#define MSG_NO 'N'
#define MSG_ERROR 'E'

#define STR_ERR_SYSCALL "C'è stato un fallimento di sistema durante la ricerca dei libri richiesti.\n"
#define STR_ERR_FRMT_RIC "La richiesta inviata non è del formato corretto.\n"

// path delle varie cose
#define SOCKET_DIR "sockets/"
#define BIB_CONF_PATH "config/bib.conf"
#define LOGS_DIR "logs/"
#define BUILD_DIR "build/"
#define FILE_RECORDS_DIR "data/file_records/"

/**
 * @struct messaggio
 * @brief Struttura per rappresentare un messaggio scambiato tra client e server.
 *
 * La struttura `messaggio` è utilizzata per incapsulare i dati e le informazioni
 * necessarie per la comunicazione tra client e server. Ogni messaggio comprende
 * un tipo, i dati effettivi e la lunghezza dei dati.
 *
 * @param type
 * Tipo del messaggio, utilizzato per identificare l'azione richiesta/detta dal messaggio.
 *
 * @param data
 * Puntatore ai dati effettivi del messaggio. La memoria per i dati deve essere allocata
 * e gestita esternamente.
 *
 * @param length
 * Lunghezza dei dati in byte. Questo campo specifica la dimensione del buffer dati
 * puntato da `data`, incluso '\0'.
 */
struct messaggio
{
    char type,
        *data;
    int32_t length;
};

#endif
