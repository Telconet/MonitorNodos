#ifndef MODBD_H
#define MODBD_H

#include "definiciones.h"
#include "monitordef.h"

#define BUFFER_CONSULTA_MAX 4096        //--CHECK antes 8192

MYSQL *conectarBD(char *servidor, char *usuario, char *clave, char *BD);

/**
 *Cierra la conexion a la base de datos;
 */
int cerrarBD(MYSQL **conexion);

int crearTabla(MYSQL **conexion, char *nombreTabla, char **tipoColumnas, int numeroColumnas);

int eliminarTabla(MYSQL **conexion, char *nombreTabla);

int insertarRegistro2(MYSQL **conexion, char *nombreTabla, char **valores, int numeroValores);
int insertarRegistro(char *nombreTabla, char **valores, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts);

char **obtenerNombreColumnas(MYSQL **conexion, char *nombreTabla, int *numeroColumnas);

char **obtenerTipoColumnas(MYSQL **conexion, char *nombreTabla, int *numeroColumnas);

char **leerArchivoTipoColumnas(char *rutaArchivo, int *numeroColumnas);

int necesitaApostrofe(char *tipo);

#endif

