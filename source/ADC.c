#include "ADC.h"


/**
 *Rutina que inicializa la tarjeta ADC24
 *Esta rutina necesitar los siguientes paramentros:
 *
 *tarjeta: el model de la tarjeta  --> sbctype
 *maxchannel: la cantidad de canales que seran leidos ( 1 a 24)
 *buffer: el tamaño del buffer a ser leido, en bytes. Se calcula como 3*maxchannel*numeroDeMuestras
 */

int inicializarADC(int numeroMuestras, int numeroCanales){
    
    int res = -1;
        
    //el tamano del buffer maximo es 3*NUMERO_CANALES*NUMERO_DE_MUESTRAS
    int tamanoBuffer = 3*numeroCanales*numeroMuestras;
    
    res = tsadclib1624_init(SBC_TS7200, numeroCanales, tamanoBuffer);
    
    if(res < 0){
        printf("ERROR: No se pudo inicializar el ADC (Codigo de error %d).\n", res);
    }
    else{
        printf("INFO: El ADC se ha inicializado correctamente.\n");
    }
    
    tsadclib1624_verbose(false);

#ifndef DEBUG_ADC
    //tsadclib1624_verbose(false);
#endif
    
    return res;
}


/**
 * Rutina que nos devuelve el status de la tarjeta de monitoreo
 */
void statusTarjetaADC(void){
    
    adctype tarjetaADC = tsadclib1624_boardstatus(0);
    
    if(tarjetaADC == TS_ADC24){
        printf("INFO: La tarjeta TS-ADC24 se encuentra funcionando correctamente.\n");
        return;
    }
    else if(tarjetaADC == TS_ADCINVALID){
        printf("ERROR: No se encuentra la tarjeta ADC.\n");
        return;
    }
}


/**
 *Convierte los conteos del ADC en un voltaje
 */
float convertirAVoltaje(uint16_t conteo, adcrange rangoADC){
    
        float valor;
        
        //En modo 0 a 2xVref, el rango 0 - 2.5V esta entre 2048 y 4095 counts
        //y el rango 2.5V - 5V esta entre 
        if((rangoADC == ADCRANGE_02VS || rangoADC == ADCRANGE_02VAS || rangoADC == ADCRANGE_02VBS) ){

            //ORIGINAL con signed int
            if(conteo == 0){
                //TODO --> problematico...
                valor = 2048*VOLTIOS_POR_ESCALON_2VREF;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }
            /*else if(conteo == 2048){         //PARTE BAJA DE ESCALA... (cero). REVISAR PARA EQUIPO PRODUCCION
                //TODO --> problematico...
                valor = 0;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }*/
            else if(conteo <= 2048 && conteo != 0){                 //original <=, voltajes 0.00122 a 2.5V -> REVISAR PARA EQUIPO PRODUCCION
                valor = (conteo - 2048)*VOLTIOS_POR_ESCALON_2VREF*-1;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }
            else if(conteo > 2048 && conteo != 0){                  //original >, voltajes 2.5+0.122 a 5.00V
                int tmp = 2048 - conteo + 4096;
                valor = (float)tmp*VOLTIOS_POR_ESCALON_2VREF;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }
            
            
            
            //PRUeBA
            /*if(conteo == 0){            //MITAD DE ESCALA..
                //TODO --> problematico...
                valor = 2048*VOLTIOS_POR_ESCALON_2VREF;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }
            else if(conteo == 2048){         //PARTE BAJA DE ESCALA... (cero)
                //TODO --> problematico...
                valor = 0;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }
            else if(conteo < 2048){                  //valores de 2.500 + 0.00122V a 5.00V  (1 a 2047)       
                int tmp = 2048 + conteo;
                valor = (float)tmp*VOLTIOS_POR_ESCALON_2VREF;
                //valor = (conteo - 2048)*VOLTIOS_POR_ESCALON_2VREF*-1;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor; 
            }
            else if(conteo > 2048){                  //valores de 0.00122V a 2.500 - 0.00122 (2049 a 4095)
                //int tmp = 2048 - conteo + 4096;
                //valor = (float)tmp*VOLTIOS_POR_ESCALON_2VREF;
                valor = (conteo - 2048)*VOLTIOS_POR_ESCALON_2VREF*-1;
                //printf("voltaje: %.4f, conteo: %d\n", valor, conteo);
                return valor;
            }*/
        
        }
        else if(rangoADC == ADCRANGE_0VS){
            //Modo de 0 - Vref (2.5V)
            //printf("conteo: %d\n", conteo);
            valor = (conteo)*2.5f / ((float)4096);
            return valor;
        }
        
        return -1.0f;
}


//Promedio voltaje
float promedioVoltaje(uint16_t* valores, int numeroValores, adcrange rangoADC){
    
    float total = 0.0f;
    int i = 0;
    
    if(valores != NULL && numeroValores > 0){
        for(i = 0 ; i < numeroValores; i++){
            total += convertirAVoltaje(valores[i], rangoADC);
        }
        
        return (total / ((float)numeroValores));
    }
    else return -1.0f;
}



int obtenerMuestra(BoardIndex tarjeta, int canal, int freqMuestreo, int numeroMuestras, adcrange rangoADC, uint16_t* data_canal_1, uint16_t* data_canal_2){
    
    //buffers para canales adyacentes. Numero de muestras no puede exceder el numero especificado
    //en la llamada de inicializacion
    int tamanoBuffers = sizeof(uint16_t)*numeroMuestras;
    
    if(data_canal_2 == NULL || data_canal_1 == NULL){
        printf("ERROR: No hay suficiente memoria para adquirir datos de ADC.\n");
    }
    
    memset(data_canal_1, 0, tamanoBuffers);
    memset(data_canal_2, 0, tamanoBuffers);
    
    //Empezamos a la adquisicion de datos
    int res = tsadclib1624_startAcquisution(tarjeta, canal, freqMuestreo, numeroMuestras, rangoADC, data_canal_1, data_canal_2, false);
    
    if(res != 0){
        printf("ERROR: Error al adquirir las muestras del ADC.\n");
        
        if(res == TSADCERR_NOINIT){
            printf("ERROR: No se ha inicializado el ADC.\n");
        }
        else if(res == TSADCERR_BADBOARD ){
           printf("ERROR: La id del ADC es incorrecta.\n"); 
        }
        else if(res == TSADCERR_BADPARAMETER ){
            printf("ERROR: Numero de canal de ADC incorrecto.\n");
        }
        return -1;
    }
    
    tsadclib1624_pollResults(tarjeta, true);
    

    
    //resultados son dados en two's complement.
    int i;
    float promedio_canal1 = 0.0f;
    float promedio_canal2 = 0.0f;
    for( i = 0; i < numeroMuestras; i++){
               
        promedio_canal1 += convertirAVoltaje(data_canal_1[i], rangoADC);//valor1;
        promedio_canal2 += convertirAVoltaje(data_canal_2[i], rangoADC);//valor2;
        
        //quitar...
        //printf("%.6f\n",convertirAVoltaje(data_canal_1[i], rangoADC));
        //printf("Muestra %d canal %d: %.6f volts. Numero de counts: %u\n", i, canal, convertirAVoltaje(data_canal_1[i], rangoADC), data_canal_1[i]);
//#ifdef DEBUG_ADC
        //printf("Muestra %d canal %d: %.6f volts. Numero de counts: %u\n", i, canal, convertirAVoltaje(data_canal_1[i], rangoADC), data_canal_1[i]);
        //printf("Muestra %d canal %d: %.6f volts. Numero de counts: %u\n", i, canal + 1, convertirAVoltaje(data_canal_2[i], rangoADC), data_canal_2[i]);
//#endif
    }
    
#ifdef DEBUG_ADC
    printf("Promedio 1: %.6f V.\n", promedio_canal1 / (float)numeroMuestras);
    printf("Promedio 2: %.6f V.\n", promedio_canal2 / (float)numeroMuestras);
#endif
    
    return 1;
}




