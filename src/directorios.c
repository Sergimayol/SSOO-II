/*
Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau
*/

#include "directorios.h"

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
        strncpy(inicial, (camino + 1), (strlen(camino) - strlen(rest) - 1));
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
}