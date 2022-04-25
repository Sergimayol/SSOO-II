#include "ficheros.h"

//
int main(int argc, char **argv)
{
    struct STAT aux_stat;
    struct tm *ts;
    char adate[80], mdate[80], cdate[80];

    // Validar sintaxis
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    {
        fprintf(stderr, "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes> \n");
        return -1;
    }
    // montar dispositivo virtual
    if (bmount(argv[1]) == -1)
    {
        fprintf(stderr, "Error montando el disco\n");
        return -1;
    }
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);
    (nbytes == 0) ? liberar_inodo(ninodo) : mi_truncar_f(ninodo, nbytes);
    // Comprobar datos inodo son correctos
    mi_stat_f(ninodo, &aux_stat);
    printf("DATOS INODO: %d\n", ninodo);
    printf("Tipo: %c\n", aux_stat.tipo);
    printf("Permisos: %d\n", aux_stat.permisos);
    ts = localtime(&aux_stat.atime);
    strftime(adate, sizeof(adate), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&aux_stat.mtime);
    strftime(mdate, sizeof(mdate), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&aux_stat.ctime);
    strftime(cdate, sizeof(cdate), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\n", adate);
    printf("mtime: %s\n", mdate);
    printf("ctime: %s\n", cdate);
    printf("numlinks: %d\n", aux_stat.nlinks);
    fprintf(stderr, "tamEnBytesLog: %d\n", aux_stat.tamEnBytesLog);
    fprintf(stderr, "numBloquesOcupados: %d\n", aux_stat.numBloquesOcupados);
    // desmontar dispositivo virtual
    if (bumount() == -1)
    {
        fprintf(stderr, "Error desmontando el disco\n");
        return -1;
    }
    return 0;
}