/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

#define DEBUGMI_ESCRIBIR 1

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    // Comprobar si se trata de un fichero
    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr, "No es un fichero\n");
        return -1;
    }

    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando disco.\n");
        return -1;
    }

#if DEBUGMI_ESCRIBIR
    fprintf(stderr, "Longitud texto: %ld\n", strlen(argv[3]));
#endif

    // Cantidad de bytes escritos
    int nbytes = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if (nbytes == -1)
    {
        nbytes = 0;
    }
#if DEBUGMI_ESCRIBIR
    fprintf(stderr, "Bytes escritos: %d\n", nbytes);
#endif
    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}