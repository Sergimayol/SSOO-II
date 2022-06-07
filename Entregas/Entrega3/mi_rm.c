/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/
#include "directorios.h"

int main(int argc, char **argv)
{
    // Comprobar sintaxis
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_rm disco /ruta\n");
        return -1;
    }
    // Montar disco
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando disco.\n");
        return -1;
    }
    // borrar entrada
    if (mi_unlink(argv[2]) == -1)
    {
        return -1;
    }
    // Desmontar disco
    bumount();
    return 0;
}
