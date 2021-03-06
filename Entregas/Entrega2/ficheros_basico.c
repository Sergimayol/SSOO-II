/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "ficheros_basico.h"

#define DEBUG3 0 // Debugger del nivel 3
#define DEBUG4 0 // Debugger del nivel 4
#define DEBUG6 0 // Debugger del nivel 6

// Calcula el tamaño en bloques necesario para el mapa de bits.
int tamMB(unsigned int nbloques)
{
    return (((nbloques / 8) % BLOCKSIZE) != 0) ? (((nbloques / 8) / BLOCKSIZE) + 1) : ((nbloques / 8) / BLOCKSIZE);
}

// Calcula el tamaño en bloques del array de inodos.
int tamAI(unsigned int ninodos)
{
    return (((ninodos * INODOSIZE) % BLOCKSIZE) != 0) ? (((ninodos * INODOSIZE) / BLOCKSIZE) + 1) : ((ninodos * INODOSIZE) / BLOCKSIZE);
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
    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }
    return 0;
}

// Inicializa el mapa de bits.
int initMB()
{
    struct superbloque SB;
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    // Lectura del superbloque para obtener localización mapa de bits.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // Inicializacion mapa bits
    for (int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++)
    {
        if (bwrite(i, buffer) == -1)
        {
            return -1;
        }
    }
    // Ponemos a 1, en el mapa de bits, los bits que corresponden a los metadatos
    for (int i = posSB; i < SB.posPrimerBloqueDatos; i++)
    {
        // reservar bloques metadatos
        reservar_bloque();
    }
    return 0;
}

// Esta función inicializa la lista de inodos libres.
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
    int end = 0;
    // Para cada bloque del AI
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && end == 0; i++)
    {
        // Para cada inodo del AI
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
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
                end = 1;
                break;
            }
        }
        // Escribir el bloque de inodos i en el dispositivo virtual
        if (bwrite(i, &inodos) == -1)
        {
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
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

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
    // Escribir buffer
    if (bwrite(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    return 0;
}

// Lee un determinado bit del MB y devuelve el valor del bit leído.
char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Lectura bloque contenedor del bit
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        return -1;
    }
    // Localizacion del byte dentro del bloqu
    posbyte = posbyte % BLOCKSIZE;
    // 10000000
    unsigned char mascara = 128;
    // desplazamiento de bits a la derecha
    mascara >>= posbit;
    // operador AND para bits
    mascara &= bufferMB[posbyte];
    // desplazamiento de bits a la derecha
    mascara >>= (7 - posbit);

#if DEBUG3
    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif

    return mascara;
}

// Encuentra el primer bloque libre, consultando el MB (primer bit a 0),
// lo ocupa (poniendo el correspondiente bit a 1) y devuelve su posición.
int reservar_bloque()
{
    unsigned char bufferMB[BLOCKSIZE], bufferAux[BLOCKSIZE];
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
    unsigned int nbloqueabs = SB.posPrimerBloqueMB;
    int libre = 0;
    while (libre == 0)
    {
        if (bread(nbloqueabs, bufferMB) == -1)
        {
            return -1;
        }
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0)
        {
            libre = 1;
            break;
        }
        nbloqueabs++;
    }
    unsigned int posbyte = 0;
    while (bufferMB[posbyte] == 255)
    {
        posbyte++;
    }
    unsigned char mascara = 128; // 10000000
    unsigned int posbit = 0;
    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara) // operador AND para bits
    {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }
    unsigned int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    if (escribir_bit(nbloque, 1) == -1)
    {
        return -1;
    }
    SB.cantBloquesLibres--;
    memset(bufferAux, 0, BLOCKSIZE);

    if (bwrite(SB.posPrimerBloqueDatos + nbloque - 1, bufferAux) == -1)
    {
        return -1;
    }

    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }
    return nbloque;
}

// Libera un bloque determinado
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    // Lectura del superbloque para obtener localización array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    if (escribir_bit(nbloque, 0))
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
    if (SB.cantInodosLibres == 0)
    {
        return -1;
    }
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    SB.cantInodosLibres--;
    SB.posPrimerInodoLibre++;
    // Inicializacion inodo
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
    unsigned int ptr;
    int ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga del inodo

    while (nivel_punteros > 0)
    {
        // iterar para cada nivel de indirectos
        if (ptr == 0)
        {
            // no cuelgan bloques de punteros
            if (reservar == 0)
            {
                // bloque no existe
                return -1;
            }
            else
            {
                // reservar bloques de punteros y crear enlaces desde el inodo hasta el bloque de datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                if (nivel_punteros == nRangoBL)
                {
                    // el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUG4
                    printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nRangoBL - 1, ptr, ptr, nivel_punteros);
#endif
                }
                else
                {
                    // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
#if DEBUG4
                    printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                           nivel_punteros, indice, ptr, ptr, nivel_punteros);
#endif
                    // salvamos en el dispositivo el buffer de punteros modificado
                    if (bwrite(ptr_ant, buffer) == -1)
                    {
                        return -1;
                    }
                }
            }
        }
        // leemos del dispositivo el bloque de punteros ya existente
        if (bread(ptr, buffer) == -1)
        {
            return -1;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    }

    if (ptr == 0)
    {
        // no existe bloque de datos
        if (reservar == 0)
        {
            // bloque inexistente
            return -1;
        }
        else
        {
            // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
            salvar_inodo = 1;
            ptr = reservar_bloque(); // de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL); // fecha actual
            if (nRangoBL == 0)
            {
                inodo.punterosDirectos[nblogico] = ptr;
// Caso n1: nblogico = 8, ptr = 3139
#if DEBUG4
                printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n",
                       nblogico, ptr, ptr, nblogico);
#endif
            }
            else
            {
                buffer[indice] = ptr;
#if DEBUG4
                printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n",
                       indice, ptr, ptr, nblogico);
#endif
                if (bwrite(ptr_ant, buffer) == -1)
                {
                    return -1;
                }
            }
        }
    }

    if (salvar_inodo == 1)
    {
        // sólo si lo hemos actualizado
        if (escribir_inodo(ninodo, inodo) == EXIT_FAILURE)
        {
            return -1;
        }
    }
    // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
    return ptr;
}

// Libera un inodo
int liberar_inodo(unsigned int ninodo)
{
    struct superbloque SB;
    struct inodo inodo;

    // Lectura inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }

    // liberar todos los bloques del inodo y restar cantidad bloques liberados
    inodo.numBloquesOcupados -= liberar_bloques_inodo(0, &inodo);
    // Comprobamos si se han liberado
    if (inodo.numBloquesOcupados != 0)
    {
        return -1;
    }
    // Marcar el inodo como tipo libre y tamEnBytesLog = 0
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    // Lectura del superbloque
    if (bread(posSB, &SB) == -1)
    {
        return -1;
    }
    // primer inodo libre de la lista enlazada
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    // Incluir inodo que queremos liberar en la lista de inodos libres
    SB.posPrimerInodoLibre = ninodo;
    // incrementar la cantidad de inodos libres
    SB.cantInodosLibres++;

    // escribir inodo
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        return -1;
    }

    // escribir superbloque
    if (bwrite(posSB, &SB) == -1)
    {
        return -1;
    }
    // devolver el num del inodo liberado
    return ninodo;
}

// libera todos los bloques ocupados a partir del bloque lógico
// indicado por el argumento (inclusive).
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int nivel_punteros, indice, ptr = 0, nBL, ultimoBL;
    // nº de bloques liberados
    int nRangoBL, liberados = 0;
    // array de bloques de punteros
    unsigned int bloques_punteros[3][NPUNTEROS];
    // para llenar de 0s y comparar
    unsigned char bufAux_punteros[BLOCKSIZE];
    // punteros a bloques de punteros de cada nivel e indices de cada nivel
    int ptr_nivel[3], indices[3];
    if ((inodo->tamEnBytesLog) == 0)
    {
        // el fichero está vacío
        return 0;
    }
    // obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {
        ultimoBL = ((inodo->tamEnBytesLog) / BLOCKSIZE) - 1;
    }
    else
    {
        ultimoBL = (inodo->tamEnBytesLog) / BLOCKSIZE;
    }
    memset(bufAux_punteros, 0, BLOCKSIZE);

#if DEBUG6
    printf("[liberar_bloques_inodo()-> primerBL: %d, ultimoBL: %d]\n", primerBL, ultimoBL);
#endif
    // recorrido BLs
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {
        // 0:D, 1:I0, 2:I1, 3:I2
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0)
        {
            return -1;
        }
        // el nivel_punteros +alto cuelga del inodo
        nivel_punteros = nRangoBL;
        while ((ptr > 0) && (nivel_punteros > 0))
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if ((indice == 0) || (nBL == primerBL))
            {
                // solo hay que leer del dispositivo si no está ya cargado previamente en un buffer
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) == -1)
                {
                    return -1;
                }
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        // si existe bloque de datos
        if (ptr > 0)
        {
            liberar_bloque(ptr);
            liberados++;
#if DEBUG6
            printf("[liberar_bloques_inodo()-> liberado BF %d de datos par a BL %d]\n", ptr, nBL);
#endif
            // es un puntero Directo
            if (nRangoBL == 0)
            {
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
#if DEBUG6
                        printf("[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n", ptr, nivel_punteros, nBL);
#endif
                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else
                    {
                        // escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == -1)
                        {
                            return -1;
                        }
                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }
#if DEBUG6
    printf("[liberar_bloques_inodo()-> total bloques liberados: %d]\n", liberados);
#endif
    // return numero bloques liberados
    return liberados;
}