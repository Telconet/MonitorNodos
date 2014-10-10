#include "mediciones.h"


int *canalesActivos;

/**
 *Wrapper para la inicializacion del sistema de adquisicion de datos
 */
int inicializarSistemaMediciones(int numeroMuestras, int numeroCanales) {

    //Inicializamos el ADC
    int res = inicializarADC(numeroMuestras, numeroCanales); //prueba

    statusTarjetaADC();

    convertirMinimosAfloat();

    canalesActivos = malloc(sizeof (int) *configuracion->numeroCanalesActivos);

    return res;

}

/**
 *Asigna memoria para una medicion
 */
struct medicion *crearMedicion(int tipo, int id, int valor, int status) {

    struct medicion *med = malloc(sizeof (struct medicion));

    if (med == NULL) {
        printf("ERROR: No se puedo crear la estructura de medicion.\n");
        return NULL;
    }

    //Asignamos los campos
    med->tipo_medicion = tipo;
    med->id_medicion = id;
    med->valor = valor;
    med->status = status;
    //Calculamos el tiempo
    //time_t tiempoBruto;

    //Tiempo en segundos desde espoca (Enero 1 1970 00:00)    
    //time(&tiempoBruto);
    //localtime(&tiempoBruto);

    med->tiempo = NULL;
    med->siguiente = NULL;
    return med;
}

/**
 *Libera la memoria asignada para la medicion apuntada por med
 */
void liberarMedicion(struct medicion *med) {
    if (med != NULL) {
        free(med);
    }
}

/**
 *Rutina que devuelve una lista inicializada de mediciones
 */
struct medicion *inicializarListaMediciones(int numeroMediciones) {

    struct medicion *inicio, *actual, *anterior;

    inicio = anterior = actual = NULL;

    //printf("Numero de mediciones: %d\n", numeroMediciones);


    int i = 0;

    for (i = 0; i < numeroMediciones; i++) {
        //printf("Iteracion: %d\n", i);
        if (i < OFFSET_HUMEDAD) {
            //temperatura
            actual = crearMedicion(TEMPERATURA, i - 3, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        } else if (i == OFFSET_HUMEDAD) {
            //humeddad
            actual = crearMedicion(HUMEDAD, 0, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        } else if (i > OFFSET_HUMEDAD && i < OFFSET_CORRIENTE_DC) {
            actual = crearMedicion(VOLTAJE_DC, i - 4, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        } else if (i >= OFFSET_CORRIENTE_DC && i < OFFSET_CORRIENTE_AC) {
            //corriente DC
            actual = crearMedicion(CORRIENTE_DC, i - 8, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        } else if (i >= OFFSET_CORRIENTE_AC && i < OFFSET_VOLTAJE_AC) {
            //corriente AC
            //DATACENTER
            //actual = crearMedicion(CORRIENTE_AC, i - 12, 0.0f, NINGUNO);
            actual = crearMedicion(CORRIENTE_DC, i - 12, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        } else if (i >= OFFSET_VOLTAJE_AC && i < OFFSET_NIVEL_COMBUSTIBLE) {
            //Voltaje AC
            actual = crearMedicion(VOLTAJE_AC, i - 16, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        }
        /*else if(i == OFFSET_NIVEL_COMBUSTIBLE){
            //Nivel de combustible
            actual = crearMedicion(NIVEL_COMBUSTIBLE, i - 18, 0.0f, NINGUNO);
            //printf("Iteracion: %d\n", i);
        }*/


        if (i == 0) {
            inicio = actual;
            anterior = actual;
        } else {
            anterior->siguiente = actual;
            anterior = actual;

            //ultimo de la lista  -- Chequear
            if (i == numeroMediciones - 1) {
                //printf("Crear lista mediciones iteracion %d\n",i);
                anterior->siguiente = NULL;
            }
        }
    }

    /*struct medicion *iniciotest = inicio;
    
    int cont = 0;
    while(iniciotest != NULL){
        printf("Contador: %d\n", cont);
        cont++;
        iniciotest = iniciotest->siguiente;
        
    }*/



    //Asignamos los valores apropiados
    return inicio;
}

/**
 *Rutina que libera la lista de mediciones
 */
int liberarListaMediciones(struct medicion *lista) {
    struct medicion *actual, *poreliminar;
    actual = lista;

    while (actual != NULL) {
        poreliminar = actual;
        actual = actual->siguiente;
        free(poreliminar);
    }

    return 0;
}

/**
 *Rutina que inicia una ronda de mediciones de las variables, usando el ADC
 *
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
int realizarMediciones(volatile struct medicion **med) { //CHECK!!!!!!!!!

#ifdef DEBUG
    printf("INFO: Realizando mediciones...\n");
#endif

    //Las mediciones se realizan en pares...
    //El ADC nos devuelve un voltaje entre 0 - 5V
    //y eso tenemos que convertirlo a la unidad
    //correspondiente.
    if (med != NULL) {
        int i = 0;
        //int numeroCanales = 24;
        int freqMuestreo = 1000;

        //Ya que los canales se leen en pares
        //creamos dos buffers para las mediciones.
        uint16_t* data_canal_2 = NULL;
        uint16_t* data_canal_1 = NULL;
        int noMuestras = NUMERO_MUESTRAS;
        adcrange rango = ADCRANGE_02VS;


        int tamanoBuffers = sizeof (uint16_t) * noMuestras;
        data_canal_1 = malloc(tamanoBuffers);
        

        //Guardamos temperatura para cualquier compensacion que sea necesaria.
        temperatura = 25;

        if (data_canal_1 == NULL) {
            printf("ERROR: No hay suficiente memoria para realizar las mediciones.\n");
            return -1;
        }
        
        data_canal_2 = malloc(tamanoBuffers);
        
        if (data_canal_2 == NULL) {
            printf("ERROR: No hay suficiente memoria para realizar las mediciones.\n");
            free(data_canal_1);
            return -1;
        }

        float voltajeADC = 0.0f;

        //float medicionesADC[NUMERO_MEDICIONES_ADC];
        volatile struct medicion *actual = *med;

        //24 canales ADC... tomamos solo los que nos interesan.

        //TODO: Si el canal no esta siendo USADO, sobretodo el de corriente AC, almancenar CERO!!!
        //USAR LISTA DE CANALES ACTIVOS.
        for (i = 0; i < NUMERO_CANALES_ADC; i++) {

            //el ADC se lee en las iteraciones pares.
            if (i % 2 == 0) {

                memset(data_canal_1, 0, tamanoBuffers);
                memset(data_canal_2, 0, tamanoBuffers);

                //Leemos el canal i e i+1 en una sola corrida
                obtenerMuestra(TARJETA_ADC24, i, freqMuestreo, noMuestras, rango, data_canal_1, data_canal_2);

                //Obtenemos el primer valor y lo asignamos
                voltajeADC = promedioVoltaje(data_canal_1, noMuestras, rango);
            } else {
                //En la iteracion impar, ya tenemos los datos leidos del ADC
                voltajeADC = promedioVoltaje(data_canal_2, noMuestras, rango);
            }

            //Vemos a que medicion nos referimos
            //y realizamos el calculo correspondiente.
            if (i == CANAL_TEMPERATURA_1) {
                //temperatura
                actual->valor = voltajeATemperatura(voltajeADC);

                //Sensor 1 esta junto a sensor de humedad               CHECK!!
                if (i == CANAL_TEMPERATURA_1) {
                    temperatura = actual->valor;
                }

                //printf("\nSe ha obtenido la medicion del canal %d: %.4f, temperatura: %.2f C\n", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            }

            if (i == CANAL_COMBUSTIBLE || i == CANAL_GENERADOR) {
                //voltaje del relé
                actual->valor = voltajeADC;

                actual = actual->siguiente;
            } else if (i == CANAL_HUMEDAD) {
                //humedad
                actual->valor = voltajeAHumedad(voltajeADC, temperatura);
                //printf("\nSe ha obtenido la medicion del canal %d: %.2f, humedad: %.2f %%HR\n,", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            } else if (i == CANAL_VOLTAJE_DC_1 || i == CANAL_VOLTAJE_DC_2 || i == CANAL_VOLTAJE_DC_3 || i == CANAL_VOLTAJE_DC_4) {
                //voltajes DC
                //
                //if(minimos[i] > 30.0f){
                actual->valor = voltajeAVoltajeDC(voltajeADC); //esto cambiara dependiendo de como se asignen
                //}
                /*else{
                    actual->valor = voltajeAVoltajeDC(voltajeADC, VOLTAJE_DC_24V);
                    actual = actual->siguiente;
                }*/
                //printf("\nSe ha obtenido la medicion del canal %d: %.2f, voltaje DC: %.2f V\n", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            } else if (i == CANAL_CORRIENTE_DC_1 || i == CANAL_CORRIENTE_DC_2 || i == CANAL_CORRIENTE_DC_3 || i == CANAL_CORRIENTE_DC_4) {
                //corriente DC
                actual->valor = voltajeACorrienteDC(voltajeADC);
                //printf("\nSe ha obtenido la medicion del canal %d: %.2f, Corriente DC: %.2f A\n", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            } else if (i == CANAL_CORRIENTE_AC_1 || i == CANAL_CORRIENTE_AC_2 || i == CANAL_CORRIENTE_AC_3 || i == CANAL_CORRIENTE_AC_4) {
                //DATACENTER
                //actual->valor = voltajeACorrienteAC(data_canal_1, noMuestras, rango); //Corriente AC solo esta en canales pares.
                actual->valor = voltajeACorrienteDC(voltajeADC);
                //printf("\nSe ha obtenido la medicion del canal %d: %.2f, Corriente AC: %.2f A RMS\n", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            } else if (i == CANAL_VOLTAJE_AC_1 || i == CANAL_VOLTAJE_AC_2) {
                //Voltaje AC
                actual->valor = voltajeAVoltajeAC(voltajeADC);
                //printf("\nSe ha obtenido la medicion del canal %d: %.2f, Voltaje AC: %.2f V RMS\n", i, voltajeADC, actual->valor);
                actual = actual->siguiente;
            }

            /*if(actual == NULL){
                printf("Actual es NULL\n");
            }*/
        }

        free(data_canal_1);
        free(data_canal_2);

        return 0;
    } else return -1;

}

/**
 *Esta rutina verifica que las mediciones esten dentro de los
 *valores permitidos. En caso de no ser asi, enviara las alertas
 *
 *Si en el archivo de los minimos, el minimo es menor a THRESHOLD_ACTIVACION_MEDICION, 
 *se desahbilita esa medicion.
 *
 *Las mediciones estan almacenadas en el siguiente orden:
 *                          i
 *ch0 - Corriente DC 1      0
 *ch1 - temperatura 1       1
 *ch2 - Corriente AC 3      2
 *ch3 - voltaje DC 2        3
 *ch4 - Corriente DC 2      4
 *ch5 - temperatura 2       5   
 *ch6 - Corriente AC 4      6
 *ch7 - voltaje DC 3        7
 *ch8 - Corriente DC 3      8
 *ch9 - temperatura 3       9
 *ch11 - voltaje DC 4       10
 *ch12 - Corriente DC 4     11
 *ch13 - voltaje AC 1       12
 *ch15 - humedad            13
 *ch16 - Corriente AC 1     14
 *ch17 - voltaje AC 2       15
 *ch20 - Corriente AC 2     16
 *ch21 - voltaje DC 1       17
 */
/*void revisarStatusMediciones(volatile struct medicion *med) {

    int i = 0;
    int j = 0;
    int indiceMinimos = 0;
    volatile struct medicion *actual = med;

#ifdef DEBUG
    printf("INFO: Revisando mediciones...\n");
#endif

    //Sacamos la lista de mediciones inactivos
    //PAra estos no enviamos alertas.
    //canalesActivos = malloc(sizeof(int)*configuracion->numeroCanalesActivos);
    //printf("Memoria asignada canales activos: %d\n", sizeof(int)*configuracion->numeroCanalesActivos);

    if (canalesActivos == NULL) {
        printf("ERROR: Problemas al asignar memoria en revisarStatusMediciones\n");
        return;
    }

    //printf("numeroCanalesActivos: %d\n", configuracion->numeroCanalesActivos);

    int w = 0;
    for (w = 0; w < configuracion->numeroCanalesActivos; w++) {
        canalesActivos[w] = atoi(configuracion->canalesActivos[w]);
        //printf("canal: %s\n", configuracion->canalesActivos[w]);
    }

    int indiceCanalActivo = 0;
    int canalActual = 0;
    
    int tamanoString = 400;
    char asunto[tamanoString];
    char mensaje[tamanoString];

    //empezamos a verificar, para mandar alertas o no.
    if (med != NULL) {

        while (i < NUMERO_CANALES_ADC) {
            j = 0;
            indiceCanalActivo = 0;

            for (indiceCanalActivo = 0; indiceCanalActivo < configuracion->numeroCanalesActivos; indiceCanalActivo++) {

                //Si el numero de canal esta en la lista, le permitimos seguir y mandar alertas. CHECK.
                if (actual != NULL && i == canalesActivos[indiceCanalActivo]) {

                    if ((i == CANAL_TEMPERATURA_1 || i == CANAL_COMBUSTIBLE || i == CANAL_GENERADOR) && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) { //CHECK
                        //temperatura
                        if (actual->valor > minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                if (i == CANAL_TEMPERATURA_1) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_TEMPERATURA_ALTA, OID_TEMPERATURA_1, actual->valor);
                                } else if (i == CANAL_COMBUSTIBLE) {
                                    //Rele NC a GND, combustible > 20 %
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_TEMPERATURA_ALTA, OID_TEMPERATURA_2, actual->valor);
                                } else if (i == CANAL_GENERADOR) {
                                    //Rele NC a GND, Generador apagado
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_TEMPERATURA_ALTA, OID_TEMPERATURA_3, actual->valor);
                                }
                            }

                            pthread_mutex_lock(&mutexEmailsAlerta);
                            switch (canalActual) {
                                case CANAL_TEMPERATURA_1:
                                    if (!alertaEmailEnviada[1]) { //Si no se ha enviado email...
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Temperatura 1 Alta.");
                                        snprintf(mensaje, tamanoString, "La temperatura 1 del nodo %s es %.2f °C.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[1] = 1;
                                    }
                                    break;
                                case CANAL_COMBUSTIBLE:
                                    if (!alertaEmailEnviada[5]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Combustible bajo.");
                                        snprintf(mensaje, tamanoString, "El nivel de combustible del nodo %s es bajo.", informacion_nodo.id);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[5] = 1;
                                    }
                                    break;
                                case CANAL_GENERADOR:
                                    if (!alertaEmailEnviada[9]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Generador encendido.");
                                        snprintf(mensaje, tamanoString, "El generador del nodo %s esta encendido.", informacion_nodo.id);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[9] = 1;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            pthread_mutex_unlock(&mutexEmailsAlerta);
                        }

                        //printf("Revisando medicion %d = %.4f, j = %d, indiceMinimos = %d\n", i, actual->valor, j, indiceMinimos);
                        //indiceMinimos++;
                        //actual = actual->siguiente;

                    } else if (i == CANAL_HUMEDAD && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) {
                        //humedad
                        if (actual->valor > minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_HUMEDAD_ALTA, OID_HUMEDAD, actual-> valor);
                            }
                        }

                        //TODO: humedad email

                    } else if ((i == CANAL_VOLTAJE_DC_1 || i == CANAL_VOLTAJE_DC_2 || i == CANAL_VOLTAJE_DC_3 || i == CANAL_VOLTAJE_DC_4) && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) {

                        //printf("Voltaje: %.2f", actual->valor);
                        if (actual->valor < minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                if (i == CANAL_VOLTAJE_DC_1) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_DC_BAJO, OID_VOLTAJE_DC_1, actual->valor);
                                } else if (i == CANAL_VOLTAJE_DC_2) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_DC_BAJO, OID_VOLTAJE_DC_2, actual->valor);
                                } else if (i == CANAL_VOLTAJE_DC_3) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_DC_BAJO, OID_VOLTAJE_DC_3, actual->valor);
                                } else if (i == CANAL_VOLTAJE_DC_4) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_DC_BAJO, OID_VOLTAJE_DC_4, actual->valor);
                                }
                            }

                            pthread_mutex_lock(&mutexEmailsAlerta);
                            switch (canalActual) {
                                case CANAL_VOLTAJE_DC_1:
                                    if (!alertaEmailEnviada[17]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje DC 1 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje DC 1 del nodo %s es %.2f V.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[17] = 1;
                                    }
                                    break;
                                case CANAL_VOLTAJE_DC_2:
                                    if (!alertaEmailEnviada[3]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje DC 2 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje DC 2 del nodo %s es %.2f V.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[3] = 1;
                                    }
                                    break;
                                case CANAL_VOLTAJE_DC_3:
                                    if (!alertaEmailEnviada[7]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje DC 3 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje DC 3 del nodo %s es %.2f V.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[7] = 1;
                                    }
                                    break;
                                case CANAL_VOLTAJE_DC_4:
                                    if (!alertaEmailEnviada[10]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje DC 4 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje DC 4 del nodo %s es %.2f V.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[10] = 1;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            pthread_mutex_unlock(&mutexEmailsAlerta);

                        }
                    } else if ((i == CANAL_CORRIENTE_DC_1 || i == CANAL_CORRIENTE_DC_2 || i == CANAL_CORRIENTE_DC_3 || i == CANAL_CORRIENTE_DC_4) && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) {
                        //Corriente DC   --> Mayor o menor?
                        if (actual->valor > minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                if (i == CANAL_CORRIENTE_DC_1) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_1, actual->valor);
                                } else if (i == CANAL_CORRIENTE_DC_2) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_2, actual->valor);
                                } else if (i == CANAL_CORRIENTE_DC_3) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_3, actual->valor);
                                } else if (i == CANAL_CORRIENTE_DC_4) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_4, actual->valor);
                                }
                            }
                        }
                    } else if ((i == CANAL_CORRIENTE_AC_3 || i == CANAL_CORRIENTE_AC_4 || i == CANAL_CORRIENTE_AC_2 || i == CANAL_CORRIENTE_AC_1) && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) {
                        
                        if (actual->valor > minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                if (i == CANAL_CORRIENTE_AC_1) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_1, actual->valor);
                                } else if (i == CANAL_CORRIENTE_AC_2) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_2, actual->valor);
                                } else if (i == CANAL_CORRIENTE_AC_3) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_3, actual->valor);
                                } else if (i == CANAL_CORRIENTE_AC_4) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_CORRIENTE_DC_ALTA, OID_CORRIENTE_DC_4, actual->valor);
                                }
                            }
                        }
                    } else if ((i == CANAL_VOLTAJE_AC_1 || i == CANAL_VOLTAJE_AC_2) && minimos[indiceMinimos] > THRESHOLD_ACTIVACION_MEDICION) {
                        //Voltaje AC - alarma si baja mucho el volaje
                        if (actual->valor < minimos[indiceMinimos]) {
                            for (j = 0; j < configuracion->numeroServidoresSNMP; j++) {
                                if (i == CANAL_VOLTAJE_AC_1) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_AC_BAJO, OID_VOLTAJE_AC_1, actual->valor);
                                } else if (i == CANAL_VOLTAJE_AC_2) {
                                    enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_ENTERSPECIFIC, TRAP_VOLTAJE_AC_BAJO, OID_VOLTAJE_AC_2, actual->valor);
                                }
                            }

                            pthread_mutex_lock(&mutexEmailsAlerta);
                            switch (canalActual) {
                                case CANAL_VOLTAJE_AC_1:
                                    if (!alertaEmailEnviada[12]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje AC 1 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje AC 1 del nodo %s es %.2f V RMS.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[12] = 1;
                                    }
                                    break;
                                case CANAL_VOLTAJE_AC_2:
                                    if (!alertaEmailEnviada[15]) {
                                        memset(asunto, 0, tamanoString);
                                        memset(mensaje, 0, tamanoString);
                                        snprintf(asunto, tamanoString, "ALERTA NODO: Voltaje AC 2 bajo.");
                                        snprintf(mensaje, tamanoString, "El voltaje AC 2 del nodo %s es %.2f V RMS.", informacion_nodo.id, actual->valor);
                                        enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas, asunto, "monitornodos@telconet.net", mensaje);
                                        alertaEmailEnviada[15] = 1;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            pthread_mutex_unlock(&mutexEmailsAlerta);
                        }
                    }
                }
            }

            //Seguimos avanzando en la lista
            if (indiceMinimos < NUMERO_MEDICIONES_ADC) {
                indiceMinimos++;
                actual = actual->siguiente;
            } else actual = NULL;

            //La variable i indica en que numero de medicion estamos. EL codigo mostrado abajo traduce esto al numero de canal
            //de la medicion en el ADC.
            i++;

            if (i < 10) {
                canalActual++;
            } else {
                switch (i) {
                    case 10:
                        canalActual = 11;
                        break;
                    case 11:
                        canalActual = 12;
                        break;
                    case 12:
                        canalActual = 13;
                        break;
                    case 13:
                        canalActual = 15;
                        break;
                    case 14:
                        canalActual = 16;
                        break;
                    case 15:
                        canalActual = 17;
                        break;
                    case 16:
                        canalActual = 20;
                        break;
                    case 17:
                        canalActual = 21;
                        break;
                    default:
                        //Llegamos al fin de la lista
                        break;
                }
            }
        }
        return;
    } else {
        printf("med es igual a NULL\n");
    }
}*/

/**
 *Rutina que almacena los valores de las mediciones en la base de datos.
 *Los valores son almancenados en la tabla <nombre_nodo>_<año>.
 */
int almacenarMediciones(volatile struct medicion **med, char *nombreNodo, char *rutaArchivoTipoColumnas, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts) {

    //MYSQL *conexion = conectarBD(configuracion->ipServidorBD, configuracion->usuarioBD, configuracion->claveBD, configuracion->BD);

    if (med != NULL && /*conexion != NULL &&*/ nombreNodo != NULL && numeroValores > 0) {

#ifdef DEBUG
        printf("\nINFO: Almacenando mediciones...\n");
#endif

        //Primero creamos el nombre de la tabla
        int longitudNombreTabla = (strlen(nombreNodo) + 20);
        char *nombreTabla = malloc(sizeof (char) *(longitudNombreTabla + 1));
        char *anio = (char *) obtenerAnio();

        if (nombreTabla == NULL) {
            printf("ERROR: No se pudo ingresar los valores medidos a la base de datos. Memoria insuficiente.\n");
            free(nombreTabla); //ok
            free(anio);
            return -1;
        }


        memset(nombreTabla, 0, longitudNombreTabla + 1);
        snprintf(nombreTabla, longitudNombreTabla, "%s_%s", nombreNodo, anio);
        ;

        //convertimos la lista de mediciones en un arreglo de strings
        char **medicionesStr = malloc(sizeof (char *) * (numeroValores + 3)); //18 mediciones y 3 columnas de nombre, fecha y hora = 21
        if (medicionesStr == NULL) {
            printf("ERROR: No se pudo ingresar los valores medidos a la base de datos. Memoria insuficiente.\n");
            free(nombreTabla); //ok
            free(anio);
            liberarMemoria(medicionesStr, numeroValores + 3);
            return -1;
        }


        //Las 3 primeras columnas son el nodo, fecha y hora
        medicionesStr[0] = nombreNodo;
        medicionesStr[1] = (char *) obtenerFecha();
        medicionesStr[2] = (char *) obtenerHora();

        int i = 0;
        volatile struct medicion *actual = *med;

        char* valorStr;
        for (i = 0; i < numeroValores; i++) {
            valorStr = malloc(sizeof (char) *10);

            //liberamos los asginado hasta este punto.
            if (valorStr == NULL) {
                free(nombreTabla); //ok
                free(anio);
                for (i = 0; i < numeroValores; i++) {
                    free(medicionesStr[i + 3]); //free(medicionesStr[i+3]);  
                }
                free(medicionesStr[1]); //ok
                free(medicionesStr[2]); //ok
                //NO LIBERAR medicioneStr[0] (nombreNodo), ya que los estariamos liberando varias veces, ya que es un puntero global!!!
                free(medicionesStr);
                return -1;
            }
            memset(valorStr, 0, 10);
            sprintf(valorStr, "%.4f", actual->valor);
            medicionesStr[i + 3] = valorStr; //strdup(valorStr);
            actual = actual->siguiente;
        }

        //Creamos la tabla, si ya existe, simplemente no se creará.
        int numeroColumnas = 0;
        char **tipoColumnas = (char **) leerArchivoTipoColumnas(rutaArchivoTipoColumnas, &numeroColumnas);

        if (tipoColumnas != NULL) {
            //int res = crearTabla(&conexion, nombreTabla, tipoColumnas, numeroColumnas); //numeroValores + 3
            //Ingresamos los datos a la tabla
            //insertarRegistro2(&conexion, nombreTabla, medicionesStr, numeroColumnas); //-->leak?
            insertarRegistro(nombreTabla, medicionesStr, numeroColumnas, stp, sts); //-->leak?
        }


        //CHECK...
        for (i = 0; i < numeroValores; i++) {
            free(medicionesStr[i + 3]); //free(medicionesStr[i+3]);  
        }
        free(medicionesStr[1]); //ok
        free(medicionesStr[2]); //ok
        //NO LIBERAR medicioneStr[0] (nombreNodo), ya que los estariamos liberando varias veces, ya que es un puntero global!!!
        free(medicionesStr);

        free(nombreTabla); //ok
        free(anio);

        //liberamos el tipo de columnas.
        liberarMemoria(tipoColumnas, numeroColumnas); //ok

        return 0;

    } else {
        printf("Error al almacenar mediciones.\n");
        return -1;
    }

    return 0;
}

/**
 *Almacenamos un evento en la base de datos
 */
int almacenarEvento(char *dispositivo, char *nombreNodo, char *rutaArchivoTipoColumnas, char *evento, char *notas) {

    //MYSQL *conexion = conectarBD(configuracion->ipServidorBD, configuracion->usuarioBD, configuracion->claveBD, configuracion->BD);
    /*
    if (dispositivo != NULL && conexion != NULL && nombreNodo != NULL && evento != NULL && dispositivo != NULL) {

        //Primero creamos el nombre de la tabla
        int longitudNombreTabla = (strlen(nombreNodo) + 200);
        char *nombreTabla = malloc(sizeof (char) *longitudNombreTabla);
        char *anio = (char *) obtenerAnio();

        if (nombreTabla == NULL) {
            printf("ERROR: No se pudo ingresar los valores medidos a la base de datos. Memoria insuficiente.\n");
            return -1;
        }


        memset(nombreTabla, 0, longitudNombreTabla);


        snprintf(nombreTabla, longitudNombreTabla, "%s_eventos_%s", nombreNodo, anio);

        //Ahora, obtenemos la fecha y hora de medicion
        char *fecha = (char *) obtenerFecha();
        char *hora = (char *) obtenerHora();

#ifdef DEBUG 
        printf("%s\n", anio);
        printf("%s\n", fecha);
        printf("%s\n", hora);
#endif


        //Creamos la tabla, si ya existe, simplemente no se creará.
        int numeroColumnas = 0;
        char **tipoColumnas = (char **) leerArchivoTipoColumnas(rutaArchivoTipoColumnas, &numeroColumnas);


        char **valores = malloc(sizeof (*valores) * numeroColumnas);

        valores[0] = nombreNodo;
        valores[1] = fecha;
        valores[2] = hora;
        valores[3] = dispositivo;
        valores[4] = evento;
        valores[5] = notas;

        int res = crearTabla(&conexion, nombreTabla, tipoColumnas, numeroColumnas);

        //Ingresamos los datos a la tabla
        insertarRegistro2(&conexion, nombreTabla, valores, numeroColumnas);

        free(nombreTabla);

        free(valores[1]);
        free(valores[2]);
        //free(valores[3]);
        //No liberamos los string literals (notas, eveto), ni dispositivo (liberando dos veces el mismo puntero)-> Segmentation fault!
        free(valores);

        liberarMemoria(tipoColumnas, numeroColumnas);
        cerrarBD(&conexion);

        return 1;
    } else return -1;*/
    
    //TODO: necesario reimplementar (ya no escribimos a base de datos directamente)
    return 0;
}

/**
 *Rutinas para activacion/desactivacion de reles
 */
int activarRele(int numeroRele, char *nombreNodo, char *rutaArchivoTipoColumnas) {

    //TODO activamos rele
    puerto_DIO puerto;

    switch (numeroRele) {
        case 1:
            puerto = puerto_DIO_0;
            break;
        case 2:
            puerto = puerto_DIO_1;
            break;
        case 3:
            puerto = puerto_DIO_2;
            break;
        case 4:
            puerto = puerto_DIO_3;
            break;
            /*case 4:
                puerto = puerto_DIO_4;
                break;
            case 5:
                puerto = puerto_DIO_5;
                break;
            case 6:
                puerto = puerto_DIO_6;
                break;
            case 7:
                puerto = puerto_DIO_7;
                break;*/
        default:
            puerto = puerto_DIO_NINGUNO;
            break;
    }


    int status = activarPuerto(puerto);
    printf("INFO: Rele %d activado.\n", numeroRele);

    if (!status) {

        //Registramos el evento
        char *buffer = malloc(sizeof (char) * 20);

        if (buffer == NULL) {
            printf("ERROR: No se pudo registrar el evento de activacion de rele.\n");
            return -1;
        }

        memset(buffer, 0, 20);
        sprintf(buffer, "rele %d", numeroRele);

        almacenarEvento(buffer, nombreNodo, rutaArchivoTipoColumnas, "activacion", "ninguna"); //--> segementation fault.    

        free(buffer);

        return 0;
    } else {
        printf("INFO: No se pudo activar el rele %d.\n", numeroRele);
        return -1;
    }
}

int desactivarRele(int numeroRele, char *nombreNodo, char *rutaArchivoTipoColumnas) {

    //TODO desactivamos rele
    puerto_DIO puerto;

    switch (numeroRele) {
        case 1:
            puerto = puerto_DIO_0;
            break;
        case 2:
            puerto = puerto_DIO_1;
            break;
        case 3:
            puerto = puerto_DIO_2;
            break;
        case 4:
            puerto = puerto_DIO_3;
            break;
            /*case 4:
                puerto = puerto_DIO_4;
                break;
            case 5:
                puerto = puerto_DIO_5;
                break;
            case 6:
                puerto = puerto_DIO_6;
                break;
            case 7:
                puerto = puerto_DIO_7;
                break;*/
        default:
            puerto = puerto_DIO_NINGUNO;
            break;
    }


    int status = desactivarPuerto(puerto);

    if (!status) {
        printf("INFO: Rele %d desactivado.\n", numeroRele);

        //Registramos el evento
        char *buffer = malloc(sizeof (char) * 20);

        if (buffer == NULL) {
            printf("ERROR: No se pudo registrar el evento de desactivacion de rele.\n");
            return -1; //Se pudo desactivar el rele, pero no almacenar el evento. Usamos otros codigo
        }

        memset(buffer, 0, 20);
        sprintf(buffer, "rele %d", numeroRele);

        almacenarEvento(buffer, nombreNodo, rutaArchivoTipoColumnas, "desactivacion", "ninguna");

        free(buffer);

        return 0;
    } else {
        printf("INFO: No se pudo desactivar el rele %d.\n", numeroRele);
        return -1;
    }


}

/**
 *Esta rutina convierte un voltaje leido por el ADC
 *a la correspondiente temperatura
 *
 *      0mV --> 2°C, luego por 10mV/°C
 *      
 */
float voltajeATemperatura(float voltaje) {

    float temperatura1 = -1.0f;

    temperatura1 = voltaje * 100.0f;

    return temperatura1;

}

/**
 *Esta rutina convierte el voltaje correspodiente
 *a la humedad relativa correspondiente.
 *Tenemos una salida de voltaje, que la podemos compensar
 *con temperatura
 */
float voltajeAHumedad(float voltaje, float temperatura) {

    float humedad = -1.0f;
    float vsupply = 5.000f;

    //Curva de primer orden, a 3.3V
    humedad = (voltaje / vsupply - 0.1515) / 0.00636;

    //Compensacion de temperatura
    humedad = humedad / (1.0546f - 0.00216 * temperatura);

    //printf("%.4f, %.4f\n", voltaje, humedad);

    return humedad;
}

/**
 * Convierte el voltaje del ADC a un voltaje de bateria proporcional
 */
float voltajeAVoltajeDC(float voltaje) {

    float voltajeDC = -1.0f;


    //voltajeDC = (1056000.0f)*voltaje/(56000.0f);        //en produccion será 1M || 56k!!

    //Original
    voltajeDC = (105600.0f) * voltaje / (5600.0f);

    return voltajeDC;
}

/**
 *
 */
float voltajeAVoltajeAC(float voltaje) {

    float voltajeAC = -1.0f;

    float pendienteVAC = 111.2789056;
    float constanteVAC = 6.186725f; //Opamp buffer sin capacitor de 1nF...

    voltajeAC = pendienteVAC * voltaje + constanteVAC;

    return voltajeAC;
}

/**
 * Sensor nos da un voltaje DC proporcional a la corriente
 * entrante. El sensor esta calibrado a 2.5V en 0A.
 * Cada ampeerio hace subir el voltaje 20mV. En caso de corriente
 * de direccion contraria, voltaje baja 20mV/A
 *
 *          100A  -->  4.5V
 *          0A    -->  2.5V
 *         -100A  -->  0.5V
 */
float voltajeACorrienteDC(float voltaje) {

    float corrienteDC = -1.0f;
    float voltioPorAmperio = 0.02000f;

    //Hasta 100A
    corrienteDC = (voltaje - 2.5000f) / voltioPorAmperio; //0.02000f

    return corrienteDC;
}

/**
 * El sensor de corrientes nos da una señal con un componente DC de 2.5V
 * y un rizado que varia en proporcion a la corriente DC. A medida
 * que aumenta la corriente, el voltaje pico a pico aumenta tambien,
 * hasta un maximo de 4.5V a 0.5 (100A).
 */
float voltajeACorrienteAC(uint16_t *voltajes, int numeroMuestras, adcrange rango) {

    float corrienteAC = -1.0f;
    float valor = 0.0f;
    int i = 0;

    //Calculamos la corriente RMS...
    float suma = 0.0000f;

    for (i = 0; i < numeroMuestras; i++) {
        valor = ((convertirAVoltaje(voltajes[i], rango) - 2.500f) / 0.020f);
        suma = suma + (valor * valor);
    }

    corrienteAC = (suma / ((float) numeroMuestras));
    corrienteAC = sqrt(corrienteAC);
    corrienteAC = corrienteAC * configuracion->razonCT;

    //Verificar min ??       
    return corrienteAC;
}

/**
 *
 */
float voltajeANivelCombustible(float voltaje) {
    return 0.0f;
}

/**
 *Convierte la lista de minimos de char a float
 *
 */
int convertirMinimosAfloat() {

    int tamano = sizeof (float) *configuracion->numeroValoresMinimosPermitidos;


    //return NULL;
    if (minimos == NULL) {
        minimos = malloc(tamano);
    }

    if (minimos != NULL) {
        int i = 0;

        for (i = 0; i < configuracion->numeroValoresMinimosPermitidos; i++) {
            float numero = atof(configuracion->valoresMinimosPermitidosMediciones[i]);
            minimos[i] = numero;

            //printf("Tamano de minimos = %d\n", tamano);
            //printf("Valor %d : %.4f\n", i, minimos[i]);
        }

        return 0;
    }
    else return -1;
}

