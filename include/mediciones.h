#ifndef MEDICIONES_H
#define MEDICIONES_H

#include <math.h>
#include "definiciones.h"
#include "monitordef.h"
#include "ADC.h"
#include "DIO.h"


//Definimos los offsets de la lista de mediciones
#define OFFSET_TEMPERATURA 0
#define OFFSET_HUMEDAD  3
#define OFFSET_VOLTAJE_DC 4
#define OFFSET_CORRIENTE_DC 8
#define OFFSET_CORRIENTE_AC 12
#define OFFSET_VOLTAJE_AC 16
#define OFFSET_NIVEL_COMBUSTIBLE 18

/*Definimos los canales asignados a cada medicion
 *ch0 - Corriente DC 1
 *ch1 - temperatura 1
 *ch2 - Corriente AC 3
 *ch3 - voltaje DC 2
 *ch4 - Corriente DC 2
 *ch5 - temperatura 2
 *ch6 - Corriente AC 4
 *ch7 - voltaje DC 3
 *ch8 - Corriente DC 3
 *ch9 - temperatura 3
 *ch11 - voltaje DC 4
 *ch12 - Corriente DC 4
 *ch13 - voltaje AC 1
 *ch15 - humedad
 *ch16 - Corriente AC 1
 *ch17 - voltaje AC 2
 *ch20 - Corriente AC 2
 *ch21 - voltaje DC 1
 */
#define CANAL_CORRIENTE_DC_1    0
#define CANAL_CORRIENTE_DC_2    4
#define CANAL_CORRIENTE_DC_3    8
#define CANAL_CORRIENTE_DC_4    12
#define CANAL_CORRIENTE_AC_1    16
#define CANAL_CORRIENTE_AC_2    20
#define CANAL_CORRIENTE_AC_3    2
#define CANAL_CORRIENTE_AC_4    6
#define CANAL_TEMPERATURA_1     1
#define CANAL_COMBUSTIBLE       5
#define CANAL_GENERADOR         9
#define CANAL_VOLTAJE_AC_1      13
#define CANAL_VOLTAJE_AC_2      17
#define CANAL_VOLTAJE_DC_1      21
#define CANAL_VOLTAJE_DC_2      3
#define CANAL_VOLTAJE_DC_3      7
#define CANAL_VOLTAJE_DC_4      11
#define CANAL_HUMEDAD           15

//
#define NUMERO_CANALES_ADC 24

//Definimos cuando no debajo de donde no tomar
//en cuenta una medicion
#define THRESHOLD_ACTIVACION_MEDICION   0.01f

//minimos
float *minimos;             //Variable global se inicializa en 0 (NULL)


//Definicion de las funciones

float temperatura;

int inicializarSistemaMediciones(int numeroMuestras, int numeroCanales);

struct medicion *inicializarListaMediciones(int numeroMediciones);

int liberarListaMediciones(struct medicion *lista);

struct medicion *crearMedicion(int tipo, int id, int valor, int status);

int realizarMediciones(volatile struct medicion **med);

//void revisarStatusMediciones(volatile struct medicion *med);

int almacenarMediciones(volatile struct medicion **med, char *nombreNodo, char* rutaArchivoTipoColumnas, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts);

int almacenarEvento(char *dispositivo, char *nombreNodo, char *rutaArchivoTipoColumnas, char *evento, char *notas);

void liberarMedicion(struct medicion *med);

int activarRele(int numeroRele, char *nombreNodo, char *rutaArchivoTipoColumnas);

int desactivarRele(int numeroRele, char *nombreNodo, char *rutaArchivoTipoColumnas);

float voltajeATemperatura(float voltaje);

float voltajeAHumedad(float voltaje, float temperatura);

float voltajeAVoltajeDC(float voltaje);

float voltajeAVoltajeAC(float voltaje);

float voltajeACorrienteDC(float voltaje);

float voltajeACorrienteAC(uint16_t *voltajes, int numeroMuestras, adcrange rango);

float voltajeANivelCombustible(float voltaje);

float *convertirAfloat();

int convertirMinimosAfloat();


#endif

