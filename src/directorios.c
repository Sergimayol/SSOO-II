/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"
#include <string.h>

#define DEBUG7 1 // Debugger del nivel 7

//
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Camino debe empezar por '/'
    if (camino[0] != '/')
    {
        return -1;
    }
    const char *cam = (camino + 1);
    // Obtenemos parte final del camino
    const char *res = strchr((camino + 1), '/');
    // Comprobar si se trata de solo un fichero sin directorio
    if (res)
    {
        // Copiamos todo en inicial menos el resto
        strncpy(inicial, (camino + 1), (strlen(camino) - strlen(res) - 1));
        // Copiamos el resto en final
        strcpy(final, res);
        // Tipo: directorio
        strcpy(tipo, "d");
    }
    else // Si no tiene parte final, entonces se trata de unicamente un fichero
    {
        // Inicial: camino
        strcpy(inicial, (camino + 1));
        // Tipo: fichero
        strcpy(tipo, "f");
        strcpy(final, "");
    }
#if DEBUG7
    fprintf(stderr, "\tCamino: %s\n\tInicio: %s\n\tFinal: %s\n\tTipo: %s\n", camino, inicial, final, tipo);
#endif
    return 0;
}

// Esta función nos buscará una determinada entrada (la parte *inicial del *camino_parcial
// que nos devuelva extraer_camino()) entre las entradas del inodo correspondiente a su
// directorio padre (identificado con *p_inodo_dir).
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    char inicial[70]; // preguntar
    memset(inicial, 0, sizeof(inicial));
    char final[1024];
    memset(final, 0, sizeof(final));
    char tipo[128];
    memset(tipo, 0, sizeof(tipo));
    struct inodo inodo_dir;
    int numentradas;
    int nentrada;
    struct entrada entradas[BLOCKSIZE / tamEntrada];
    if (!strcmp(camino_parcial, "/"))
    {
        *p_inodo = 0;
        *p_entrada = 0;
        return 0;
    }
    if (extraer_camino(camino_parcial, inicial, final, tipo) == -1)
        return ERROR_EXTRAER_CAMINO;
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1)
        return EXIT_FAILURE;

    if ((inodo_dir.permisos & 4) != 4)
    {
#if DEBUG7
        fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
#endif
        return ERROR_PERMISO_LECTURA;
    }
    numentradas = inodo_dir.tamEnBytesLog / tamEntrada;
    nentrada = 0;
    memset(entradas, 0, BLOCKSIZE);
    if (numentradas > 0)
    {
        // leer el primer bloque de entradas y mirarlas desde el buffer hasta acabar de leer el buffer
        if (mi_read_f(*p_inodo_dir, entradas, 0, BLOCKSIZE) == -1)
            return -1;
        while ((nentrada < numentradas) && strcmp(inicial, entradas[nentrada % (BLOCKSIZE / tamEntrada)].nombre))
        {
            nentrada++;
            if ((nentrada % (BLOCKSIZE / tamEntrada)) == 0 && nentrada < numentradas) // múltiplo del número de entradas en un bloque
            {
                // leer el siguiente bloque de entradas
                if (mi_read_f(*p_inodo_dir, entradas, nentrada * tamEntrada, BLOCKSIZE) == -1)
                    return -1;
            }
        }
    }
    if (nentrada == numentradas)
    {
        switch (reservar)
        {
        case 0: // modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: // modo escritura
            if (inodo_dir.tipo == 'f')
            {
#if DEBUG7
                fprintf(stderr, "[buscar_entrada()→ No se puede crear entrada en un fichero]\n");
#endif
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2)
            {
#if DEBUG7
                fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de escritura]\n", *p_inodo_dir);
#endif
                return ERROR_PERMISO_ESCRITURA;
            }
            strcpy(entradas[nentrada % (BLOCKSIZE / tamEntrada)].nombre, inicial);
            if (!strcmp(tipo, "DIRECTORIO")) // es un directorio
            {
                if (!strcmp(final, "/"))
                {
                    entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo = reservar_inodo('d', permisos);
#if DEBUG7
                    fprintf(stderr, "[buscar_entrada()→ entrada.nombre: %s, entrada.ninodo: %d]\n", entradas[nentrada].nombre, entradas[nentrada].ninodo);
                    fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d]\n", entradas[nentrada].ninodo, permisos);
#endif
                }
                else
                {
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                }
            }
            else // es un fichero
            {
                entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo = reservar_inodo('f', permisos);
#if DEBUG7
                fprintf(stderr, "[buscar_entrada()→ entrada.nombre: %s, entrada.ninodo: %d]\n", entradas[nentrada].nombre, entradas[nentrada].ninodo);
                fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d]\n", entradas[nentrada].ninodo, permisos);
#endif
            }
            if (mi_write_f(*p_inodo_dir, &entradas[nentrada % (BLOCKSIZE / tamEntrada)], nentrada * tamEntrada, tamEntrada) == -1)
            {
                if (entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo != -1)
                {
                    liberar_inodo(entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo);
                }
                return -1;
            }
        }
    }
    if (!strcmp(final, "") || !strcmp(final, "/"))
    {
        if ((nentrada < numentradas) && (reservar == 1))
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo;
        *p_entrada = nentrada;
        return 0;
    }
    else
    {
        *p_inodo_dir = entradas[nentrada % 16].ninodo;
        return (buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos));
    }
}

/* -Función: mostrar_error_buscar_entrada.
** -Descripción: Dado un resultado de error de la operaciónd de buscar_entrada,
** se imprimer un texto de error determinado.
** -Parámetros: número de error.
** -Return:
*/
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error\n");
        break;
    case EXIT_FAILURE:
        fprintf(stderr, "Error\n");
        break;
    case ERROR_EXTRAER_CAMINO:
        fprintf(stderr, "Error: Camino incorrecto\n");
        break;
    case ERROR_PERMISO_LECTURA:
        fprintf(stderr, "Error: Permiso denegado de lectura\n");
        break;
    case ERROR_NO_EXISTE_ENTRADA_CONSULTA:
        fprintf(stderr, "Error: No existe la entrada\n");
        break;
    case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO:
        fprintf(stderr, "Error: No se puede crear una entrada en un fichero\n");
        break;
    case ERROR_PERMISO_ESCRITURA:
        fprintf(stderr, "Error: Permiso denegado de escritura\n");
        break;
    case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO:
        fprintf(stderr, "Error: No existe directorio intermedio\n");
        break;
    case ERROR_ENTRADA_YA_EXISTENTE:
        fprintf(stderr, "Error: La entrada ya existe\n");
        break;
    }
}

int mi_creat(const char *camino, unsigned char permisos)
{
    int error;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    p_inodo_dir = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) != EXIT_SUCCESS)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    return 0;
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    p_inodo_dir = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
        return -1;
    return mi_chmod_f(p_inodo, permisos);
}

int mi_stat(const char *camino, struct STAT *stat)
{
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    p_inodo_dir = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
        return -1;
    return mi_stat_f(p_inodo, stat);
}