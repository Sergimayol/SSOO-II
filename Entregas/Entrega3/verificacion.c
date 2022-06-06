/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "verificacion.h"

#define DEBUGVERIFICACION 1 // debbuger programa verificacion

int main(int argc, char **argv)
{
    // Comprobar sintaxis
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación> \n");
        return -1;
    }
    // Montar el dispositivo
    if (bmount(argv[1]) == -1)
    {
        return -1;
    }
#if DEBUGVERIFICACION
    fprintf(stderr, "dir_sim: %s\n", argv[2]);
#endif
    // Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    struct STAT st;
    mi_stat(argv[2], &st);
    int num_entradas = st.tamEnBytesLog / sizeof(struct entrada);
    if (num_entradas != NUMPROCESOS) // Si son diferentes -> error
    {
        fprintf(stderr, "verificacion.c: Error en el número de entradas.\n");
        bumount();
        return -1;
    }
#if DEBUGVERIFICACION
    fprintf(stderr, "numentradas: %i, NUMPROCESOS: %i\n", num_entradas, NUMPROCESOS);
#endif
    // Crear el fichero "informe.txt" dentro del directorio de simulación
    char nfichero[100];
    sprintf(nfichero, "%s%s", argv[2], "informe.txt");
    if (mi_creat(nfichero, 7) < 0)
    {
        bumount(argv[1]);
        exit(0);
    }

    // Entradas del directorio de simulación
    struct entrada entradas[num_entradas];
    if (mi_read(argv[2], entradas, 0, sizeof(entradas)) == 0)
    {
        return -1;
    }
    int nbytes_info_f = 0;
    // Para cada entrada de directorio de un proceso
    for (int numEntrada = 0; numEntrada < num_entradas; numEntrada++)
    {
        // Leer la entrada de directorio y extraer el PID a partir del nombre
        // de la entrada y guardarlo en el registro info
        pid_t pid = atoi(strchr(entradas[numEntrada].nombre, '_') + 1);
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;

        char fichero_prueba[128]; // camino fichero
        sprintf(fichero_prueba, "%s%s/%s", argv[2], entradas[numEntrada].nombre, "prueba.dat");

        // Buffer de N registros de escrituras
        int total_registros_escritura = 256 * 24; // Un multiple de BLOCKSIZE, en plataforma de 64bits
        struct REGISTRO buffer_escrituras[total_registros_escritura];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        int offset = 0;
        // Mientras haya escrituras en prueba.dat
        while (mi_read(fichero_prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {
            // iterador escrituras buffer
            int indice = 0;
            while (indice < total_registros_escritura)
            {
                // Si la escritura es válida
                if (buffer_escrituras[indice].pid == info.pid)
                {
                    // Si es la primera escritura validada
                    if (!info.nEscrituras)
                    {
                        info.MenorPosicion = buffer_escrituras[indice];
                        info.MayorPosicion = buffer_escrituras[indice];
                        info.PrimeraEscritura = buffer_escrituras[indice];
                        info.UltimaEscritura = buffer_escrituras[indice];
                        info.nEscrituras++;
                    }
                    else
                    {
                        // Actualizamos los datos de las fechas la primera
                        // y si es preciso la última escritura
                        if ((difftime(buffer_escrituras[indice].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[indice].nEscritura < info.PrimeraEscritura.nEscritura)
                        {
                            info.PrimeraEscritura = buffer_escrituras[indice];
                        }
                        if ((difftime(buffer_escrituras[indice].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[indice].nEscritura > info.UltimaEscritura.nEscritura)
                        {
                            info.UltimaEscritura = buffer_escrituras[indice];
                        }
                        if (buffer_escrituras[indice].nRegistro < info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion = buffer_escrituras[indice];
                        }
                        if (buffer_escrituras[indice].nRegistro > info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion = buffer_escrituras[indice];
                        }
                        info.nEscrituras++;
                    }
                }
                indice++;
            }
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }

#if DEBUGVERIFICACION
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", numEntrada + 1, info.nEscrituras, fichero_prueba);
#endif
        // Añadir la información del struct info al fichero informe.txt por el final
        char tiempoPrimero[100];
        char tiempoUltimo[100];
        char tiempoMenor[100];
        char tiempoMayor[100];
        struct tm *tm;

        tm = localtime(&info.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %Y-%m-%d %H:%M:%S", tm);

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:"
                "\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMayor po"
                "sición:\t\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                tiempoPrimero,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                tiempoUltimo,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                tiempoMenor,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                tiempoMayor);
        // Escribimos en prueba.dat y actualizamos offset
        nbytes_info_f += mi_write(nfichero, &buffer, nbytes_info_f, strlen(buffer));
        if (nbytes_info_f < 0)
        {
            printf("verifiacion.c -> Error al escribir el fichero: '%s'\n", nfichero);
            bumount();
            return -1;
        }
    }
    // Desmontar dispositivo
    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}
