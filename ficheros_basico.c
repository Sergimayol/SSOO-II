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
    return 0;
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
        return -1;
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
            return -1;
        }
    }
    return 0;
}

/* --------- Nivel 3 --------- */

// Esta función escribe el valor indicado por el parámetro
// bit: 0 (libre) ó 1 (ocupado) en un determinado bit del MB
// que representa el bloque nbloque.
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    unsigned char bufferMB[BLOCKSIZE];
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    posbyte = nbloque / 8;
    posbit = nbloque % 8;
    nbloqueMB = posbyte / BLOCKSIZE;
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    posbyte = posbyte % BLOCKSIZE;
    // 10000000
    unsigned char mascara = 128;
    // desplazamiento de bits a la derecha
    mascara >>= posbit;
    if (bit == 0)
    {
        // operadores AND y NOT para bits
        bufferMB[posbyte] &= ~mascara;
    }
    else if (bit == 1)
    {
        //  operador OR para bits
        bufferMB[posbyte] |= mascara;
    }
    else
    {
        return -1;
    }
    if (bwrite(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    return 0;
}

// Lee un determinado bit del MB y devuelve el valor del bit leído.
char leer_bit(unsigned int nbloque)
{
    unsigned char bufferMB[BLOCKSIZE];
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    posbyte = nbloque / 8;
    posbit = nbloque % 8;
    nbloqueMB = posbyte / BLOCKSIZE;
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    posbyte = posbyte % BLOCKSIZE;
    // 10000000
    unsigned char mascara = 128;
    // desplazamiento de bits a la derecha
    mascara >>= posbit;
    // operador AND para bits
    mascara &= bufferMB[posbyte];
    // desplazamiento de bits a la derecha
    mascara >>= (7 - posbit);
    return mascara;
}

// Encuentra el primer bloque libre, consultando el MB (primer bit a 0),
// lo ocupa (poniendo el correspondiente bit a 1) y devuelve su posición.
int reservar_bloque()
{
    int nbloqueabs, posbyte, nbloque, posbit;
    unsigned char bufferMB[BLOCKSIZE], bufferAux[BLOCKSIZE], mascara, bufferByte;
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Quedan nodos libres?
    if (SB.cantBloquesLibres == 0)
    {
        return -1;
    }
    memset(bufferAux, 255, BLOCKSIZE);
    nbloqueabs = SB.posPrimerBloqueMB;
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    while (nbloqueabs <= SB.posUltimoBloqueMB)
    {
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0)
        {
            break;
        }
        nbloqueabs++;
        if (bread(nbloqueabs, bufferMB) == -1)
        {
            return -1;
        }
    }
    if (memcmp(bufferMB, bufferAux, BLOCKSIZE) == 0)
    {
        return -1;
    }
    posbyte = 0;
    bufferByte = 255;
    while (posbyte < BLOCKSIZE)
    {
        if (memcmp(&bufferMB[posbyte], &bufferByte, 1) != 0)
        {
            break;
        }
        posbyte++;
    }
    if (posbyte == BLOCKSIZE)
    {
        return -1;
    }
    mascara = 128; // 10000000
    posbit = 0;
    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara) // operador AND para bits
    {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }
    nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    if (escribir_bit(nbloque, 1) == -1)
    {
        return -1;
    }
    SB.cantBloquesLibres--;
    memset(bufferAux, 0, BLOCKSIZE);
    if (bwrite(nbloque, bufferAux) == -1)
    {
        return -1;
    }
    return nbloque;
}

// Libera un bloque determinado
int liberar_bloque(unsigned int nbloque)
{
    if (escribir_bit(nbloque, 0))
    {
        return -1;
    }
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }
    return nbloque;
}

// Escribe el contenido de una variable de tipo struct inodo
// en un determinado inodo del array de inodos, inodos.
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    unsigned int nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;
    if (bread(nbloqueabs, inodos) == -1)
    {
        return -1;
    }
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo;
    if (bwrite(nbloqueabs, inodos) == -1)
    {
        return -1;
    }
    return 0;
}

// Lee un determinado inodo del array de inodos para volcarlo
// en una variable de tipo struct inodo pasada por referencia.
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    unsigned int nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;
    if (bread(nbloqueabs, inodos) == -1)
    {
        return -1;
    }
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return 0;
}

// Encuentra el primer inodo libre (dato almacenado en el superbloque), lo reserva,
// devuelve su número y actualiza la lista enlazada de inodos libres.
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodoAux;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    if (SB.cantInodosLibres < 1)
    {
        return -1;
    }
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    if (lee_inodo(posInodoReservado, &inodoAux) == -1)
    {
        return -1;
    }

    SB.posPrimerInodoLibre++;
    SB.posPrimerInodoLibre = inodoAux.punterosDirectos[0];
    inodoAux.tipo = tipo;
    inodoAux.permisos = permisos;
    inodoAux.nlinks = 1;
    inodoAux.tamEnBytesLog = 0;
    inodoAux.atime = time(NULL);
    inodoAux.mtime = time(NULL);
    inodoAux.ctime = time(NULL);
    inodoAux.numBloquesOcupados = 0;
    memset(inodoAux.punterosDirectos, 0, 12 * sizeof(unsigned int));
    memset(inodoAux.punterosIndirectos, 0, 3 * sizeof(unsigned int));
    if (escribir_inodo(posInodoReservado, inodoAux) == -1)
    {
        return -1;
    }
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }
    return posInodoReservado;
}

/* --------- Nivel 4 --------- */

// Funcion auxiliar que asocia un nivel a cada rango de bloques lógicos, devolviendo
// el rango del bloque lógico indicado, siendo el rango 0 para bloques lógicos [0 , 11],
// 1 para [12 , 267], 2 para [268 , 65.803] y 3 para [65.804 , 16.843.019].
// También actualizar una variable puntero, ptr, pasada por referencia, para que apunte
// donde lo hace el puntero correspondiente del inodo.
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        printf("Bloque lógico fuera de rango");
        return -1;
    }
}

// Obtención de los índices de los bloques de punteros.
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return -1;
}

// Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
// Enmascara la gestión de los diferentes rangos de punteros directos e indirectos del inodo, de manera
// que funciones externas no tienen que preocuparse de cómo acceder a los bloques físicos apuntados desde el inodo.
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{
    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    leer_inodo(ninodo, &inodo);
    ptr = 0, ptr_ant = 0, salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    {
        // no cuelgan bloques de punteros
        if (ptr == 0)
        {
            if (reservar == 0)
            {
                // Si no existe bloque físico de datos, dará error.
                return -1;
            }
            else
            {
                // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
            
            }
        }
    }
}