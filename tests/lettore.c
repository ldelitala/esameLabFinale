#include "../include/bib_conf.h"
#include <stdio.h>
#include <stdlib.h> // per free()

int main()
{
    char *bib_path = "config/bib.conf"; // Percorso del file di configurazione

    char *content = bib_leggi(bib_path);
    if (content == NULL)
    {
        printf("Errore nella lettura del file.\n");
        return 1;
    }

    printf("////////////////////////////////////////////\n");
    printf("CONTENUTO FILE\n\n%s\n", content);
    printf("////////////////////////////////////////////\n");

    free(content); // Liberiamo la memoria allocata da bib_leggi
    return 0;
}
