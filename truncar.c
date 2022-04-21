#include "ficheros.h"

//
int main(int argc, char **argv){
   
    struct STAT p_stat;
   
    //Validar sintaxis
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL ){  
        fprintf(stderr,"Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes> \n");
        return -1;
    }
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    // Montar el disco
    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error montando el disco\n");
        return -1;
    }
    if (nbytes == 0){
        liberar_inodo(ninodo);
    }else{
        mi_truncar_f(ninodo, nbytes);
    }
    
    //Desmontar el disco??
    mi_stat_f(ninodo, &p_stat);
  
}