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
        printf("Sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n");
        exit(1);
    }

    // Diferenciamos entre fichero y directorio
    const char *camino = argv[3];
    if (camino[strlen(camino) - 1] != '/')
    {
        fprintf(stderr, "Error (mi_mkdir.c): No es un directorio");
        return -1;
    }

    char *nombre_fichero = argv[1];
    if (bmount(nombre_fichero) == -1)
    {
        printf("Error (mi_mkdir.c) en montar el disco %s\n", nombre_fichero);
        exit(1);
    }

    int permisos = atoi(argv[2]);
    // Hay que comprobar que permisos sea un nº válido (0-7).
    if (permisos < 0 || permisos > 7)
    {
        printf("Error : modo inválido: <<%d>> \n", permisos);
        exit(1);
    }

    if (mi_creat(camino, permisos) == -1)
    {
        exit(1);
    }

    if (bumount() == -1)
    {
        printf("Error (mi_mkdir.c) en desmontar el disco %s\n", nombre_fichero);
        exit(1);
    }

    return 0;
}