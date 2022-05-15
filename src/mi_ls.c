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
        fprintf(stderr, "Error de sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return -1;
    }
    // Montar disco
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando disco.\n");
        return -1;
    }
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);
    int total = mi_dir(argv[2], buffer, '\0');
    if (total < 0)
    {
        return -1;
    }

    if (total >= 0)
    {
        printf("Total: %d\n", total);
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------------------\n");
        printf("%s\n", buffer);
    }

    if (bumount() == -1)
    {
        fprintf(stderr, "Error desmontando disco.\n");
        return -1;
    }

    return 0;
}