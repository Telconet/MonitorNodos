#include "monitordef.h"

//#define DEBUG


/**
 *Salir. Limpiamos todos los recursos
 */
void salir(int status){
    
    printf("INFO: El daemon de monitoreo se esta cerrando...\n");
    
    //cerramos el socket
    close(sd1);
    close(sd2);
    
    //Removemos el archivo con el pid    
    char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
    
    strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
    strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
    system(comandoRemover);

    exit(status);
}


/**
 *Rutina que se encarga de monitorear exclusivamente la puerta de acceso al nodo
 *El puerto de entrada esta por defaul activa high (1 logico = 3.3V)
 *0V   --> PUERTA CERRADA     (con circuito pull-up)
 *3.3V --> PUERTA ABIERTA
 */
void monitorPuerta(void *sd){
    
    struct configuracionMonitor *conf = (struct configuracionMonitor *)sd;
    int intervaloMonitoreo = conf->intervaloMonitoreoPuerta;
    status_medicion puertaAbiertaUltMed;
    int j = 0;
    status_puerto_DIO sensorPuerta;
    
    //Medimos el sensor de apertura de puerta   
    sensorPuerta = statusPuerto(puerto_DIO_4);          //puerto puerta es DIO_4
    
    char *hora = obtenerHora();
    char *fecha = obtenerFecha();
       
    //Hacemos una primera medicion, para saber el status real de la puerta
    if(sensorPuerta == PUERTO_OFF){        
        //La puerta esta cerrada
        puertaAbiertaUltMed = CERRADO;
        printf("INFO: La puerta de acceso al nodo esta siendo monitoreada. Puerta cerrada.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "puerta cerrada");
        free(fecha);
        free(hora);
        
    }
    else if(sensorPuerta == PUERTO_ON){
        //La puerta estaba abierta
        puertaAbiertaUltMed = ABIERTO;
        printf("INFO: La puerta de acceso al nodo esta siendo monitoreada. Puerta abierta.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "puerta abierta");
        free(fecha);
        free(hora);
    }

    //Empezamos a monitorear la puerta regularmente.
    while(1){
        
        j = 0;
        
        //TODO chequear el sensor de la puerta
        sensorPuerta = statusPuerto(puerto_DIO_4);
        
        //Puerta abierta
        if(sensorPuerta == PUERTO_OFF && puertaAbiertaUltMed == CERRADO){        
            //            
        }
        else if(sensorPuerta == PUERTO_OFF && puertaAbiertaUltMed == ABIERTO){
            
            //La puerta estaba abierta, y fue cerrada
            puertaAbiertaUltMed = CERRADO;
            
            printf("INFO: La puerta de acceso al nodo ha sido cerrada.\n");
            
            
            char *hora = obtenerHora();
            char *fecha = obtenerFecha();
            
            //Mandamos notificacion al servidor
            insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "puerta cerrada");
            
            free(fecha);
            free(hora);
        

        }
        else if(sensorPuerta == PUERTO_ON && puertaAbiertaUltMed == CERRADO){
            //La puerta estaba cerrada y fue abierta.
            puertaAbiertaUltMed = ABIERTO;
            
            printf("INFO: La puerta de acceso al nodo ha sido abierta.\n");
            
            char *hora = obtenerHora();
            char *fecha = obtenerFecha();
            
            //Mandamos notificacion al servidor
            insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "puerta abierta");
            
            free(fecha);
            free(hora);            
        }
        else if(sensorPuerta == PUERTO_ON && puertaAbiertaUltMed == ABIERTO){
            /*NOP*/
        }
        usleep(intervaloMonitoreo*1000);        //convertir usecs a msecs
    }
}


/**
 *Monitor Aires Acondicionados
 *Usamos el mismo intervalo de monitoreo de puerta (2 secs default)
 *
 *AC Principal DIO_5
 *AC Backup    DIO_6
 *
 *PUERTO_OFF --> AIRE APAGADO
 *PUERTO_ON --> AIRE ENCENDIDO
 *
 *DIO_0 controla AC_Principal
 *DIO_1 controla AC_backup
 *(sujeto a cambio)
 *
 *PUERTO_OFF (0), aire prendido 
 */
void monitorAiresAcondicionados(void *sd){
    
    struct configuracionMonitor *conf = (struct configuracionMonitor *)sd;
    int intervaloMonitoreo = conf->intervaloMonitoreoPuerta;
    status_medicion aaCCPrincipalUltMed, aaCCBackupUltMed;
    int j = 0;
    float temperatura = 0.0f;
    status_puerto_DIO sensorAACCPrincipal;
    status_puerto_DIO sensorAACCBackup;
    
    if(conf->temperaturaCritica < 2.0f){
        conf->temperaturaCritica = 27.0f;
    }
    
    
    //Encendemos el aire principal y apagamos el backup (DIO_0 es principal, DIO_1 es backup)
                                                        //(o DIO_2 principal, DIO_3 backup)
    activarPuerto(configuracion->puertoDIO_ACPrincipal);    //Principal
    desactivarPuerto(configuracion->puertoDIO_ACBackup); //Backup
    sleep(2);
    
    //Medimos el sensor del AC Pinr  
    sensorAACCPrincipal = statusPuerto(puerto_DIO_5);
    sensorAACCBackup = statusPuerto(puerto_DIO_6);
    
    pthread_mutex_lock(&mutex_status_AACC);
    status_A_C_principal = sensorAACCPrincipal;
    status_A_C_backup = sensorAACCBackup;
    pthread_mutex_unlock(&mutex_status_AACC);
    
    char *hora = obtenerHora();
    char *fecha = obtenerFecha();
    
    printf("INFO: Verificando status inicial de los aires acondicionados (%d).\n", sensorAACCPrincipal);
       
    //Hacemos una primera medicion, para saber el status real de la puerta
    //Principal
    if(sensorAACCPrincipal == PUERTO_OFF){        
        aaCCPrincipalUltMed = APAGADO;
        printf("INFO: Aire acondicionado principal apagado.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "Aire acondicionado principal apagado");     
    }
    else if(sensorAACCPrincipal == PUERTO_ON){
        aaCCPrincipalUltMed = ENCENDIDO;
        printf("INFO: Aire acondicionado principal encendido.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "Aire acondicionado principal encendido");
    }
    
    //Backup
    if(sensorAACCBackup == PUERTO_OFF){        
        //La puerta esta cerrada
        aaCCBackupUltMed = APAGADO;
        printf("INFO: Aire acondicionado backup apagado.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "Aire acondicionado backup apagado");      

    }
    else if(sensorAACCBackup == PUERTO_ON){
        aaCCBackupUltMed = ENCENDIDO;
        printf("INFO: Aire acondicionado backup encendido.\n");
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "Aire acondicionado backup encendido");
    }
    
    free(fecha);
    free(hora);
    
    //Buscamos puntero a medicion de temperatura
    volatile struct medicion *medicionTemperatura_ptr = listaMediciones;

    //Temperatura es CANAL 1, por lo tanto, vemos el siguiente elemento
    //en la lista.
    medicionTemperatura_ptr = medicionTemperatura_ptr->siguiente;
    
    //Damos 5 minutos hasta que se estabilice temperatura.
    sleep(10);          //segundos...
        
    //Empezamos a monitorear el A/C
    while(1){
        
        j = 0;
    
        char temperaturaStr[100];
        
        //Leer temperatura...
	pthread_mutex_lock(&mutexTemperatura);
        temperatura = medicionTemperatura_ptr->valor;
        pthread_mutex_unlock(&mutexTemperatura);
        
        printf("Temp: %.2f\n", temperatura);
        
	if (temperatura > conf->temperaturaCritica){
            
            hora = obtenerHora();
            fecha = obtenerFecha();
            
            //Conmutamos
	    activarPuerto(configuracion->puertoDIO_ACBackup);   //Activar el AACC backup 
            desactivarPuerto(configuracion->puertoDIO_ACPrincipal);   //Apagamos el principal   
            
            sleep(3);       //dar tiempo a reles para conmutar.
            
            sensorAACCPrincipal = statusPuerto(puerto_DIO_5);
            sensorAACCBackup = statusPuerto(puerto_DIO_6);
            
            pthread_mutex_lock(&mutex_status_AACC);
            status_A_C_principal = sensorAACCPrincipal;
            status_A_C_backup = sensorAACCBackup;
            pthread_mutex_unlock(&mutex_status_AACC);
            
            //Cuando cambiamos de estado notificamos
            if(sensorAACCPrincipal == PUERTO_OFF){          //solo guardamos si A/C principal esta apagado. Caso contrario prendimos el A/C backup por temperatura.
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "Aire acondicionado principal apagado. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
                //printf("%s\n", temperaturaStr);
            }
            else{
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "Aire acondicionado principal encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
                //printf("AACC principal prendido\n");
            }
            
            
            if(sensorAACCBackup == PUERTO_ON){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "Aire acondicionado backup encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
                //printf("%s\n", temperaturaStr);
            }
            else{
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "Aire acondicionado backup apagado. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
                //printf("AACC backup apagado\n");
            }
            
            aaCCPrincipalUltMed = APAGADO;
            aaCCBackupUltMed = ENCENDIDO;
            free(fecha);
            free(hora);
	}
	
        usleep(intervaloMonitoreo*1000);        //convertir usecs a msecs
    }
}


/**
 *Manejador de la señales de terminacion del daemon.
 *
 */
void manejadorSenalSIGTERMSIGINT(int sig)
{    
    //Tipo de señales
    if(sig == SIGINT)
    {
        printf("Catched SIGINT signal\n");
    }
    else if(sig == SIGTERM){
        printf("Cerrando el daemon...\n"); 
    }
    
    salir(0);
}

/**
 *Rutina que lee el archivo de configuracion
 */
struct configuracionMonitor* leerArchivoConfiguracion(char *rutaArchivo){
    
    FILE *archivo = fopen(rutaArchivo, "r");
    int longitudLinea = 512;
    
    if(archivo != NULL){
        printf("INFO: Ruta de archivo de configuracion: %s\n", rutaArchivo);
        
        struct configuracionMonitor *configuracion = malloc(sizeof(struct configuracionMonitor));
        char linea[longitudLinea];               //--CHECK
        
        memset(configuracion, 0, sizeof(struct configuracionMonitor));      
        
        //Asignamos la ruta del archivo...
        configuracion->rutaArchivoConfiguracion = rutaArchivo;      //CHECK
        
        while(1){
            
            int num;
            num = 0;
            char **valores;
            
            //ponemos en cero el buffer
            memset(linea, 0, longitudLinea);            //--CHECK
            
            if(fgets(linea, longitudLinea, archivo) == NULL){   //--CHECK
                return configuracion;
            }
                        
            //Ignoramos las lineas que contienen el caractar '#', que es de comentario
            if(strstr(linea,"#") == NULL){
                
                if(strstr(linea, ID_NODO) != NULL){
                    //Parametro de ID del nodo
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->id_nodo = valores[0];
                        printf("INFO: ID del nodo: %s\n",configuracion->id_nodo);
                        
                    }
                    else return NULL;
                }
                else if(strstr(linea, INTERVALO_MON_PUERTA) != NULL){
                    //Intervalo de monitoreo de la puerta
                    valores = obtenerValorConfig(linea, &num);
                    
                    if(valores != NULL){
                        int intervalo = atoi(valores[0]);
                        
                        if(intervalo == 0){
                            configuracion->intervaloMonitoreoPuerta = INTERVALO_MON_PUERTA_PRED;    //valor predeterminado
                        }
                        else{
                            configuracion->intervaloMonitoreoPuerta = intervalo;    //valor predeterminado
                        }
                        
                        printf("INFO: Intervalo de monitoreo de puerta de entrada: %.2f segundo(s)\n",((double)(configuracion->intervaloMonitoreoPuerta) / 1000.0f));      //intervalo en millis
                    }
                    else return NULL;
                }
                else if(strstr(linea, INTERVALO_MON) != NULL){
                    //Intervalo de monitoreo 
                    valores = obtenerValorConfig(linea, &num);
                    
                    if(valores != NULL){
                        int intervalo = atoi(valores[0]);
                        
                        if(valores[0] == 0){
                            configuracion->intervaloMonitoreo = INTERVALO_MON_PRED;    //valor predeterminado
                        }
                        else{
                            configuracion->intervaloMonitoreo = intervalo;              //valor predeterminado
                        }
                        printf("INFO: Intervalo de monitoreo de status del nodo: %.2f minuto(s)\n", ((float)(configuracion->intervaloMonitoreo))/((float)(SEGUNDOS_POR_MINUTO)));
                        
                    }
                    else return NULL;
                }
                else if(strstr(linea, ARCHIVO_COL_BD_ADC) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->rutaArchivoColumnasBDADC = valores[0];
                        printf("INFO: Archivo de columnas de la base de datos (ADC): %s\n", configuracion->rutaArchivoColumnasBDADC);
                    }
                    else return NULL;
                }
                else if(strstr(linea, ARCHIVO_COL_BD_DIO) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->rutaArchivoColumnasBDDIO = valores[0];
                        printf("INFO: Archivo de columnas de la base de datos (DIO): %s\n", configuracion->rutaArchivoColumnasBDDIO);
                    }
                    else return NULL;
                }
                else if(strstr(linea, RAZON_CT)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->razonCT = atof(valores[0]);
                        printf("INFO: Razon de transformador de corriente: %.0f:1.\n", configuracion->razonCT);
                    }
                    else return NULL;
                }
                else if(strstr(linea, IP_SERVIDOR_DATOS)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->ip_servidor_datos = valores[0];
                        printf("INFO: IP del servidor de almacenamiento de datos: %s.\n", configuracion->ip_servidor_datos);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MONITOREO_AIRES)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        if(strstr(valores[0], "si") != NULL){
                            configuracion->monitoreoAires = 1;
                            printf("INFO: Monitoreo de aires acondicionados activado\n");
                        }
                        else{
                            configuracion->monitoreoAires = 0;
                            printf("INFO: Monitoreo de aires acondicionados desactivado\n");
                        }
                    }
                    else return NULL;
                }
                else if(strstr(linea, TEMPERATURA_CRITICA)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->temperaturaCritica = atof(valores[0]);
                        printf("INFO: Temperatura critica: %.0f C.\n", configuracion->temperaturaCritica);
                    }
                    else return NULL;
                }
                else if(strstr(linea, PUERTO_DIO_AC_PRINCIPAL) != NULL){
                    //DIO AC Princiapl
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        int puerto = atoi(valores[0]);
                          
                        if(puerto == 0 || puerto > 3){
                            configuracion->puertoDIO_ACPrincipal = puerto_DIO_0;    //valor predeterminado
                        }
                        else{
                            configuracion->puertoDIO_ACPrincipal = puerto;    //valor predeterminado
                        }
                        printf("INFO: Puerto DIO del A/C principal: %d\n", configuracion->puertoDIO_ACPrincipal);
                    }
                    else return NULL;
                }
                else if(strstr(linea, PUERTO_DIO_AC_BACKUP) != NULL){
                    //DIO AC backup
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        int puerto = atoi(valores[0]);
                          
                        if(puerto == 0 || puerto > 3){
                            configuracion->puertoDIO_ACBackup = puerto_DIO_1;    //valor predeterminado
                        }
                        else{
                            configuracion->puertoDIO_ACBackup = puerto;    //valor predeterminado
                        }
                        printf("INFO: Puerto DIO del A/C backup: %d\n", configuracion->puertoDIO_ACBackup);
                    }
                    else return NULL; 
                }
            }
        }
        fclose(archivo);
    }
    return NULL;
}

/**
 *Obtiene el valor de un parametro de configuracion. El parametro que
 *se obtiene es el valor despues del simbolo '='
 */
char **obtenerValorConfig(char *linea, int *numeroValores){
    
    int long_linea = strlen(linea);
    int numero_caracteres = 0;
    int i = 0;
    
    while(*linea != '='){
        linea++;                            //avanzamos el puntero hasta el simbolo igual
        i++;
        
        if(i >= long_linea) return NULL;    //no queremos sobrepasarnos si nos mandan un string mal formateado
    }
    
    linea++;                                //nos movemos pasando el simbolo =
    
    while(*linea == ' ' || *linea == '\t'){ //pasamos cualquier espacio entre el valor y el simbolo igual
        linea++;
        i++;
        if(i >= long_linea) return NULL;    //no queremos sobrepasarnos si nos mandan un string mal formateado
    }
    
    //Contamos cuanto puntos y comas hay para saber el numero de valores
    char *ptr;
    ptr = linea;
    int numeroPuntoYComas = 0;
    int w = i;
    
    while(*ptr != '\n'){
        
        if(*ptr == ';'){
            numeroPuntoYComas++;
        }
        ptr++;
        w++;
        if(w >= long_linea) return NULL;
    }

    //Asignamos memoria para el arreglo y notificamos cuantos valores serán extraidos
    char **buffer = malloc((numeroPuntoYComas + 1)*(sizeof(*buffer)));
    *numeroValores = numeroPuntoYComas + 1;
    
    //En este punto ya tenemos la direccion del primer caracter del valor
    //Ahora contamos la longitud de caracteres del valor, para saber la cantidad
    //memoria que debemos asignar
    int j = 0; 
    for(j = 0; j < *numeroValores ; j++){
        char *inicio_valor = linea;
        numero_caracteres = 0;
        
        while(*linea != '\n'){
            linea++;
            numero_caracteres++;
            if(*linea == ';'){
                linea++;
                break;
            }
            i++;
            if(i >= long_linea) return NULL;    //no queremos sobrepasarnos si nos mandan un string mal formateado
        }
#ifdef DEBUG   
        printf("Numero caracteres %d\n", numero_caracteres);
#endif
        
        //numero_caracteres++;            //para el caracter nulo
        buffer[j] = malloc((numero_caracteres + 1)*sizeof(char));
        
        if(buffer == NULL){
            return NULL;            
        }
        
        memset(buffer[j], 0, numero_caracteres);
        
        //copiamos los charateres al buffer
        strncpy(buffer[j], inicio_valor, numero_caracteres);
        buffer[j][numero_caracteres] = '\0';               //???
        
#ifdef DEBUG
        printf("Valor leido: %s\n", buffer[j]);
#endif
        
    }
    return buffer;
}




/**
 *Funcion que manejara las solicitudes MODBUS
 */
void monitorModbus(void *sd){
    
    modbus_t *contexto_modbus = (modbus_t *)sd;
    
    if(sd == NULL){
        perror("ERROR: No se recibio el contexto modbus. Se deshabilito MODBUS.\n");
        pthread_exit((void *)-1);
    }
    
    //Solicitud recibida
    uint8_t solicitud[MODBUS_TCP_MAX_ADU_LENGTH];
    
    //Empezamos a oir el puerto serio. Para este instante ya debemos tener
    //el contexto y mapa de registros listo.
    while(true){
        perror("Esperando por solicitudes...\n");
        memset(solicitud, 0, MODBUS_TCP_MAX_ADU_LENGTH);
        
        int rc = modbus_receive(contexto_modbus, solicitud);                //esperamos comunicaci'on
        
        printf("rc: %d\n",rc);
        
        if(rc > 0){
             pthread_mutex_lock(&mutexModbus);
        
            int i = 0;
            char buffer[2*MODBUS_TCP_MAX_ADU_LENGTH + 1];
            memset(buffer, 0, 2*MODBUS_TCP_MAX_ADU_LENGTH + 1);
            
            printf("Solicitud: ");
            for(i=0 ; i < rc; i++){
                printf("%02X", solicitud[i]);
            }
            printf("\n");

            printf("INFO: Respondiendo solicitud MODBUS... %s\n", buffer);
            int err = modbus_reply(contexto_modbus, solicitud, rc, mapeo_modbus);    //MODBUS responde a la solicitud
            printf("err = %d\n",err);
            if(err < 0)
                perror("Error al responder solicitud");
            
            pthread_mutex_unlock(&mutexModbus);
        }
        else{
            modbus_close(contexto_modbus);
            perror("Error al recibir solicitud modbus.\n");
            modbus_connect(contexto_modbus);        //si hay error, reconectamos..
            
            //Setear nuevamente para RS485 HD o FD..
            //TODO...
            
            /*int fd  = modbus_get_socket(contexto_modbus);
    
            int mcr = AUTO485HD;
            ioctl(fd, TIOC_SBCS485, &mcr);*/

        }
    }
}

