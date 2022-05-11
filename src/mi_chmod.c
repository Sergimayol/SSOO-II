/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

// Cambia los permisos de un fichero o directorio, llamando a la
// función mi_chmod() de la capa de directorios. Los permisos se
// indican en octal, será 4 para sólo lectura (r--), 2 para sólo
// escritura (-w-), 1 para sólo ejecución (--x)...
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Error de sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }
    // Montar disco
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando disco.\n");
        return -1;
    }
    struct STAT p_stat;
    if (mi_stat(argv[2], &p_stat) < 0)
    {
        return -1;
    }
    if (bumount() == -1)
    {
        fprintf(stderr, "Error desmontando disco.\n");
        return -1;
    }
    return 0;
}