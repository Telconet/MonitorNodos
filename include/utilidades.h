#ifndef UTILIDADES_H
#define UTILIDADES_H

#include "definiciones.h"


/**
 *Aqui reportamos el PID al proceso controlador del daemon
 */
void reportarPID(char *archivoProceso, int processID);

/**
 *Rutina para verificar que esta corriendo una sola instancia del daemon
 */
void verificarProgramaNoCorriendo(char *archivoProceso);

/**
 *Rutina para buscar el PID del daemon de monitoreo
 *
 */
int buscarArchivoPIDProceso(char *archivoProceso);


/**
 *Reemplaza el caracter de salto de linea
 */
void removerSaltoDeLinea(char *string, int tamano);


//RELACIONADAS A IP
char *obtenerInterfazDeRed(void);

int obtenerIPHost(char *host, int maxhost);

int parse_IP(char *ip_string, unsigned char *ip); //revisar

//RELACIONADA al TIEMPO
char *obtenerHora(void);

char *obtenerFecha(void);

char *obtenerAnio(void);

//ADMINISTRACION DE MEMORIA
void liberarMemoria(char **bloque, int numeroElementos);

//Matematicas


#endif
