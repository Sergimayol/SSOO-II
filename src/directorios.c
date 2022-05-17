/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

#define DEBUG 1  // Debugger del nivel 7
#define DEBUG8 1 // Debugger del nivel 8
#define DEBUG9 1 // Debugger del nivel 9

static struct UltimaEntrada UltimaEntradaEscritura[CACHE];
int MAXCACHE = CACHE;

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
    if (leer_inodo(*(p_inodo_dir), &inodo_dir) == -1)
    {
        return -1;
    }
    // Comprobamos que el inodo tenga permiso de lectura.
    if ((inodo_dir.permisos & 4) != 4)
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

    /* if (cant_entradas_inodo > 0)
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
     }*/
    int res = 0;
    if (cant_entradas_inodo > 0)
    {
        res += mi_read_f(*p_inodo_dir, &buffer_lectura, res, BLOCKSIZE);
        while ((num_entrada_inodo < cant_entradas_inodo) &&
               (strcmp(inicial, buffer_lectura[num_entrada_inodo].nombre) != 0))
        {
            num_entrada_inodo++;
            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                res += mi_read_f(*p_inodo_dir, &buffer_lectura, res, BLOCKSIZE);
            }
        }
    }

    // inicial != entrada.nombre --> la entrada no existe
    /*if (num_entrada_inodo == cant_entradas_inodo &&
        (inicial != buffer_lectura[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre))*/
    if (strcmp(buffer_lectura[num_entrada_inodo].nombre, inicial) != 0)
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
                        entrada.ninodo = reservar_inodo('d', 6); // reservar_inodo(tipo, permisos)
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
                    entrada.ninodo = reservar_inodo('f', 6); // reservar_inodo(tipo, permisos)
#if DEBUG
                    printf("[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                }
#if DEBUG
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif
                // escribir la entrada en el directorio padre
                // if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) == -1)
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
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
        *(p_inodo) = buffer_lectura[num_entrada_inodo].ninodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *(p_entrada) = num_entrada_inodo;
        // cortamos la recursividad
        return 0;
    }
    else
    {
        // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        //*(p_inodo_dir) = entrada.ninodo;
        *p_inodo_dir = buffer_lectura[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
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

// Función de la capa de directorios que crea un fichero/directorio
// y su entrada de directorio.
int mi_creat(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    // Obtenemos el valor de buscar_entrada
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    // Si el valor es menor que 0
    if (error < 0)
    {
        // Imprimir error
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    return 0;
}

// Funcion para cambiar los permisos de un fichero o directorio
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    // Obtenemos el valor de buscar_entrada
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    // Si el valor es menor que 0
    if (error < 0)
    {
        // Imprimir error
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (mi_chmod_f(p_inodo, permisos) == -1)
    {
        return -1;
    }
    return 0;
}

// Funcion para obtiener la metainformacion del elemento del camino.
int mi_stat(const char *camino, struct STAT *stat)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    // Obtenemos el valor de buscar_entrada
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    // Si el valor es menor que 0
    if (error < 0)
    {
        // Imprimir error
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (mi_stat_f(p_inodo, stat) == -1)
    {
        return -1;
    }
    return 0;
}

//
int mi_dir(const char *camino, char *buffer, char *tipo)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    int nEntradas = 0;
    struct inodo inodo;
    struct tm *tm;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); // Permisos para leer
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        return -1;
    }
    if ((inodo.permisos & 4) != 4)
    {
        return -1;
    }

    // Para el tiempo
    char tmp[100];
    // 10 = valor maximo de un unsigned int
    char tamEnBytes[10];
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        return -1;
    }
    *tipo = inodo.tipo;
    // Buffer de salida
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(&entradas, 0, sizeof(struct entrada));
    nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    int offset = mi_read_f(p_inodo, entradas, 0, BLOCKSIZE);
    // Leemos todos las entradas
    for (int i = 0; i < nEntradas; i++)
    {
        // Leer el inodo correspndiente
        if (leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == -1)
        {
            return -1;
        }

        // Tipo
        if (inodo.tipo == 'd')
        {
            strcat(buffer, MAGENTA);
            strcat(buffer, "d");
        }
        else
        {
            strcat(buffer, CYAN);
            strcat(buffer, "f");
        }
        strcat(buffer, "\t");

        // Permisos
        strcat(buffer, AZUL);
        strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
        strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
        strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
        strcat(buffer, "\t");

        // mTime
        strcat(buffer, AMARILLO);
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // Tamaño
        strcat(buffer, AZUL_F);
        sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytes);
        strcat(buffer, "\t");

        // Nombre
        strcat(buffer, ROJO_F);
        strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
        while ((strlen(buffer) % TAMFILA) != 0)
        {
            strcat(buffer, " ");
        }

        strcat(buffer, RESET_FORMATO);
        strcat(buffer, "\n"); // Preparamos el string para la siguiente entrada

        if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0)
        {
            offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
        }
    }

    return nEntradas;
}

/* --------- Nivel 9 --------- */

// Función de directorios.c para escribir contenido en un fichero.
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo = 0, p_inodo_dir = 0, p_entrada = 0;
    unsigned int enc = 0;

    // comprobar si ya se ha leido anteriormente
    for (int i = 0; i < (MAXCACHE - 1); i++)
    {
        if (strcmp(camino, UltimaEntradaEscritura[i].camino) == 0)
        {
            p_inodo = UltimaEntradaEscritura[i].p_inodo;
            enc = 1;
#if DEBUG9
            fprintf(stderr, "[mi_write() → Utilizamos la caché de escritura]\n");
#endif
            break;
        }
    }
    // En el caso de que no este en cache
    if (!enc)
    {
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        // si aun no se ha llenado la caché
        if (MAXCACHE > 0)
        {
            strcpy(UltimaEntradaEscritura[CACHE - MAXCACHE].camino, camino);
            UltimaEntradaEscritura[CACHE - MAXCACHE].p_inodo = p_inodo;
            MAXCACHE--;
#if DEBUG9
            fprintf(stderr, "[mi_write() → Actualizamos la caché de escritura]\n");
#endif
        }
        else // Remplazo FIFO
        {
            for (int i = 0; i < CACHE - 1; i++)
            {
                strcpy(UltimaEntradaEscritura[i].camino, UltimaEntradaEscritura[i + 1].camino);
                UltimaEntradaEscritura[i].p_inodo = UltimaEntradaEscritura[i + 1].p_inodo;
            }
            strcpy(UltimaEntradaEscritura[CACHE - 1].camino, camino);
            UltimaEntradaEscritura[CACHE - 1].p_inodo = p_inodo;

#if DEBUG9
            fprintf(stderr, "[mi_write() → Actualizamos la caché de escritura]\n");
#endif
        }
    }
    // Escribimos en el archivo
    int bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    if (bytes_escritos == -1)
    {
        bytes_escritos = 0;
    }
    return bytes_escritos;
}

// Función de directorios.c para leer los nbytes del fichero indicado por camino,
// a partir del offset pasado por parámetro y copiarlos en el buffer buf.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo = 0, p_inodo_dir = 0, p_entrada = 0;
    unsigned int enc = 0;
    for (int i = 0; i < (MAXCACHE - 1); i++)
    {
        if (strcmp(camino, UltimaEntradaEscritura[i].camino) == 0)
        {
            p_inodo = UltimaEntradaEscritura[i].p_inodo;
            enc = 1;
#if DEBUG9
            fprintf(stderr, "[mi_read() → Utilizamos la caché de lectura]\n");
#endif
            break;
        }
    }
    if (!enc)
    {
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        // si aun no se ha llenado la caché
        if (MAXCACHE > 0)
        {
            strcpy(UltimaEntradaEscritura[CACHE - MAXCACHE].camino, camino);
            UltimaEntradaEscritura[CACHE - MAXCACHE].p_inodo = p_inodo;
            MAXCACHE--;
#if DEBUG9
            fprintf(stderr, "[mi_read() → Actualizamos la caché de lectura]\n");
#endif
        }
        else
        {
            for (int i = 0; i < CACHE - 1; i++)
            {
                strcpy(UltimaEntradaEscritura[i].camino, UltimaEntradaEscritura[i + 1].camino);
                UltimaEntradaEscritura[i].p_inodo = UltimaEntradaEscritura[i + 1].p_inodo;
            }
            strcpy(UltimaEntradaEscritura[CACHE - 1].camino, camino);
            UltimaEntradaEscritura[CACHE - 1].p_inodo = p_inodo;

#if DEBUG9
            fprintf(stderr, "[mi_read() → Actualizamos la caché de lectura]\n");
#endif
        }
    }
    // Realiza la lectura del archivo.
    int bytes_leidos = mi_read_f(p_inodo, buf, offset, nbytes);
    if (bytes_leidos == -1)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return -1;
    }
    return bytes_leidos;
}