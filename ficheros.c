#include "ficheros.h"
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }

    if ((inodo.permisos & 2) != 2)
    {
        // Error de permiso
        fprintf(stderr, "Error: Permiso de escritura denegado. \n");
        return 0;
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    if (primerBL == ultimoBL)
    {
        // Código
    }
    else
    {
        // Código
    }
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    int leidos;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }
    if ((inodo.permisos & 4) != 4)
    {
        // Error de permiso
        fprintf(stderr, "Error: Permiso de lectura denegado. \n");
        return 0;
    }
    if (offset >= inodo.tamEnBytesLog)
    {
        leidos = 0; // No podemos leer nada
        return leidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    { // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF
    }
    // Código
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    
}