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
    struct superbloque SB;
    char tipo;
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    char buffer[TAMBUFFER];
    memset(buffer, 0, sizeof(buffer));
    if ((mi_dir(argv[2], buffer, &tipo)) != -1)
    {
        if ((argv[2][(strlen(argv[2]) - 1)] != '/') && (tipo == 'd'))
        {
            fprintf(stderr, "No se ha encontrado el fichero\n");
            bumount();
            return -1;
        }
        printf("Tipo\tModo\tmTime\t\tTamaño\tNombre\n");
        printf("-----------------------------------------------------\n%s", buffer);
    }
    else
    {
        bumount();
        return -1;
    }
    bumount();
    return 0;
}