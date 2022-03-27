#include "ficheros_basico.h"

struct superbloque SB;

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
}

// Inicializa los datos del superbloque.
int initSB(unsigned int nbloques, unsigned int ninodos)
{
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
    unsigned char buffer[BLOCKSIZE];
    if (memset(buffer, 0, sizeof(buffer)) == NULL)
    {
        // error
        fprintf(stderr, "Error asignando un valor al array.");
        return -1;
    }
    for (size_t i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++)
    {
        if (bwrite(i, buffer) == -1)
        {
            // Error
            fprintf(stderr, "Error escribiendo bloque(%d).", i);
            return -1;
        }
    }
    //! Creo que no finalizado
    //? Reservar bloques¿?
}

// Esta función se encargará de inicializar la lista de inodos libres.
int initAI()
{
}