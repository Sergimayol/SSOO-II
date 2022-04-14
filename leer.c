#include "ficheros.h"

/*Le pasaremos por línea de comandos un nº de inodo obtenido con el programa
  anterior (además del nombre del dispositivo). Su funcionamiento tiene que
  ser dimilar a la función cat de linux, explorando TODO el fichero.
*/
int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL)
    {
        fprintf(stderr, "Error de sintaxis, la sintaxis correcta es: leer <nombre_dispositivo> <nº inodo>\n");
        return -1;
    }
    bmount(argv[1]);

    int ninodo = atoi(argv[2]);
    struct inodo inodo;

    if (leer_inodo(atoi(argv[2]), &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo - leer.c \n");
        return -1;
    }

    int offset = 0;
    int leidos, total_leidos = 0;
    int tam = 1500;
    char buffer[tam];

    memset(buffer, 0, tam);
    if ((leidos = mi_read_f(ninodo, buffer, offset, tam)) == -1)
    {
        fprintf(stderr, "Error lectura fichero-directorio - leer.c \n");
        fprintf(stderr, "\ntotal_leídos: %d\n", total_leidos);
        return -1;
    }
    while (leidos > 0)
    {

        write(1, buffer, leidos);
        offset += tam;
        total_leidos += leidos;
        memset(buffer, 0, tam);
        if ((leidos = mi_read_f(ninodo, buffer, offset, tam)) == -1)
        {
            fprintf(stderr, "Error lectura fichero-directorio - leer.c \n");
            fprintf(stderr, "\ntotal_leídos: %d\n", total_leidos);
            return -1;
        }
    }

    fprintf(stderr, "\ntotal_leídos: %d\n", total_leidos);
    fprintf(stderr, "tamEnBytesLog: %d\n", inodo.tamEnBytesLog);

    bumount();

    return 0;
}