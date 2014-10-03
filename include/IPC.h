#ifndef IPC_H
#define IPC_H

#include "definiciones.h"


/**
 *Rutina para enviar datos
 *
 */
int enviarDatos(int des_archivo, void *valor, int tipo);

/**
 *Rutina para recibir datos
 */
int recibirDatos(int des_archivo, void *buffer, int tipo);


#endif

