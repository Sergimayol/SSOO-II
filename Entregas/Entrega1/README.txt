Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau  
 - Carlos Galardon Burges

Sintaxis Específica:
 - escribir.c: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
 - leer.c: leer <nombre_dispositivo> <numero_inodo>
 - leer_sf.c: leer_sf <nombre_dispositivo>
 - mi_fks.c: mi_fks <nombre del fichero> <numero de bloques>
 - permitir.c: permitir <nombre_dispositivo> <ninodo> <permisos>
 - truncar.c: truncar <nombre_dispositivo> <ninodo> <nbytes>

Observaciones:
 - En la funcion initMB del fichero: "ficheros_basico.c" para poner a 1 los bits del mapa de bits que corresponden
 a los metadatos hemos empleado la funcion: "reservar_bloque()".
 - En la mayoría de casos que se han podido emplear condicionales ternarios se han empleado, para simplificar el codigo
 - En cuanto a mensajes de errores solo salen desde el programa principal y los de debuggin se han dejado los niveles 5
 y 6.