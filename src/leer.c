#include "ficheros.h"

/*Le pasaremos por línea de comandos un nº de inodo obtenido con el programa
  anterior (además del nombre del dispositivo). Su funcionamiento tiene que
  ser dimilar a la función cat de linux, explorando TODO el fichero.
*/
int main(int argc, char **argv)
{
    struct superbloque SB;
    struct inodo inodo;
    int leidos, total_leidos = 0, offset = 0, tam = 1500;
    char buffer[tam];

    if (argv[1] == NULL || argv[2] == NULL)
    {
        fprintf(stderr, "Error de sintaxis, la sintaxis correcta es: leer <nombre_dispositivo> <nº inodo>\n");
        return -1;
    }

    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando el dispositivo\n");
        return -1;
    }

    // Leer superbloque
    if (bread(0, &SB) == EXIT_FAILURE)
    {
        fprintf(stderr, "leer.c: Error de lectura del superbloque.\n");
        return EXIT_FAILURE;
    }

    memset(buffer, 0, tam);
    int ninodo = atoi(argv[2]);
    // Lee del fichero hasta llenar el buffer o fin de fichero.
    leidos = mi_read_f(ninodo, buffer, offset, tam);
    while (leidos > 0)
    {
        total_leidos += leidos;
        write(1, buffer, leidos);
        memset(buffer, 0, tam);
        offset += tam;
        leidos = mi_read_f(ninodo, buffer, offset, tam);
        if (leidos == -1)
        {
            fprintf(stderr, "Error lectura fichero-directorio - leer.c \n");
            fprintf(stderr, "\ntotal_leídos: %d\n", total_leidos);
            return -1;
        }
    }

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo - leer.c \n");
        return -1;
    }

    fprintf(stderr, "\ntotal_leídos: %d\n", total_leidos);
    fprintf(stderr, "tamEnBytesLog: %d\n", inodo.tamEnBytesLog);

    if (bumount() == -1)
    {
        fprintf(stderr, "Error desmontando el dispositivo\n");
        return -1;
    }

    return 0;
}