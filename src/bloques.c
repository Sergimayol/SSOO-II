/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau  
 - Carlos Galardon Burges
*/

#include "bloques.h"

// Descriptor del fichero alamcenado como variable global estática,
// descpara que sólo pueda ser accedida en bloques.c
static int descriptor = 0;

// Función para montar el dispositivo virtual, y dado que se
// trata de un fichero, esa acción consistirá en abrirlo.
int bmount(const char *camino)
{
    umask(000);
    // Permisos rw-rw-rw (6-6-6)
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        return -1;
    }
    return descriptor;
}

// Desmonta el dispositivo virtual. Devuelve 0 (o EXIT_SUCCESS)
// si se ha cerrado el fichero correctamente, o -1 (o EXIT_FAILURE)
// en caso contrario.
int bumount()
{
    descriptor = close(descriptor);
    if (descriptor == -1)
    {
        return -1;
    }
    return 0;
}

// Escribe 1 bloque en el dispositivo virtual,
// en el bloque físico especificado por nbloque.
int bwrite(unsigned int nbloque, const void *buf)
{
    // lseek: desplazamiento en el gestor de ficheros al bloque deseado
    // SEEK_SET = punto de referencia, desde el inicio del fichero
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1)
    {
        fprintf(stderr, "(bwrite, 1) --> Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    else
    {
        int bloquesW = write(descriptor, buf, BLOCKSIZE);
        // devuelve el nº de bytes que ha podido escribir
        //(si ha ido bien, será BLOCKSIZE), o -1 (o EXIT_FAILURE) si se produce un error.
        if (bloquesW == -1)
        {
            fprintf(stderr, "(bwrite, 2) --> Error %d: %s\n", errno, strerror(errno));
            // Se ha producido un error al escribir
            return -1;
        }
        // Se devuelven el num de bloques escritos
        return bloquesW;
    }
}

// Lee 1 bloque del dispositivo virtual, que se corresponde
// con el bloque físico especificado por nbloque.
int bread(unsigned int nbloque, void *buf)
{
    // lseek: despplazamiento en el gestor de ficheros al bloque deseado
    // SEEK_SET = punto de referencia, desde el inicio del fichero
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1)
    {
        fprintf(stderr, "(bread, 1)Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    else
    {
        int bloqueL = read(descriptor, buf, BLOCKSIZE);
        if (bloqueL == -1)
        {
            fprintf(stderr, "(bread, 2)Error %d: %s\n", errno, strerror(errno));
            // Se ha producido un error al leer
            return -1;
        }
        // Se devuelven el num de bloques leidos
        return bloqueL;
    }
}