#include "ficheros.h"

struct STAT aux_stat;
struct tm *ts;
char adate[80], mdate[80], cdate[80];

int printSTAT(int inodo_reservado);

int main(int argc, char **argv)
{
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{ // Checkear syntax
		fprintf(stderr, "Error de sintaxis, la sintaxis correcta es: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_diferentes_inodos>\nOffsets: 9000, 209000, 30725000, 409605000, 480000000\n Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
		return -1;
	}
	// Pasamos el texto a un buffer de las 3 opciones que habia
	char buffer[18];
	strcpy(buffer, "Texto de prueba");

	int diferentes_inodos = atoi(argv[3]);
	int longitud = strlen(argv[2]);
	int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

	bmount(argv[1]);

	printf("\n\nOffsets: 9000, 209000, 30725000, 409605000, 480000000\n");
	printf("longitud texto: %d\n", longitud);

	int inodo_reservado = reservar_inodo('f', 6);
	printf("\nNº inodo reservado: %d\noffset: %d\n", inodo_reservado,
		   offsets[0]);
	int bytesEscritos = mi_write_f(inodo_reservado, argv[2], offsets[0], longitud);
	printf("Bytes escritos: %d\n\n", bytesEscritos);

	printSTAT(inodo_reservado);

	for (int i = 1; i < 5; ++i)
	{
		if (diferentes_inodos != 0)
		{
			inodo_reservado = reservar_inodo('f', 6);
		}

		printf("\nNº inodo reservado: %d\n", inodo_reservado);
		printf("offset: %d\n", offsets[i]);

		int bytesEscritos = mi_write_f(inodo_reservado, argv[2], offsets[i], longitud);
		printf("Bytes escritos: %d\n\n", bytesEscritos);
		printSTAT(inodo_reservado);
	}

	bumount();

	return 0;
}

int printSTAT(int inodo_reservado)
{
	mi_stat_f(inodo_reservado, &aux_stat);
	printf("DATOS INODO: %d\n", inodo_reservado);
	printf("Tipo: %c\n", aux_stat.tipo);
	printf("Permisos: %d\n", aux_stat.permisos);
	ts = localtime(&aux_stat.atime);
	strftime(adate, sizeof(adate), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&aux_stat.mtime);
	strftime(mdate, sizeof(mdate), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&aux_stat.ctime);
	strftime(cdate, sizeof(cdate), "%a %Y-%m-%d %H:%M:%S", ts);
	printf("atime: %s\n", adate);
	printf("mtime: %s\n", mdate);
	printf("ctime: %s\n", cdate);
	printf("numlinks: %d\n", aux_stat.nlinks);
	fprintf(stderr, "tamEnBytesLog: %d\n", aux_stat.tamEnBytesLog);
	fprintf(stderr, "numBloquesOcupados: %d\n", aux_stat.numBloquesOcupados);
	return 0;
}