#include "directorios.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    { // Check sintaxis
        fprintf(stderr, "Command syntax should be: mi_mkdir <disco> <permisos> </ruta> \n");
        return -1;

        int dir = strlen(argv[3]);

        if (argv[3][dir - 1] == '/')
        {
            if ((atoi(argv[2]) < 0) || (atoi(argv[2]) > 7))
            { // Check permisos del 0 al 7
                fprintf(stderr, "Permisos no validos\n");
                return -1;
            }

            if (bmount(argv[1]) == -1)
            {
                fprintf(stderr, "Error while mounting\n");
                return -1;
            }

            if (mi_creat(argv[3], atoi(argv[2])) == -1)
            {
                bumount();
                return -1;
            }
        }
        else
        {
            fprintf(stderr, "No es una ruta válida \n");
            return -1;
        }
        bumount();
        return 0;
    }
}