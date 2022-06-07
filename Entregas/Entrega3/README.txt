Miembros:
 - Sergi Mayol Matos
 - Alejandro Rodríguez Arguimbau  

Sintaxis Específica:
 - escribir.c: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>
 - leer.c: leer <nombre_dispositivo> <numero_inodo>
 - leer_sf.c: leer_sf <nombre_dispositivo>
 - mi_fks.c: mi_fks <nombre del fichero> <numero de bloques>
 - permitir.c: permitir <nombre_dispositivo> <ninodo> <permisos>
 - truncar.c: truncar <nombre_dispositivo> <ninodo> <nbytes>
 - mi_cat.c: ./mi_cat </ruta_fichero>
 - mi_chmod.c: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta>
 - mi_escribir_varios.c: mi_escribir <nombre_dispositivo> </ruta_fichero>
 - mi_link.c: ./mi_link disco /ruta_fichero_original /ruta_enlace
 - mi_ls.c: ./mi_ls </ruta_directorio>
 - mi_rm.c: ./mi_rm </ruta_fichero>
 - mi_stat.c: ./mi_stat <disco> </ruta>
 - mi_touch.c: ./mi_touch <disco> <permisos> </ruta>
 - verificacion.c: ./verificacion <nombre_dispositivo> <directorio_simulación>
 - simulacion.c: ./simulacion disco

Observaciones:
 - En la funcion initMB del fichero: "ficheros_basico.c" para poner a 1 los bits del mapa de bits que corresponden
 a los metadatos hemos empleado la funcion: "reservar_bloque()".
 - En la mayoría de casos que se han podido emplear condicionales ternarios se han empleado, para simplificar el codigo
 - En cuanto a mensajes de errores solo salen desde el programa principal y los de debuggin se han dejado los niveles 5
 y 6.

Mejoras:
 - Para las funciones "mi_write()" y "mi_read()" de "direcotrios.c" se ha usado una CACHE de directorios de manera FIFO.
 - Los errores que se puedan causar durante la ejecución saldrán de color rojo.
 - En el programa mi_ls a la hora de imprimir la información se muestra con diferentes colores.