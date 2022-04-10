#include "ficheros.h"
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }

    if ((inodo.permisos & 2) != 2)
    {
        // Error de permiso
        fprintf(stderr, "Error: Permiso de escritura denegado. \n");
        return 0;
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    int nbytesEscritos=0;

    if (primerBL == ultimoBL)
    {
        //A revisar
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        res = bwrite(nbfisico, buf_bloque);
    if (res == -1)
    {
        fprintf(stderr, "Error al escribir el superBloque. \n");
        return -1;
    }
nbytesEscritos=nbytes;

    }
    else
    {
        
        //Escribimos el 1er bloque
memcpy (buf_bloque + desp1, buf_original, BLOCKSIZE-desp1);
 res = bwrite(nbloqueFisico, buf_bloque);
    if (res == -1)
    {
        fprintf(stderr, "Error al escribir el superBloque \n");
        return -1;
    }

nbytesEscritos=BLOCKSIZE-desp1;

//Escribimos los bloques intermedios
int primerIntermedio = primerBL + 1;
while(primerIntermedio< ultimoBL){

      nbloqueFisico = traducir_bloque_inodo(ninodo, i, 1);
            res= bwrite(nbloqueFisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if(res == -1){ 
                fprintf(stderr, "Error al escribir el superBloque \n");
                return -1;
            }
            nbytesEscritos+=BLOCKSIZE;
    primerIntermedio++;
}
//Ultimo bloque
int nUltimobloqueFisico=traducir_bloque_inodo(ninodo,primerIntermedio,1);

res=bread(nUltimobloqueFisico, buf_bloque);
if( res == -1){ 
        fprintf(stderr, "Error al leer el bloque \n");
        return -1;
    }
memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

 res= bwrite(nbloqueFisico,buf_bloque);
            if(res == -1){ 
                fprintf(stderr, "Error al escribir el superBloque \n");
                return -1;
            }

            nbytesEscritos+=desp2 + 1;
    }
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    int leidos;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }
    if ((inodo.permisos & 4) != 4)
    {
        // Error de permiso
        fprintf(stderr, "Error: Permiso de lectura denegado. \n");
        return 0;
    }
    if (offset >= inodo.tamEnBytesLog)
    {
        leidos = 0; // No podemos leer nada
        return leidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    { // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF
    }
    //Primer bloque
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; 
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    int nbytesLeidos = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    memset(buf_bloque, 0, sizeof(buf_bloque)); 
    
    if(primerBL == ultimoBL){
        int BF = traducir_bloque_inodo(ninodo, primerBL, 0);
        if(BF != -1){
            res= bread(BF, buf_bloque);
            if(res == -1){
        fprintf(stderr, "Error al leer buf_bloque || mi_read_f() \n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
            nbytesLeidos = nbytes;
        }  
    }else{ //Caso de varios

        //Primer bloque        
        int nbloqueFisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if(nbloqueFisico != -1){
            res = bread(nbloqueFisico, buf_bloque);
            if(res == -1){
                fprintf(stderr, "Error al lectura \n");
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        } 
        nbytesLeidos = BLOCKSIZE - desp1;

        //Bloque intermedios 
        for(int i = primerBL + 1; i < ultimoBL; i++){
            nbloqueFisico = traducir_bloque_inodo(ninodo, i, 0);
            if(nbloqueFisico != -1){
                res = bread(nbloqueFisico, buf_bloque);
                if(res  == -1){
                    fprintf(stderr, "Error al lectura \n");
                    return -1;
                }
                memcpy((buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE), buf_bloque, BLOCKSIZE);
                nbytesLeidos += desp2 + 1; 
            }           
        }

        //ultimo bloque
        nbloqueFisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if(nbloqueFisico != 1){
            res = bread(nbloqueFisico, buf_bloque);
            if(res == -1){
                    fprintf(stderr, "Error al lectura \n");
                return -1;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque,  desp2 + 1);	
            nbytesLeidos += desp2 + 1;
        } 
    }

    //Actualizar metadatos
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr, "Error al lectura inodo \n");
        return -1;
    }
    inodo.atime = time(NULL);
    res = escribir_inodo(ninodo, inodo);
    if(res == -1){
        fprintf(stderr, "Error al escritura inodo || mi_read_f() \n");
        return -1;
    }

    return nbytesLeidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return EXIT_FAILURE;
    }
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;

int res= leer_inodo(ninodo, &inodo);
    if(res == -1){
        fprintf(stderr, "Error al lectura inodo \n");
        return -1;
    }

    inodo.permisos=permisos;
    inodo.ctime=time(NULL);

res= escribir_inodo(ninodo, &inodo);
    if(res == -1){
        fprintf(stderr, "Error al escribir inodo \n");
        return -1;
    }
    return 0;
}