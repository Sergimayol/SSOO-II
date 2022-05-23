/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/
#include "directorios.h"

int main(int argc, char **argv)
{
    // Comprobar sintaxis
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return -1;
    }
    // Comprobar si es un fichero (ruta original)
    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr, "Error: La ruta del fichero original no es un fichero\n");
        return -1;
    }
    // Comprobar si es un fichero (ruta nueva)
    if (argv[3][strlen(argv[3]) - 1] == '/')
    {
        fprintf(stderr, "Error: La ruta de enlace no es un fichero\n");
        return -1;
    }
    // Montar disco
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error: al montar el dispositivo.\n");
        return -1;
    }
    // Crear enlace
    if (mi_link(argv[2], argv[3]) < 0)
    {
        return -1;
    }
    // Desmontar disco
    bumount();
    return 0;
}
