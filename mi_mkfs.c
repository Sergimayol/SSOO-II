/* El programa mi_mkfs.c sirve para formatear el dispositivo
virtual con el tamaño adecuado de bloques, nbloques. Debe ser
llamado desde la línea de comandos con los siguientes parámetros
para dar nombre al dispositivo virtual y determinar la cantidad
de bloques de que dispondrá nuestro sistema de ficheros.
*/
#include "bloques.h"

// Uso:      ./mi_mkfs <nombre_dispositivo> <nbloques>
// Núm. parámetros: argc=3
// Parámetros: argv[0]="mi_mkfs", argv[1]=nombre_dispositivo, argv[2]=nbloques
int main(int argc, char **argv)
{
    //! FALTA OPTIMIZAR
    // Comprobar si los parámetros son correctos
    if (argc == 3)
    {
        if (bmount(argv[1]) == -1)
        {
            // Error
            fprintf(stderr, "Error montando el dispositivo virtual.");
            return -1;
        }
        unsigned int nbloques = atoi(argv[2]);
        // array de tipo unsigned char del tamaño de un bloque
        unsigned char buffer[BLOCKSIZE];
        // Bloque vacio
        memset(buffer, '\0', BLOCKSIZE);
        for (size_t i = 0; i < nbloques; i++)
        {
            if (bwrite(i, buffer) == -1)
            {
                // error
                fprintf(stderr, "Error escribiendo bloque (%d).", i);
                return -1;
            }
            memset(buffer, '\0', BLOCKSIZE);
        }
        if (bumount() == -1)
        {
            // error
            fprintf(stderr, "Error desmontando el dispositivo virtual.");
            return -1;
        }
    }
    else
    {
        // Error (parámetros incorrectos)
        fprintf(stderr, "Error: parametros incorrectos.");
        return -1;
    }
}