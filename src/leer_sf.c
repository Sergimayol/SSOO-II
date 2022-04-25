#include "ficheros_basico.h"

struct superbloque SB;
struct inodo inodos[BLOCKSIZE / INODOSIZE];

int printSuperBloque();
int printListaEnlazada();
int printMapaBits();
int printTestBloque();
int printInodo();

int main(int argc, char **argsv)
{
    if (argc == 2)
    {
        if (bmount(argsv[1]) == -1)
        {
            fprintf(stderr, "Error montando dispositivo virtual.\n");
            return -1;
        }
        printSuperBloque();
        // printf("sizeof struct superbloque: %li\n", sizeof(struct superbloque));
        // printf("sizeof struct inodo: %li\n\n", sizeof(struct inodo));
        //  printListaEnlazada();
        //  printMapaBits();
        //  printTestBloque();
        //  printInodo();
        if (bumount() == -1)
        {
            fprintf(stderr, "Error desmontando dispositivo virtual.\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Error sintaxis: ./leer_sf <nombre_dispositivo>\n");
        return -1;
    }
    return 0;
}

int printSuperBloque()
{
    if (bread(0, &SB) == -1)
    {
        return -1;
    }
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB: %i\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB: %i\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI: %i\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %i\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos: %i\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos: %i\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaíz: %i\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %i\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    printf("cantInodosLibres: %i\n", SB.cantInodosLibres);
    printf("totBloques: %i\n", SB.totBloques);
    printf("totInodos: %i\n\n", SB.totInodos);
    return 0;
}

int printListaEnlazada()
{
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES:\n");
    int indexInodos = SB.posPrimerInodoLibre + 1; // Cantidad de Inodos
    int lastInodo = 0;
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && (lastInodo == 0); i++)
    { // Recorrido de todos los inodos
        if (bread(i, inodos) == -1)
        {
            fprintf(stderr, "Error while reading");
            return -1;
        }
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        { // Recorrido de cada uno de los inodos
            if (indexInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = indexInodos; // Declaramos la conexion con el siguiente inodo
                indexInodos++;
                printf("%d ", inodos[j].punterosDirectos[0]);
            }
            else
            { // Estamos al final
                inodos[j].punterosDirectos[0] = UINT_MAX;
                lastInodo = 1;
                printf("%d ", inodos[j].punterosDirectos[0]);
                break;
            }
        }
    }
    printf("\n");
    return 0;
}

int printMapaBits()
{
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int bit = leer_bit(posSB);
    printf("leer_bit(%i) = %i\n", posSB, bit);
    bit = leer_bit(SB.posPrimerBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, bit);
    bit = leer_bit(SB.posUltimoBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, bit);
    bit = leer_bit(SB.posPrimerBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, bit);
    bit = leer_bit(SB.posUltimoBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, bit);
    bit = leer_bit(SB.posPrimerBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, bit);
    bit = leer_bit(SB.posUltimoBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, bit);
    return 0;
}

int printTestBloque()
{
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int reservado = reservar_bloque(); // Actualiza el SB
    bread(posSB, &SB);                 // Actualizar los valores del SB
    printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", reservado);
    printf("SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    printf("liberar el bloque [ %d ]: %d \n", reservado, liberar_bloque(reservado));
    bread(posSB, &SB); // Actualizar los valores del SB
    printf("Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\n", SB.cantBloquesLibres);
    return 0;
}

int printInodo()
{
    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    struct inodo inodo;
    int ninodo = SB.posInodoRaiz; // el directorio raiz es el inodo 0
    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n", ninodo, atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    printf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);
    return 0;
}