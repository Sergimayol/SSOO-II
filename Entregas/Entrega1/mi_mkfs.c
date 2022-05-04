/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau  
*/

/* El programa mi_mkfs.c sirve para formatear el dispositivo
virtual con el tamaño adecuado de bloques, nbloques. Debe ser
llamado desde la línea de comandos con los siguientes parámetros
para dar nombre al dispositivo virtual y determinar la cantidad
de bloques de que dispondrá nuestro sistema de ficheros.
*/
#include "ficheros_basico.h"

// Uso:      ./mi_mkfs <nombre_dispositivo> <nbloques>
// Núm. parámetros: argc=3
// Parámetros: argv[0]="mi_mkfs", argv[1]=nombre_dispositivo, argv[2]=nbloques
int main(int argc, char **argv)
{
    // Comprobar si los parámetros son correctos
    if (argc == 3)
    {
        if (bmount(argv[1]) == -1)
        {
            // Error
            fprintf(stderr, "(mi_mkfs,bmount)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        int nbloques = atoi(argv[2]);
        if (nbloques <= 0)
        {
            // Error
            fprintf(stderr, "(mi_mkfs,num. Bloques<=0)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        // array de tipo unsigned char del tamaño de un bloque
        unsigned char buffer[BLOCKSIZE];
        // Bloque vacio
        memset(buffer, 0, BLOCKSIZE);
        for (int i = 0; i < nbloques; i++)
        {
            if (bwrite(i, buffer) == -1)
            {
                // error
                fprintf(stderr, "(mi_mkfs,bwrite)Error %d: %s\n", errno, strerror(errno));
                fprintf(stderr, "Error escribiendo bloque (%d).\n", i);
                return -1;
            }
        }
        if (initSB(nbloques, (nbloques / 4)) == -1)
        {
            fprintf(stderr, "(mi_mkfs,initSB)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        if (initMB() == -1)
        {
            fprintf(stderr, "(mi_mkfs,initMB)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        if (initAI() == -1)
        {
            fprintf(stderr, "(mi_mkfs,initAI)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
        // Creacion de direccorio raíz
        reservar_inodo('d', 7);
        if (bumount() == -1)
        {
            // error
            fprintf(stderr, "(mi_mkfs,bumount)Error %d: %s\n", errno, strerror(errno));
            return -1;
        }
    }
    else
    {
        // Error (parámetros incorrectos)
        fprintf(stderr, "Error: parametros incorrectos.\n");
        return -1;
    }
    return 0;
}