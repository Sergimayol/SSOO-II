#include "ficheros.h"

// Escribe el contenido procedente de un buffer de memoria, buf_original,
// de tamaño nbytes, en un fichero/directorio
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int primerBL, ultimoBL, desp1, desp2, nbfisico = 0, nbytesEscritos = 0, auxbytesEscritos = 0;
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }

    if ((inodo.permisos & 2) != 2)
    {
        // Error de permiso
        fprintf(stderr, "Error: Permiso de escritura denegado. \n");
        return -1;
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // Obtencion del numero de bloque fisico
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if (nbfisico == -1)
    {
        return -1;
    }
    // Leemos el bloque fisico
    if (bread(nbfisico, buf_bloque) == -1)
    {
        return -1;
    }

    if (primerBL == ultimoBL)
    {
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
            return -1;
        }
        nbytesEscritos += nbytes;
    }
    else if (primerBL < ultimoBL)
    {
        // Escribimos el 1er bloque
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        auxbytesEscritos = bwrite(nbfisico, buf_bloque);
        if (auxbytesEscritos == -1)
        {
            return -1;
        }

        nbytesEscritos += BLOCKSIZE - desp1;

        // Escribimos los bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1)
            {
                return -1;
            }
            nbytesEscritos += BLOCKSIZE;
        }
        // Ultimo bloque
        int nUltimobloqueFisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (nUltimobloqueFisico == -1)
        {
            return -1;
        }

        if (bread(nUltimobloqueFisico, buf_bloque) == -1)
        {
            return -1;
        }
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        if (bwrite(nbfisico, buf_bloque) == -1)
        {
            return -1;
        }

        nbytesEscritos += desp2 + 1;
    }
    // Leer el inodo actualizado.
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }

    // Actualizar la metainformación
    if (inodo.tamEnBytesLog < (nbytes + offset))
    {
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        return -1;
    }
    if (nbytes != nbytesEscritos)
    {
        return -1;
    }
    return nbytesEscritos;
}

// Lee información de un fichero/directorio (correspondiente al nº de inodo,
// ninodo, pasado como argumento) y la almacena en un buffer de memoria, buf_original
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    unsigned char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    int leidos;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
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
    // Primer bloque
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    int nbytesLeidos = 0;

    memset(buf_bloque, 0, sizeof(buf_bloque));

    if (primerBL == ultimoBL)
    {
        int BF = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (BF != -1)
        {
            if (bread(BF, buf_bloque) == -1)
            {
                fprintf(stderr, "Error al leer buf_bloque || mi_read_f() \n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
            nbytesLeidos = nbytes;
        }
    }
    else
    { // Caso de varios
        // Primer bloque
        int nbloqueFisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbloqueFisico != -1)
        {
            if (bread(nbloqueFisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error al lectura \n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        nbytesLeidos = BLOCKSIZE - desp1;

        // Bloque intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nbloqueFisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbloqueFisico != -1)
            {
                if (bread(nbloqueFisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error al lectura \n");
                    return -1;
                }
                memcpy((buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE), buf_bloque, BLOCKSIZE);
                nbytesLeidos += desp2 + 1;
            }
        }

        // ultimo bloque
        nbloqueFisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbloqueFisico != 1)
        {
            if (bread(nbloqueFisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error al lectura \n");
                return -1;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
            nbytesLeidos += desp2 + 1;
        }
    }

    // Actualizar metadatos
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error al lectura inodo \n");
        return -1;
    }
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        fprintf(stderr, "Error al escritura inodo || mi_read_f() \n");
        return -1;
    }

    return nbytesLeidos;
}

// Devuelve la metainformación de un fichero/directorio
//(correspondiente al nº de inodo pasado como argumento):
// tipo, permisos, cantidad de enlaces de entradas en directorio,
// tamaño en bytes lógicos, timestamps y cantidad de bloques ocupados
// en la zona de datos, es decir todos los campos menos los punteros.
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
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

// Cambia los permisos de un fichero/directorio (correspondiente
// al nº de inodo pasado como argumento, ninodo) con el valor que
// indique el argumento permisos.
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error al lectura inodo \n");
        return -1;
    }

    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, inodo) == -1)
    {
        fprintf(stderr, "Error al escribir inodo \n");
        return -1;
    }
    return 0;
}

// Trunca un fichero/directorio (correspondiente al nº de inodo,
// ninodo, pasado como argumento) a los bytes indicados como nbytes,
// liberando los bloques necesarios.
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    unsigned int primerBL = 0;
    // bloques liberados
    int bloquesL;
    if (leer_inodo(ninodo, &inodo) < 0)
    {
        return -1;
    }
    // comprobar que el inodo tenga permisos de escritura
    if ((inodo.permisos & 2) == 2)
    {
        if (nbytes >= inodo.tamEnBytesLog)
        {
            return -1;
        }
        // saber que nº de bloque lógico le hemos de pasar como primer bloque lógico a liberar
        if (nbytes % BLOCKSIZE == 0)
        {
            primerBL = nbytes / BLOCKSIZE;
        }
        else
        {
            primerBL = nbytes / BLOCKSIZE + 1;
        }
        bloquesL = liberar_bloques_inodo(primerBL, &inodo);
        // Actualizar mtime, ctime, el tamaño en bytes lógicos del inodo, tamEnBytesLog
        // y el número de bloques ocupados del inodo
        inodo.numBloquesOcupados -= bloquesL;
        inodo.tamEnBytesLog = nbytes;
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        escribir_inodo(ninodo, inodo);
        // Devolver la cantidad de bloques liberados.
        return bloquesL;
    }
    return -1;
}