#include "ficheros_basico.h"

// Calcula el tamaño en bloques necesario para el mapa de bits.
int tamMB(unsigned int nbloques)
{
    if ((nbloques / 8) % BLOCKSIZE != 0)
    {
        return ((nbloques / 8) / BLOCKSIZE) + 1;
    }
    else
    {
        return ((nbloques / 8) / BLOCKSIZE);
    }
    // return (((nbloques / 8) % BLOCKSIZE) != 0) ? (((nbloques / 8) / BLOCKSIZE) + 1) : ((nbloques / 8) / BLOCKSIZE);
}

// Calcula el tamaño en bloques del array de inodos.
int tamAI(unsigned int ninodos)
{
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0)
    {
        return ((ninodos * INODOSIZE) / BLOCKSIZE) + 1;
    }
    else
    {
        return ((ninodos * INODOSIZE) / BLOCKSIZE);
    }
    // return (((ninodos * INODOSIZE) % BLOCKSIZE) != 0) ? (((ninodos * INODOSIZE) / BLOCKSIZE) + 1) : ((ninodos * INODOSIZE) / BLOCKSIZE);
}

// Inicializa los datos del superbloque.
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    // Posición del primer bloque del mapa de bits
    SB.posPrimerBloqueMB = posSB + tamSB;
    // Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    // Posición del primer bloque del array de inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    // Posición del último bloque del array de inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    // Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    // Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    // Posición del inodo del directorio raíz en el array de inodos
    SB.posInodoRaiz = 0;
    // Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    // Cantidad de bloques libres en el SF
    SB.cantBloquesLibres = nbloques;
    // Cantidad de inodos libres en el array de inodos
    SB.cantInodosLibres = ninodos;
    // Cantidad total de bloques
    SB.totBloques = nbloques;
    // Cantidad total de inodos
    SB.totInodos = ninodos;
    // Escritura estructura bloque
    return bwrite(posSB, &SB);
}

// Inicializa el mapa de bits. En este nivel,de momento,
// simplemente pondremos a 0 todos los bits del mapa de bits.
int initMB()
{
    struct superbloque SB;
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    // Lectura del superbloque para obtener localización mapa de bits.
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, "(initMB)Error %d: %s\n", errno, strerror(errno));
        fprintf(stderr, "Error leyendo superbloque.\n");
        return -1;
    }
    for (size_t i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++)
    {
        if (bwrite(i, buffer) == -1)
        {
            // Error
            fprintf(stderr, "(initMB)Error %d: %s\n", errno, strerror(errno));
            fprintf(stderr, "Error escribiendo bloque (%zu).\n", i);
            return -1;
        }
    }
    return EXIT_SUCCESS;
}

// Esta función se encargará de inicializar la lista de inodos libres.
int initAI()
{
    // buffer para ir recorriendo el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;
    unsigned int contInodos;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return EXIT_FAILURE;
    }
    //+1 porque hemos iniciado SB.posPrimerInodoLibre = 0
    contInodos = SB.posPrimerInodoLibre + 1;
    // Para cada bloque del AI
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Para cada inodo del AI
        for (size_t j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {
            inodos[j].tipo = 'l'; // libre
            // Si no hemos llegado al último inodo
            if (contInodos < SB.totInodos)
            {
                // Enlazamos con el siguiente
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            {
                // Hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        // Escribir el bloque de inodos i  en el dispositivo virtual
        if (bwrite(i, inodos) == -1)
        {
            // Error
            fprintf(stderr, "(initMB)Error %d: %s\n", errno, strerror(errno));
            fprintf(stderr, "Error escribiendo bloque (%zu).\n", i);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}