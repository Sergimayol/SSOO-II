/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

#define DEBUG 1 // Debugger del nivel 7

/* --------- Nivel 7 --------- */

//
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Camino debe empezar por '/'
    if (camino[0] != '/')
    {
        return -1;
    }
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
#if DEBUG
    // fprintf(stderr, "Camino: %s\nInicio: %s\nFinal: %s\nTipo: %s\n", camino, inicial, final, tipo);
#endif
    return 0;
}

// Esta función nos buscará una determinada entrada (la parte *inicial del *camino_parcial
// que nos devuelva extraer_camino()) entre las entradas del inodo correspondiente a su
// directorio padre (identificado con *p_inodo_dir).
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    // camino_parcial es “/”
    if (!strcmp(camino_parcial, "/"))
    {
        // lectura superbloque
        struct superbloque SB;
        if (bread(posSB, &SB) == -1)
        {
            return -1;
        }

        // nuestra raiz siempre estará asociada al inodo 0
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUG
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif
    // buscamos la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1)
    {
        return ERROR_PERMISO_LECTURA;
    }
    // array de tipo struct entrada de las entradas que caben en un bloque, para optimizar la lectura en RAM
    struct entrada buffer_lectura[BLOCKSIZE / sizeof(struct entrada)];
    memset(buffer_lectura, 0, (BLOCKSIZE / sizeof(struct entrada)) * sizeof(struct entrada));
    // cantidad de entradas que contiene el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    // nº de entrada inicial
    num_entrada_inodo = 0;
    if (cant_entradas_inodo > 0)
    {
        // Comprobar permisos
        if ((inodo_dir.permisos & 4) != 4)
        {
            return ERROR_PERMISO_LECTURA;
        }
        // leer entrada
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            return ERROR_PERMISO_LECTURA;
        }
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0))
        {
            num_entrada_inodo++;
            // leer siguiente entrada
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0)
            {
                return ERROR_PERMISO_LECTURA;
            }
        }
    }

    // inicial != entrada.nombre --> la entrada no existe
    if (num_entrada_inodo == cant_entradas_inodo &&
        (inicial != buffer_lectura[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre))
    {
        // seleccionar(reserva)
        switch (reservar)
        {
            // modo consulta. Como no existe retornamos error
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;
        // modo escritura
        case 1:
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            // si es directorio comprobar que tiene permiso de escritura
            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                // copiar *inicial en el nombre de la entrada
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        // reservar un nuevo inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo(tipo, permisos); // reservar_inodo('d', 6)
#if DEBUG
                        printf("[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                    }
                    else
                    {
                        // cuelgan más diretorios o ficheros
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    // es un fichero
                    // reservar un inodo como fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo(tipo, permisos); // reservar_inodo('f', 6)
#if DEBUG
                    printf("[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                }
#if DEBUG
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif

                // escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) == -1)
                {
                    // se había reservado un inodo para la entrada
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
#if DEBUG
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif
                    }
                    return -1;
                }
            }
        }
    }
    // Si hemos llegado al final del camino
    if (!strcmp(final, "/") || !strcmp(final, ""))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *(p_inodo) = num_entrada_inodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *(p_entrada) = entrada.ninodo;
        // cortamos la recursividad
        return 0;
    }
    else
    {
        // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        *(p_inodo_dir) = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}

//
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

/* --------- Nivel 8 --------- */

//
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

int mi_dir(const char *camino, char *buffer)
{
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    int numentradas;
    int nentrada;
    struct entrada entradas[BLOCKSIZE / tamEntrada];
    struct tm *tm;
    char tmp[100], tam[100], nom[100];
    p_inodo_dir = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        return -1;
    }
    if (inodo.tipo == 'f') // comprueba si es un fichero
    {
        strcat(buffer, "f\t");
        // permisos del inodo

        if (inodo.permisos & 4)
        {
            strcat(buffer, "r");
        }
        else
        {
            strcat(buffer, "-");
        }
        if (inodo.permisos & 2)
        {
            strcat(buffer, "w");
        }
        else
        {
            strcat(buffer, "-");
        }
        if (inodo.permisos & 1)
        {
            strcat(buffer, "x");
        }
        else
        {
            strcat(buffer, "-");
        }
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        sprintf(tam, "%d\t", inodo.tamEnBytesLog);
        if (mi_read_f(p_inodo_dir, entradas, p_entrada * tamEntrada, tamEntrada) == -1)
            return -1;
        sprintf(nom, "%s\n", entradas[0].nombre);
        strcat(buffer, nom);
        return 1;
    }
    if ((inodo.permisos & 2) != 2) // comprueba el permiso de lectura
        return -1;

    numentradas = inodo.tamEnBytesLog / tamEntrada;
    nentrada = 0;
    if (numentradas > 0)
    {
        // lee el primer bloque de entradas
        if (mi_read_f(p_inodo, entradas, 0, BLOCKSIZE) == -1)
            return -1;
        while (nentrada < numentradas)
        {
            if (leer_inodo(entradas[nentrada % (BLOCKSIZE / tamEntrada)].ninodo, &inodo) == -1)
                return -1;
            // tipo de inodo
            if (inodo.tipo == 'f')
            {
                strcat(buffer, "f\t");
            }
            else
            {
                strcat(buffer, "d\t");
            }
            // permisos del inodo
            if (inodo.permisos & 4) // lectura
            {
                strcat(buffer, "r");
            }
            else
            {
                strcat(buffer, "-");
            }
            if (inodo.permisos & 2) // escritura
            {
                strcat(buffer, "w");
            }
            else
            {
                strcat(buffer, "-");
            }
            if (inodo.permisos & 1) // ejecución
            {
                strcat(buffer, "x\t");
            }
            else
            {
                strcat(buffer, "-\t");
            }
            // mtime del inodo
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900,
                    tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            // tamaño del inodo
            sprintf(tam, "%d\t", inodo.tamEnBytesLog);
            strcat(buffer, tam);

            // nombre de la entrada
            if (inodo.tipo == 'f')
            {
                sprintf(nom, "%s", entradas[nentrada % (BLOCKSIZE / tamEntrada)].nombre);
            }
            else
            {
                sprintf(nom, "%s", entradas[nentrada % (BLOCKSIZE / tamEntrada)].nombre);
            }
            strcat(buffer, nom);
            switch (opcion)
            {
            case 0:
                strcat(buffer, "\t");
                break;

            case 1:
                strcat(buffer, "\n");
                break;
            }
            nentrada++;
            if (((nentrada) % (BLOCKSIZE / tamEntrada)) == 0)
            {
                // leer el siguiente bloque de entradas
                // fprintf(stderr, "ESTOY EN LA ENTRADA %d Y VOY A LEER EL SIGUIENTE BLOQUE DE ENTRADAS\n", nentrada);
                if (mi_read_f(p_inodo, entradas, nentrada * tamEntrada, BLOCKSIZE) == -1)
                    return -1;
            }
        }
        if (!opcion)
            strcat(buffer, "\n");
    }
    return numentradas;
}