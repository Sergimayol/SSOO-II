/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

#define DEBUGMI_ESCRIBIR 1

int main(int argc, char **argv)
{

    if (argc < 5)
    {
        puts("Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>");
        return -1;
    }

    unsigned int offset = atoi(argv[4]);
    const char *buffer;
    const char *camino = argv[2];
    int acumulado = 0;
    for (size_t i = 3; i < argc - 1; i++)
    {
        buffer = argv[i];
        int len = strlen(buffer);
        acumulado += len;
        if (i > 3)
        {
            acumulado++;
        }
    }

    bmount(argv[1]);
#if DEBUGMI_ESCRIBIR
    fprintf(stderr, "Longitud texto: %ld\n", acumulado);
#endif
    int resultado = mi_write(camino, buffer, offset, acumulado);
    if (resultado < 0)
        return resultado;

    printf("%d bytes escritos.\n", resultado);

    bumount();

    return 0;
}
