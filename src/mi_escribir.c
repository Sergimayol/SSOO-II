/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"
// Solo está escribiendo la primera palabra (REVISAR)
int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL)
    {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    int pathL = strlen(argv[2]); // Ruta del fichero

    if (argv[2][pathL - 1] != '/')
    { // Es un fichero

        if (bmount(argv[1]) == -1)
        {
            fprintf(stderr, "Error while mounting\n");
            return -1;
        }
        char *camino = argv[2];
        char *buffer_texto = argv[3];
        unsigned int offset = atoi(argv[4]);
        // Cantidad de bytes escritos
        int nbytes = mi_write(camino, buffer_texto, offset, strlen(buffer_texto));

        if (nbytes == -1)
        {
            nbytes = 0;
        }
        bumount();
        fprintf(stderr, "Se han escrito %d bytes\n", nbytes);
        return 0;
    }
    else // No es un fichero
    {
        fprintf(stderr, "No es un fichero\n");
        return -1;
    }
}