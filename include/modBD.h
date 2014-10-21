#ifndef MODBD_H
#define MODBD_H

#include "definiciones.h"
#include "monitordef.h"

#define BUFFER_CONSULTA_MAX 4096        //--CHECK antes 8192

int insertarRegistro(char *nombreTabla, char **valores, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts);

int insertarEvento(int id_nodo, char *fecha, char *hora, char *evento);

char **leerArchivoTipoColumnas(char *rutaArchivo, int *numeroColumnas);

int necesitaApostrofe(char *tipo);

#endif

