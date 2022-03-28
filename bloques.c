#include "bloques.h"

// Descriptor del fichero alamcenado como variable global estática,
// descpara que sólo pueda ser accedida en bloques.c
static int descriptor = 0;

// Función para montar el dispositivo virtual, y dado que se
// trata de un fichero, esa acción consistirá en abrirlo.
int bmount(const char *camino)
{
    if (descriptor > 0)
    {
        close(descriptor);
    }
    unmask(000);
    // Permisos rw-rw-rw (6-6-6)
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        fprintf(stderr, "(bmount(*c))Error %d: %s\n", errno, strerror(errno));
        // error (return -1)
        return EXIT_FAILURE;
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
        fprintf(stderr, "(bmount)Error %d: %s\n", errno, strerror(errno));
        // return -1
        return EXIT_FAILURE;
    }
    // return 0
    return EXIT_SUCCESS;
}

// Escribe 1 bloque en el dispositivo virtual,
// en el bloque físico especificado por nbloque.
int bwrite(unsigned int nbloque, const void *buf)
{
    // lseek: desplazamiento en el gestor de ficheros al bloque deseado
    // SEEK_SET = punto de referencia, desde el inicio del fichero
    off_t desp = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if (desp == -1)
    {
        fprintf(stderr, "(bwrite, 1)Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    else
    {
        //! Optimizable, de momento no optimizar para una mayor comprensión
        size_t bloquesW = write(descriptor, buf, BLOCKSIZE);
        // devuelve el nº de bytes que ha podido escribir
        //(si ha ido bien, será BLOCKSIZE), o -1 (o EXIT_FAILURE) si se produce un error.
        if (bloquesW == -1)
        {
            fprintf(stderr, "(bwrite, 2)Error %d: %s\n", errno, strerror(errno));
            // Se ha producido un error al escribir
            return EXIT_FAILURE;
        }
        else
        {
            // Se devuelven el num de bloques escritos
            return BLOCKSIZE;
        }
    }
}

// Lee 1 bloque del dispositivo virtual, que se corresponde
// con el bloque físico especificado por nbloque.
int bread(unsigned int nbloque, void *buf)
{
    // lseek: despplazamiento en el gestor de ficheros al bloque deseado
    // SEEK_SET = punto de referencia, desde el inicio del fichero
    off_t desp = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if (desp == -1)
    {
        fprintf(stderr, "(bread, 1)Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    else
    {
        //! Optimizable, de momento no optimizar para una mayor comprensión
        size_t bloqueL = read(descriptor, buf, BLOCKSIZE);
        if (bloqueL == -1)
        {
            fprintf(stderr, "(bread, 2)Error %d: %s\n", errno, strerror(errno));
            // Se ha producido un error al escribir
            return EXIT_FAILURE;
        }
        else
        {
            // Se devuelven el num de bloques leidos
            return BLOCKSIZE;
        }
    }
}