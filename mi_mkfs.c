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
            return -1;
        }
        for (size_t i = 0; i < argv[2]; i++)
        {
            /* code */
        }
        
    }
    else
    {
        // Error (parámetros incorrectos)
        return -1;
    }
}