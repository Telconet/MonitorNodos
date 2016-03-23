#ifndef CONTROLADORDEF_H
#define CONTROLADORDEF_H

#include <stdlib.h>
#include "definiciones.h"
#include "utilidades.h"
#include "IPC.h"


//Variables globales
int daemonPID;
int sd1;                //Socket para ocmunicarnos con el daemon de monitoreo

//Rutinas
/**
 *Rutina que hara limpieza de arhivos/proceso al salir
 */
void salir(int status);

void enviarSenalProceso(int senal, int pid);

int crearComando(char *comando_usuario, struct comando *com);

char *extraerArgumento(char *comando_usuario, int numero_de_argumento, int *longitud_argumento);

char *traducirStatusRespuesta(int codigo);

#endif

