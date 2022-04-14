#include "ficheros.h"

int main(int argc, char **argv)
{
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{ // Checkear syntax
		fprintf(stderr, "Error de sintaxis, la sintaxis correcta es: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_diferentes_inodos>\n");
		return -1;
	}
	// Pasamos el texto a un buffer de las 3 opciones que habia
	char buffer[18];
	strcpy(buffer, "Texto de prueba");

	int diferentes_inodos = atoi(argv[3]);
	int longitud = strlen(argv[2]);
	int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

	struct STAT stat;
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];

	bmount(argv[1]);

	printf("\n\nOffsets: 9000, 209000, 30725000, 409605000, 480000000\n");
	printf("longitud texto: %d\n", longitud);

	printf("\n###########################################################\n");

	int inodo_reservado = reservar_inodo('f', 6);
	printf("\nNº inodo reservado: %d\noffset: %d\n", inodo_reservado,
		   offsets[0]);
	int bytesEscritos = mi_write_f(inodo_reservado, argv[2], offsets[0],
								   longitud);
	printf("Bytes escritos: %d\n\n", bytesEscritos);

	mi_stat_f(inodo_reservado, &stat);

	printf("DATOS INODO: %d\n", inodo_reservado);
	printf("Tipo: %c\n", stat.tipo);
	printf("Permisos: %d\n", stat.permisos);
	ts = localtime(&stat.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	printf("atime: %s\n", atime);
	printf("mtime: %s\n", mtime);
	printf("ctime: %s\n", ctime);
	printf("numlinks: %d\n", stat.nlinks);
	fprintf(stderr, "tamEnBytesLog: %d\n", stat.tamEnBytesLog);
	fprintf(stderr, "numBloquesOcupados: %d\n", stat.numBloquesOcupados);

	printf("\n###########################################################\n");

	for (int i = 1; i < 5; ++i)
	{

		if (diferentes_inodos != 0)
		{
			inodo_reservado = reservar_inodo('f', 6);
		}

		printf("\nNº inodo reservado: %d\n", inodo_reservado);
		printf("offset: %d\n", offsets[i]);

		int bytesEscritos = mi_write_f(inodo_reservado, argv[2], offsets[i],
									   longitud);
		printf("Bytes escritos: %d\n\n", bytesEscritos);

		mi_stat_f(inodo_reservado, &stat);

		printf("DATOS INODO: %d\n", inodo_reservado);
		printf("Tipo: %c\n", stat.tipo);
		printf("Permisos: %d\n", stat.permisos);
		ts = localtime(&stat.atime);
		strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
		ts = localtime(&stat.mtime);
		strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
		ts = localtime(&stat.ctime);
		strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
		printf("atime: %s\n", atime);
		printf("mtime: %s\n", mtime);
		printf("ctime: %s\n", ctime);
		printf("numlinks: %d\n", stat.nlinks);
		fprintf(stderr, "tamEnBytesLog: %d\n", stat.tamEnBytesLog);
		fprintf(stderr, "numBloquesOcupados: %d\n", stat.numBloquesOcupados);

		printf("\n###########################################################\n");
	}

	bumount();

	return 0;
}