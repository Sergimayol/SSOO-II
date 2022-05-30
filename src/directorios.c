/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

#define DEBUG 0   // Debugger del nivel 7
#define DEBUG8 0  // Debugger del nivel 8
#define DEBUG9 0  // Debugger del nivel 9
#define DEBUG10 1 // Debugger del nivel 10
#define DEBUG11 1 // Debugger del nivel 11

struct superbloque SB;
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
    fprintf(stderr, "Camino: %s\nInicio: %s\nFinal: %s\nTipo: %s\n", camino, inicial, final, tipo);
#endif
    return 0;
}

// Esta función nos buscará una determinada entrada (la parte *inicial del *camino_parcial
// que nos devuelva extraer_camino()) entre las entradas del inodo correspondiente a su
// directorio padre (identificado con *p_inodo_dir).
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = 0; // Inodo raìz
        *p_entrada = 0;
        return 0;
    }

    if (p_inodo == NULL)
    {
        // printf("DEBUG - buscar_entrada() | p_inodo es null! Set a 0\n");
        unsigned int placeholder = 0;
        p_inodo = &placeholder;
    }
    if (p_entrada == NULL)
    {
        // printf("DEBUG - buscar_entrada() | p_entrada es null! Set a 0\n");
        unsigned int placeholder = 0;
        p_entrada = &placeholder;
    }

    char inicial[MAX_CHAR];
    char final[strlen(camino_parcial)];
    memset(inicial, 0, MAX_CHAR);
    memset(final, 0, strlen(camino_parcial));
    char tipo;
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        fprintf(stderr, "Error en directorios.c buscar_entrada() --> Error en extraer camino\n");
        return -1;
    }
    struct inodo in;

    if (leer_inodo(*p_inodo_dir, &in) == -1)
    {
        fprintf(stderr, "Error en directorios.c buscar_entrada() --> Ha ocurrido un error leyendo el inodo\n");
        return -1;
    }
    // printf("DEBUG - buscar_entrada() | Desp. de lectura inodo | *p_inodo: %d | *p_entrada: %d\n",*p_inodo,*p_entrada);

    char buffer[in.tamEnBytesLog];
    struct entrada *entrada;
    entrada = malloc(sizeof(struct entrada));
    entrada->nombre[0] = '\0';
    int numentrades = in.tamEnBytesLog / sizeof(struct entrada);
    int nentrada = 0;

    // printf("DEBUG - buscar_entrada() | numentrades: %d | nentrada: %d\n",numentrades,nentrada);

    if (numentrades > 0)
    {
        // printf("DEBUG - buscar_entrada() | Permisos del inodo %d: %c\n",*p_inodo_dir, in.permisos);
        if ((in.permisos & 4) != 4)
        {
            fprintf(stderr, "Error en directorios.c buscar_entrada() --> No tiene permisos de lectura\n");
            return -1;
        }
        int offset = 0;
        int encontrado = 1;
        while (nentrada < numentrades && encontrado != 0)
        {
            mi_read_f(*p_inodo_dir, buffer, nentrada * sizeof(struct entrada), sizeof(buffer)); // leer siguiente entrada
            memcpy(entrada, buffer, sizeof(struct entrada));

            // printf("DEBUG - buscar_entrada() | inicial: %s | entrada->nombre: %s\n",inicial,entrada->nombre);

            encontrado = strcmp(inicial, entrada->nombre);
            // printf("DEBUG - buscar_entrada() | strcmp en while 1 OK\n");
            while (offset < numentrades && nentrada < numentrades && encontrado != 0)
            {
                // printf("DEBUG - buscar_entrada() | EN WHILE 2\n");
                nentrada++;
                offset++;
                memcpy(entrada, offset * sizeof(struct entrada) + buffer, sizeof(struct entrada));
                // printf("DEBUG - buscar_entrada() | EN WHILE 2 | inicial: %s | entrada->nombre: %s\n",inicial,entrada->nombre);
                encontrado = strcmp(inicial, entrada->nombre);
                // printf("DEBUG - buscar_entrada() | strcmp en while 2 OK\n");
            }
            offset = 0;
        }
    }
    if (nentrada == numentrades)
    {
        // printf("DEBUG - buscar_entrada() | En if 2\n");
        switch (reservar)
        {
        case 0:
            fprintf(stderr, "Error en directorios.c buscar_entrada() --> No existe entrada consulta\n");
            return -1;
        case 1:
            if (in.tipo == 'f')
            {
                fprintf(stderr, "Error en directorios.c buscar_entrada() --> Reservar = 1 y tipo de inodo = 'f'\n");
                return -1;
            }
            strcpy(entrada->nombre, inicial);
            if (tipo == 'd')
            {
                if (strcmp(final, "/") == 0)
                {
                    entrada->ninodo = reservar_inodo('d', permisos);
                }
                else
                {
                    fprintf(stderr, "Error en directorios.c buscar_entrada() --> No existe directorio intermedio\n");
                    return -1;
                }
            }
            else
            {
                entrada->ninodo = reservar_inodo('f', permisos);
            }
            if (mi_write_f(*p_inodo_dir, entrada, nentrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
            {
                if (entrada->ninodo != -1)
                {
                    liberar_inodo(entrada->ninodo);
                }
                fprintf(stderr, "Error en directorios.c buscar_entrada() --> No tiene permisos de escritura\n");
                return -1;
            }
        }
    }
    // printf("DEBUG - buscar_entrada() | Fuera de if 2\n");
    if ((strcmp(final, "/") == 0) || strcmp(final, "\0") == 0)
    {
        // printf("DEBUG - buscar_entrada() | strcmp final OK | final: %s\n",final);
        if ((nentrada < numentrades) && (reservar == 1))
        {
            fprintf(stderr, "Error en directorios.c buscar_entrada() --> Entrada ya existente\n");
            return -1;
        }
        *p_inodo = entrada->ninodo;
        *p_entrada = nentrada;
        // printf("DEBUG - buscar_entrada() | *p_inodo: %d | *p_entrada: %d\n",*p_inodo,*p_entrada);
        // printf("DEBUG - buscar_entrada() | Antes de return 0\n");
        return 0;
    }
    else
    {
        *p_inodo_dir = entrada->ninodo;
        // printf("DEBUG - buscar_entrada() | A final de todo, llamada recursiva\n");
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}

//
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -1:
        fprintf(stderr, ROJO_F "Error: Camino incorrecto.\n" RESET_FORMATO);
        break;
    case -2:
        fprintf(stderr, ROJO_F "Error: Permiso denegado de lectura.\n" RESET_FORMATO);
        break;
    case -3:
        fprintf(stderr, ROJO_F "Error: No existe el archivo o el directorio.\n" RESET_FORMATO);
        break;
    case -4:
        fprintf(stderr, ROJO_F "Error: No existe algún directorio intermedio.\n" RESET_FORMATO);
        break;
    case -5:
        fprintf(stderr, ROJO_F "Error: Permiso denegado de escritura.\n" RESET_FORMATO);
        break;
    case -6:
        fprintf(stderr, ROJO_F "Error: El archivo ya existe.\n" RESET_FORMATO);
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n" RESET_FORMATO);
        break;
    }
}

/* --------- Nivel 8 --------- */

// Función de la capa de directorios que crea un fichero/directorio
// y su entrada de directorio.
int mi_creat(const char *camino, unsigned char permisos)
{
#if DEBUG11
    mi_waitSem();
#endif
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
#if DEBUG11
    mi_waitSem();
#endif
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
    bread(posSB, &SB);
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    int error;
    // leemos el inodo, así que permisos bastan los de lectura
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        mostrar_error_buscar_entrada(error);
#if DEBUG
        printf("***********************************************************************\n");
#endif
        return EXIT_FAILURE;
    }
    mi_stat_f(p_inodo, stat);
    return p_inodo;
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

/* --------- Nivel 9 --------- */

// Crea el enlace de una entrada de directorio camino2 al
// inodo especificado por otra entrada de directorio camino1.
int mi_link(const char *camino1, const char *camino2)
{
#if DEBUG11
    mi_waitSem();
#endif
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct entrada entrada;
    struct inodo inodo;
    if (buscar_entrada(camino1, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0) < 0)
    {
        return -1;
    }
    int ninodo = p_inodo;
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }
    if (inodo.tipo != 'f' && (inodo.permisos & 4) != 4)
    {
        return -1;
    }
    p_inodo_dir = 0;
    if (buscar_entrada(camino2, &p_inodo_dir, &p_inodo, &p_entrada, 1, 6) < 0)
    {
        return -1;
    }
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1)
    {
        return -1;
    }
    liberar_inodo(entrada.ninodo);
    entrada.ninodo = ninodo;
    if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == -1)
    {
        return -1;
    }
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1;
    }
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    escribir_inodo(ninodo, inodo);
#if DEBUG11
    mi_signalSem();
#endif
    return 0;
}
// Función de la capa de directorios que borra la entrada del directorio especificado
int mi_unlink(const char *camino)
{
#if DEBUG11
    mi_waitSem();
#endif
    unsigned int p_inodo = 0, p_inodo_dir = 0, p_entrada = 0;
    struct inodo inodo;
    int err = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (err < 0)
    {
#if DEBUG10
        fprintf(stderr, "[mi_unlink() → Error buscando entrada]\n");
#endif
        mostrar_error_buscar_entrada(err);
        return -1;
    }
    // Inodo del archivo a borrar.
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
#if DEBUG10
        fprintf(stderr, "[mi_unlink() → Error leyendo inodo del archivo a borrar]\n");
#endif
        return -1;
    }
    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0))
    {
#if DEBUG10
        fprintf(stderr, "[mi_unlink() → Error el directorio no está vacío]\n");
#endif
        return -1;
    }
    // Inodo del directorio.
    struct inodo inodo_dir;
    if (leer_inodo(p_inodo_dir, &inodo_dir) == -1)
    {
#if DEBUG10
        fprintf(stderr, "[mi_unlink() → Error leyendo directorio]\n");
#endif
        return -1;
    }
    // Obtener el numero de entradas
    int num_entrada = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    // Entrada a eliminar es la última
    // Si hay mas entradas
    if (p_entrada != num_entrada - 1)
    {
        // Leer la última y colocarla en la posición de la entrada que queremos eliminar
        struct entrada entrada;
        if (mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (num_entrada - 1), sizeof(struct entrada)) < 0)
        {
#if DEBUG10
            fprintf(stderr, "[mi_unlink() → Error mi_read_f()]\n");
#endif
            return -1;
        }

        if (mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (p_entrada), sizeof(struct entrada)) < 0)
        {
#if DEBUG10
            fprintf(stderr, "[mi_unlink() → Error mi_write_f()]\n");
#endif
            return -1;
        }
    }
    // Elimina la ultima entrada.
    if (mi_truncar_f(p_inodo_dir, sizeof(struct entrada) * (num_entrada - 1)) == -1)
    {
#if DEBUG10
        fprintf(stderr, "[mi_unlink() → Error mi_truncar_f()]\n");
#endif
        return -1;
    }
    inodo.nlinks--;
    // Si no quedan enlaces (nlinks) entonces liberaremos el inodo
    if (!inodo.nlinks)
    {
        if (liberar_inodo(p_inodo) == -1)
        {
#if DEBUG10
            fprintf(stderr, "[mi_link() → Error liberando inodo 2]\n");
#endif
            return -1;
        }
    }
    else // Actualizamos su ctime y escribimos el inodo.
    {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, inodo) == -1)
        {
#if DEBUG10
            fprintf(stderr, "[mi_link() → Error escribiendo inodo]\n");
#endif
            return -1;
        }
    }
#if DEBUG11
    mi_signalSem();
#endif
    return 0;
}