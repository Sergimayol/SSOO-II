/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Error de sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }

    return 0;
}