#include "monitordef.h"

/**
 *Rutina para hacer del proceso un daemon. TODO
 *
 */
void daemonize(){
    
    int i=fork();           //Emancipamos el proceso...
    if (i<0) exit(1); 
    if (i>0) exit(0); 
}

/**
 *Programa principal
 */
int main(int argc, char *argv[]){
    
    //cambiemos el directorio de trabajo
    chdir(DIRECTORIO_DE_TRABAJO);
    
    //Configuramos los signal handlers, para manejar Ctrl-C
    signal(SIGINT, manejadorSenalSIGTERMSIGINT);
    signal(SIGTERM, manejadorSenalSIGTERMSIGINT);
    
    processID = getpid();

    //Primero verificamos si el archivo existe
    //Tratamos de abrir el archivo, si no hay error, el archivo existe, y ya
    //hay un proceso corriendo
    verificarProgramaNoCorriendo(ARCHIVO_PROCESS_ID_DAEMON); 
    
    //Aqui reportamos nuestro pid al proceso de control
    reportarPID(ARCHIVO_PROCESS_ID_DAEMON, processID);
    
    //Leemos el archivo de configuracion. Extraemos la ruta del archivo
    //de los argumentos de ejecucion
    if(argc < 2){
        printf("ERROR: No se ha proporcionado la ruta del archivo de configuracion. Este programa se cerrara.\n");
        salir(EXIT_FAILURE);
    }
    
    char * rutaArchivoConfiguracion = argv[1];
    configuracion = leerArchivoConfiguracion(rutaArchivoConfiguracion);
    
    if(configuracion == NULL){
        printf("ERROR: Existe un error en el formato del archivo de configuracion %s,revise la configuracion del mismo. Este programa se cerrara.\n", rutaArchivoConfiguracion);
        salir(1);
    }
    
    //Solo para informacion
    configuracion->interfazRed = obtenerInterfazDeRed();
    
   
    //Esta rutina inicializa todo el sistema de mediciones
    //(asigna memoria, inicializa y configura el ADC, etc)
    inicializarSistemaMediciones(NUMERO_MUESTRAS, NUMERO_MAXIMO_CANALES_ADC);
    
    //Configuramos el servidor SNMP. SESION DEBE ABRIRSE ANTES DE CREAR THREAD DE MONITOREO DE PUERTA.
    ss = abrirMultiplesSesiones(configuracion->ipServidorSNMP, configuracion->numeroServidoresSNMP,
                                                  configuracion->nombreSesionSNMP, configuracion->comunidadSNMP, SNMP_VERSION_1);
    
    if(ss == NULL){
        printf("ERROR: no se pudo abrir la sesion SNMP.\n");
    }
    
    //Iniciamos los puerto DIO como salida
    configurarPuertosDIO();
    printf("INFO: Puertos entrada/salida configurados.\n");   
    
    //Creamos el thread que será el local de recepcion de comandos
    //y envio de respuestas
    int tRet;
    
    tRet = pthread_create(&localComandosThread, NULL, recComandosEnvResp, NULL);
    
    if(tRet != 0){
        perror("ERROR: No se pudo crear el thread de comandos. Saliendo...\n");
        salir(0);
    }

    //Obtener informacion del nodo
    //Obtenemos la direccion IP del dispositivo
    char ip_host[NI_MAXHOST];
    
    if((obtenerIPHost(ip_host, NI_MAXHOST)) != -1){
        int longitud_id = strlen(configuracion->id_nodo);
        informacion_nodo.id = malloc(longitud_id + 1);
        memset(informacion_nodo.id, 0, longitud_id + 1);
        strncpy(informacion_nodo.id, configuracion->id_nodo, longitud_id);
        informacion_nodo.ip = ip_host;
        informacion_nodo.lon_ip = strlen(informacion_nodo.ip);
    }

    //Creamos un thread para controlar acceso a las puertas.
    int tPuerta;
    
    tPuerta = pthread_create(&monPuertaThread, NULL, monitorPuerta, (void *)configuracion);
    
    if(tPuerta != 0){
        perror("ALERTA: No se pudo crear el thread de monitoreo de la puerta de acceso. Saliendo...\n");
    }
    
    //Ya que no tenemos un RTC, actualizamos la fecha silenciosamente
    if(system("ntpdate -s -u time.nist.gov")){
        printf("ERROR: No se pudo actualizar la fecha.\n");
    }
    else{
        printf("INFO: Se ha actualizado la fecha correctamente.\n");
    }
    
    printf("INFO: Direccion IP del monitor de nodo: %s\n", informacion_nodo.ip);
    printf("INFO: Se inicializo el servidor SNMP.\n");
    
    //Trap de inicio frio
    int j;
    for(j = 0; j < configuracion->numeroServidoresSNMP; j++){
        enviarTrap(ss[j], informacion_nodo.ip, SNMP_GENERICTRAP_COLDSTART, 0);
    }
    
    //Creamos la lista de mediciones 
    listaMediciones = inicializarListaMediciones(NUMERO_MEDICIONES_ADC);
    
    //Empezamos el monitoreo
    printf("INFO: Hora de inicializacion: %s\n", obtenerHora());
    printf("INFO: Fecha de inicializacion: %s\n", obtenerFecha());
    printf("INFO: Programa de monitoreo iniciado (PID %d).\n", processID);
    printf("INFO: Monitoreando...\n");
    
    //Enviamos un email notificando que el monitor se ha (re)iniciado
    char mensaje[TAMANO_MAX_RESPUESTA];
    char asunto[TAMANO_MAX_RESPUESTA];

    memset(mensaje, 0, TAMANO_MAX_RESPUESTA);
    memset(asunto, 0, TAMANO_MAX_RESPUESTA);
    
    snprintf(mensaje, TAMANO_MAX_RESPUESTA, "El monitor del nodo %s se ha iniciado el %s a las %s.\n", informacion_nodo.id, obtenerFecha(), obtenerHora());
    snprintf(asunto, TAMANO_MAX_RESPUESTA, "Inicializacion de monitor nodo %s", informacion_nodo.id);
    
    enviarMultiplesEmails(configuracion->destinatariosAlertas, configuracion->numeroDestinatariosAlertas,
                          asunto, "", mensaje);
    
    //*********PRUEBA ADC
    
        uint16_t* data_canal_2 = NULL;
        uint16_t* data_canal_1 = NULL;
        int noMuestras = NUMERO_MUESTRAS;
        int canal = 0;
        float voltaje1 = 0.0f;
        float voltaje2 = 0.0f;
        
        int tamanoBuffers = sizeof(uint16_t)*noMuestras;
        data_canal_1 = malloc(tamanoBuffers);
        data_canal_2 = malloc(tamanoBuffers);
        
    //********PRUEBA ADC
    //canal = 10;
    //Empezamos el monitoreo
    float temperaturaHumedad = 0.0f;
    while(1){
        sleep(configuracion->intervaloMonitoreo);  //damos tiempo que sensores se activen, etc.
        //realizarMediciones(listaMediciones);
        //revisarStatusMediciones(listaMediciones); //configuracion->valoresMinimosPermitidosMediciones, configuracion->numeroValoresMinimosPermitidos);
        //almacenarMediciones(listaMediciones, informacion_nodo.id, configuracion->rutaArchivoColumnasBDADC, NUMERO_MEDICIONES_ADC);
        
        
        //*******Prueba del ADC
        memset(data_canal_1, 0, tamanoBuffers);
        memset(data_canal_2, 0, tamanoBuffers);
        //if(canal == 2){
            obtenerMuestra(TARJETA_ADC24, canal, 2000, noMuestras, ADCRANGE_02VS, data_canal_1, data_canal_2);
        //}
        
        voltaje1 = promedioVoltaje(data_canal_1, noMuestras, ADCRANGE_02VS);
        voltaje2 = promedioVoltaje(data_canal_2, noMuestras, ADCRANGE_02VS);
        
        //printf("\nVoltaje canal %d: %.4f V\n", canal, voltaje1);
        
        
        switch(canal){
            //corriente DC 1 y temp 1
            case 0:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            //Corriente AC 3 y temp 2
            case 1:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 2:
                printf("\nVoltaje canal %d: %.4f V --> Corriente AC 3: %.1f A\n", canal, voltaje1, voltajeACorrienteAC(data_canal_1, noMuestras, ADCRANGE_02VS));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 2: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 3:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 4:
                //Corriente DC 2 y temp 3
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 2: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 3: %.1f C\n\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 5:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 6:
                //Corriente AC 4 Voltaje AC 1   
                printf("\nVoltaje canal %d: %.4f V --> Corriente AC 4: %.2f A\n", canal, voltaje1, voltajeACorrienteAC(data_canal_1, noMuestras, ADCRANGE_02VS));
                //printf("\nVoltaje canal %d: %.4f V --> Voltaje AC 1: %.2f V\n", canal+1, voltaje2, voltajeAVoltajeAC(voltaje2));
                break;
            case 7:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 8:
                //Corriente DC 3 y voltaje AC 2
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 3: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Voltaje AC 2: %.2f V\n", canal+1, voltaje2, voltajeAVoltajeAC(voltaje2));
                break;
            case 9:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 10:
                //10 nada y voltaje DC 1
                printf("\nVoltaje canal %d: %.4f V --> Voltaje DC 1: %.2f V\n", canal, voltaje1, voltajeACorrienteDC(voltaje2));
                break;
            case 11:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 12:
                //12 nada y voltaje DC 2
                printf("\nVoltaje canal %d: %.4f V --> Voltaje DC 2: %.2f V\n", canal, voltaje1, voltajeAVoltajeDC(voltaje2));
                break;
            case 13:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 14:
                //14 nada y voltaje DC 3
                printf("\nVoltaje canal %d: %.4f V --> Voltaje DC 3: %.2f V\n", canal, voltaje1, voltajeAVoltajeDC(voltaje2));
                break;
            case 15:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 16:
                //Corriente AC 1 y voltaje DC 4
                printf("\nVoltaje canal %d: %.4f V --> Corriente AC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteAC(data_canal_1, noMuestras, ADCRANGE_02VS));
                //printf("\nVoltaje canal %d: %.4f V --> Voltaje DC 4: %.2f V\n", canal+1, voltaje2, voltajeAVoltajeDC(voltaje2));
                break;
            case 17:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 18:
                //18 nada y humedad
                printf("\nVoltaje canal %d: %.4f V --> Humedad: %.2f %% HR\n", canal, voltaje1, voltajeAHumedad(voltaje2, temperaturaHumedad));
                break;
            case 19:
                temperaturaHumedad = voltajeATemperatura(voltaje2);
                printf("\nVoltaje canal %d: %.4f V --> Corriente DC 1: %.2f A\n", canal, voltaje1, voltajeACorrienteDC(voltaje1));
                //printf("\nVoltaje canal %d: %.4f V --> Temperatura 1: %.1f C\n", canal+1, voltaje2, voltajeATemperatura(voltaje2));
                break;
            case 20:
                //20 Corriente AC 2
                printf("\nVoltaje canal %d: %.4f V --> Corriente AC 2: %.2f A\n", canal, voltaje1, voltajeACorrienteAC(data_canal_1, noMuestras, ADCRANGE_02VS));
                break;
            default:
                break;
        }
        
        
        //printf("\nVoltaje canal %d: %.4f V\nVoltaje canal %d: %.4f V\n", canal, voltaje1, canal + 1, voltaje2);
        
        if(canal == 20){         //14     
            canal = 0;
        }
        else canal += 1;
       
    
        //FIN prueba ADC
        //desactivarPuerto(puerto_DIO_0);
        
        //Prueba DIO - salida
        if(statusPuerto(puerto_DIO_7) == PUERTO_ON){
            desactivarPuerto(puerto_DIO_7);
            desactivarPuerto(puerto_DIO_6);
            desactivarPuerto(puerto_DIO_5);
            desactivarPuerto(puerto_DIO_4);
            printf("Puerto OFF\n");
        }
        else if(statusPuerto(puerto_DIO_7) == PUERTO_OFF){
            activarPuerto(puerto_DIO_7);
            activarPuerto(puerto_DIO_6);
            activarPuerto(puerto_DIO_5);
            activarPuerto(puerto_DIO_4);
            printf("Puerto ON\n");
        }
        
        //Prueba puerta
        if(statusPuerto(puerto_DIO_0) == PUERTO_ON){
            //desactivarPuerto(puerto_DIO_2);
            printf("Puerto puerta: ON\n");
        }
        else if(statusPuerto(puerto_DIO_0) == PUERTO_OFF){
            //activarPuerto(puerto_DIO_2);
            printf("Puerto puerta: OFF\n");
        }   
    }
    
    return 0;    
}

