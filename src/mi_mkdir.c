#include "directorios.h"

int main(int argc, char **argv)
{
    // Comprobación sintaxis
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    {
        fprintf(stderr, "Error de sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n");
        return -1;
    }

    // Comprobación permisos
    if ((atoi(argv[2]) < 0) || (atoi(argv[2]) > 7))
    {
        fprintf(stderr, "Error de permisos: permisos [%d] no válidos.\n", atoi(argv[2]));
        return -1;
    }

    // Comprobar de si se trata de un fichero o un directorio
    if (argv[3][strlen(argv[3]) - 1] == '/') // si no es un fichero
    {
        // Montar disco
        if (bmount(argv[1]) == -1)
        {
            fprintf(stderr, "Error montando disco.\n");
            return -1;
        }

        if (mi_creat(argv[3], atoi(argv[2])) == -1)
        {
            return -1;
        }
        if (bumount() == -1)
        {
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "No es una ruta de directorio válida.\n");
        return -1;
    }
    return 0;
}