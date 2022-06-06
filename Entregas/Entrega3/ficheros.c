/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "ficheros.h"

// Escribe el contenido procedente de un buffer de memoria, buf_original,
// de tamaño nbytes, en un fichero/directorio
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    int bytes_escritos = 0;
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned char buf_bloque[BLOCKSIZE];

    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) == 2)
    {

        mi_waitSem();
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        mi_signalSem();
        // cabe en un solo bloque
        if (primerBL == ultimoBL)
        {

            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while reading\n");
                return -1;
            }
            memcpy(buf_bloque + desp1, buf_original, nbytes);

            if (bwrite(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while writing\n");
                return -1;
            }
            bytes_escritos += nbytes;
        }
        else
        { // No cabe en un solo bloque
            // primer bloque lógico

            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while reading\n");
                return -1;
            }
            memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

            if (bwrite(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while writing\n");
                return -1;
            }
            bytes_escritos += BLOCKSIZE - desp1;

            // bloques lógicos intermedios

            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                mi_waitSem();
                nbfisico = traducir_bloque_inodo(ninodo, i, 1);
                mi_signalSem();

                if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1)
                {
                    fprintf(stderr, "Error while writing\n");
                    return -1;
                }

                bytes_escritos += BLOCKSIZE;
            }

            //último bloque lógico
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
            mi_signalSem();

            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while reading\n");
                return -1;
            }
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

            if (bwrite(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error while writing\n");
                return -1;
            }
            bytes_escritos += desp2 + 1;
        }
        // actualizamos inodo
        mi_waitSem();
        leer_inodo(ninodo, &inodo);
        // printf("TAMENBYTESLOG %d \n",inodo.tamEnBytesLog);//SHOULD BE STAT?
        if (inodo.tamEnBytesLog < (bytes_escritos + offset))
        {
            inodo.tamEnBytesLog = bytes_escritos + offset;
            inodo.ctime = time(NULL);
        }
        inodo.mtime = time(NULL);
        escribir_inodo(ninodo, inodo);
        mi_signalSem();
        return bytes_escritos;
    }
    else
    {
        fprintf(stderr, "Inodo[%d] doesn't have writing privileges\n", ninodo);
        return -1;
    }
}

/*
 * Lee información de un fichero/directorio correspondiente al nº de inodo, ninodo, pasado como argumento y la almacena en un buffer de memoria
 *
 * Input:   ninodo          => Nº del inodo a leer
 *          *buf_original   => buffer de memoria
 *          offset          => posición inicia de lectura con respecto al inodo (en bytes)
 *          nbytes          => numero de bytes a leer
 * Output:  0 uppon success, 1 otherwise
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    int bytes_leidos = 0;
    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    inodo.atime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    // Modificamos el atime

    if (offset >= inodo.tamEnBytesLog)
    {
        return bytes_leidos; // return 0;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    if ((inodo.permisos & 4) == 4)
    { // Check permisos de lectura inodo leido

        int primerBL = offset / BLOCKSIZE;
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;

        unsigned char buf_bloque[BLOCKSIZE];

        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

        // Todo cabe en un solo bloque
        if (primerBL == ultimoBL)
        {
            if (nbfisico != -1)
            {
                if (bread(nbfisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error while reading\n");
                    return -1;
                }

                memcpy(buf_original, buf_bloque + desp1, nbytes);
            }
            bytes_leidos = nbytes;
        }
        else
        { // Los datos no caben en un solo bloque

            // Primer bloque
            if (nbfisico != -1)
            {
                if (bread(nbfisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error while reading \n");
                    return -1;
                }

                memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1); // Leemos el primer bloque
            }
            bytes_leidos = BLOCKSIZE - desp1;

            // Bloques intermedios
            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                nbfisico = traducir_bloque_inodo(ninodo, i, 0);

                if (nbfisico != -1)
                {
                    if (bread(nbfisico, buf_bloque) == -1)
                    {
                        fprintf(stderr, "Error while reading\n");
                        return -1;
                    }
                    memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE); // leemos todo el bloque
                }
                bytes_leidos += BLOCKSIZE;
            }

            // Ultimo bloque
            int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);

            if (nbfisico != -1)
            {
                if (bread(nbfisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error while reading\n");
                    return -1;
                }
                memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
            }
            bytes_leidos += desp2 + 1;
        }

        return bytes_leidos;
    }
    else
    {
        fprintf(stderr, "Inodo[%d] doesn't have reading privileges\n", ninodo);
        return -1;
    }
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
#if DEBUG11
    mi_waitSem();
#endif
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
#if DEBUG11
    mi_signalSem();
#endif
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
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }
    // comprobar que el inodo tenga permisos de escritura
    if ((inodo.permisos & 2) != 2)
    {
        return -1;
    }
    if (nbytes > inodo.tamEnBytesLog)
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
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        return -1;
    }
    // Devolver la cantidad de bloques liberados.
    return bloquesL;
}