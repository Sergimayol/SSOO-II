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

    int tambuffer = BLOCKSIZE * 4;
    int bytes_leidos = 0;
    int offset = 0;
    char buffer[tambuffer];
    memset(buffer, 0, sizeof(buffer));
    // Leemos todo el fichero o hasta completar el buffer
    int bytes_leidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    while (bytes_leidosAux > 0)
    {
        // Actualiza el número de bytes leidos.
        bytes_leidos += bytes_leidosAux;
        // Escribe el contenido del buffer en el destino indicado.
        write(1, buffer, bytes_leidosAux); // imprime por pantalla
        // Limpia el buffer de lectura, actualiza el offset y vuelve a leer.
        memset(buffer, 0, sizeof(buffer));
        offset += tambuffer;
        bytes_leidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    }
    if (bytes_leidos < 0)
    {
        mostrar_error_buscar_entrada(bytes_leidos);
        bytes_leidos = 0;
    }
    fprintf(stderr, "\nTotal_leidos: %d\n", bytes_leidos);

    // Desmontar disco
    bumount();
    return 0;
}