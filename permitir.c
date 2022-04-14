#include "ficheros.h"

int main(int argc, char **argv)
{
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{ // dir ninodo permisos
		fprintf(stderr, "Error de sintaxis, la sintaxis correcta es: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
		return -1;
	}

	if (bmount(argv[1]) == -1)
	{
		return -1;
	}

	if (mi_chmod_f(atoi(argv[2]), atoi(argv[3])) == -1)
	{
		fprintf(stderr, "Error - permitir.c\n");
		return -1;
	}

	if (bumount() == -1)
	{
		return -1;
	}

	return 0;
}
