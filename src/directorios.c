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
}

// Esta función nos buscará una determinada entrada (la parte *inicial del *camino_parcial
// que nos devuelva extraer_camino()) entre las entradas del inodo correspondiente a su
// directorio padre (identificado con *p_inodo_dir).
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos)
{
}