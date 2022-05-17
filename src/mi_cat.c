/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        // No es un fichero
        fprintf(stderr, "No es un fichero\n");
        return -1;
    }
    // Es un fichero
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando disco.\n");
        return -1;
    }

    unsigned int offset = 0, bytes_leidos = 0, total_bytes_leidos = 0;
    char *camino = argv[2];
    int tambuffer = BLOCKSIZE * 4;
    char read[tambuffer];
    if (memset(read, 0, sizeof(read)) == NULL)
    {
        // Filtramos basura
        return -1;
    }
    bytes_leidos = mi_read(camino, read, offset, tambuffer);
    while (bytes_leidos > 0)
    {
        write(1, read, bytes_leidos); // Motrar resultados por pantalla
        total_bytes_leidos += bytes_leidos;
        offset += tambuffer;

        if (memset(read, 0, sizeof(read)) == NULL)
        {
            // Limpieza
            return -1;
        }
        bytes_leidos = mi_read(camino, read, offset, tambuffer);
    }
    fprintf(stderr, "\nSe han leido %d bytes\n", total_bytes_leidos);
    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}