#include "../include/bib_conf.h"
#include <stdio.h>
#include <unistd.h> // per sleep()

int main()
{
    char *bib_path = "config/bib.conf";   // Percorso del file di configurazione
    char *socket_path = "path/to/socket"; // Percorso del socket da aggiungere
    char *bib_name = "NomeBiblioteca";    // Nome della biblioteca da aggiungere

    if (bib_addSocket(bib_path, socket_path, bib_name) != SUCCESS)
    {
        printf("Errore nell'aggiungere il socket.\n");
        return 1;
    }

    sleep(30); // Attendi 30 secondi

    if (bib_removeSocket(bib_path, socket_path, "build/") != SUCCESS)
    {
        printf("Errore nel rimuovere il socket.\n");
        return 1;
    }

    return 0;
}
