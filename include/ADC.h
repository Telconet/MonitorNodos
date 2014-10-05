#ifndef ADC_H
#define ADC_H

#include "definiciones.h"
#include "tsadclib1624.h"

#define TARJETA_ADC24 0


int inicializarADC(int numeroMuestras, int numeroCanales);

void statusTarjetaADC(void);

float convertirAVoltaje(uint16_t conteo, adcrange rangoADC);

float promedioVoltaje(uint16_t* valores, int numeroValores, adcrange rangoADC);

int obtenerMuestra(BoardIndex tarjeta, int canal, int freqMuestreo, int numeroMuestras, adcrange rangoADC, uint16_t* data_canal_1, uint16_t* data_canal_2);

#endif
