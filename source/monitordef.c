#include "monitordef.h"


/**
 *Salir. Limpiamos todos los recursos
 */
void salir(int status){
    
    printf("INFO: El daemon de monitoreo se esta cerrando...\n");
    
    //cerramos el socket
    close(sd1);
    close(sd2);
    
    //Cerramos el sistema SNMP
    /*if(status != EXIT_FAILURE){
        int i;
        if(ss != NULL){
            for(i = 0; i < configuracion->numeroServidoresSNMP; i++){
                if(ss[i] != NULL){
                    cerrarSistemaSnmp(ss[i]);
                }
            }
        }
    }*/
    
    //Removemos el archivo con el pid    
    char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
    
    strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
    strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
    system(comandoRemover);

    exit(status);
}



/**
 *Esta rutina manejara las peticiones del controlador.
 *El thread principal creará worker threads para
 *manejar las peticiones de cada controlador.
 */
void manejarComandosControlador(void *sd){
    
    char mensaje[MAX_BUFFER_SIZE];
    int bytes_recibidos = -1;                        //????
    int *sd_intptr = (int *)sd;                    //el file descriptor, convertimos a int *
    int sid2 = *sd_intptr;                           //obtenemos el valor apuntado por el puntero
    char pidControladorStr[6];
    int pidControlador = 0;
        
    //Inicializamos el buffer
    memset(mensaje, 0, MAX_BUFFER_SIZE);
    
    //recibimos el PID del controlador
    bytes_recibidos = recv(sid2, pidControladorStr, 6, 0);
    
    if(bytes_recibidos > 0){
        pidControlador = atoi(pidControladorStr);
        
        printf("INFO: PID del controlador conectado es %d\n", pidControlador);

    }
    else printf("Error al recibir PID\n");
    
    bytes_recibidos = -1;
    
    //Empezamos a recibir los mensajes  ---> check!!!!!
    do{
        int comando;
        
        bytes_recibidos = recv(sid2, &comando, sizeof(comando), 0);  
        
        //Error al recibir
        if(bytes_recibidos < 0){
#ifdef DEBUG
            perror("Error en el mensaje ");
#endif
            return;
        }
        else if(bytes_recibidos > 0){
            
            if(comando == COMANDO){                     //Poner en rutina recibir_datos
                
                struct comando com;
                
                //Si no hubo errores, procesamos el comando
                if(recibirDatos(sid2, &com, COMANDO) == 0){            //Agregar n =??
                                        
                    
                    procesarComando(sid2, &com);                              //Realizamos acciones pedidas
                    
                    //liberar memoria de los argumentos del comando
                    int indiceComando = 0;
                    for(indiceComando = 0; indiceComando < com.numero_argumentos; indiceComando++){
                        free(com.argumentos[indiceComando]);
                    }

                }
                
            }
        }
        else{
            //Aqui se entra cuando el controlador es cerrado.
            close(sid2);
#ifdef DEBUG
            printf("El thread %u ha finalizado.\n", (unsigned int)pthread_self());
#endif
        }
        
    }while(bytes_recibidos > 0);
    
    return;
}

/**
 *Rutina que se encarga de llevar el control de cuando se deben enviar las alertas.
 */
void temporizadorEnvioEmails(void *sd){
    
    struct configuracionMonitor *conf = (struct configuracionMonitor *)sd;
    int tiempoEmails = conf->periodoEnvioEmails;
    
    int periodoSleep = 60;          //segundos
    
    //dormimos 1 minuto a la vez.
    //la primera vez, dejamos pasar 3 minutos, para dar tiempo a la tarjeta que se inicialice
    //y haga las primeras mediciones
    sleep(3*periodoSleep);
    
    int i;
    //volatile struct medicion *listaMedicionesTemporal = listaMediciones;
    
    //Entramos al lazo
    while(1){
        
        pthread_mutex_lock(&mutexEmailsAlerta);
        for(i = 0; i < NUMERO_MEDICIONES_ADC; i++){
            
            //revisarStatusMediciones notifica si envió e-mail. Si es positivo, este hilo
            //lleva control de cuanto tiempo ha pasado desde el ultimo e-mail.
            if(alertaEmailEnviada[i]){
                
                if(alertaEmailEnviada[i] && tiempoDesdeUltimaAlerta[i] < tiempoEmails ){            //Verficamos que dicho canal haya enviado alerta y que el tiempo sea menor de 30 minutos
                    //alertaEmailEnviada[i] = 0;
                    tiempoDesdeUltimaAlerta[i] += 1;
#ifdef DEBUG
                    printf("Tiempo desde ultima alerta de %d: %d\n", i, tiempoDesdeUltimaAlerta[i]);
#endif
                }
                else if(alertaEmailEnviada[i] && tiempoDesdeUltimaAlerta[i] == tiempoEmails){
                    alertaEmailEnviada[i] = 0;                            //damos nuevo permiso para mandar e-mails
                    tiempoDesdeUltimaAlerta[i] = 0;
                }
            }
        }
        pthread_mutex_unlock(&mutexEmailsAlerta);

        sleep(periodoSleep);
    }
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
            //
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
 */
void monitorAiresAcondicionados(void *sd){
    
    struct configuracionMonitor *conf = (struct configuracionMonitor *)sd;
    int intervaloMonitoreo = conf->intervaloMonitoreoPuerta;
    status_medicion aaCCPrincipalUltMed, aaCCBackupUltMed;
    int j = 0;
    float temperatura = 0.0f;
    status_puerto_DIO sensorAACCPrincipal;
    status_puerto_DIO sensorAACCBackup;
    
    //Medimos el sensor del AC Pinr  
    sensorAACCPrincipal = statusPuerto(puerto_DIO_5);
    sensorAACCBackup = statusPuerto(puerto_DIO_6);
    
    char *hora = obtenerHora();
    char *fecha = obtenerFecha();
    
    printf("INFO: Verificando status inicial de los aires acondicionados (%d).\n", sensorAACCPrincipal);
       
    //Hacemos una primera medicion, para saber el status real de la puerta
    //Principal
    if(sensorAACCPrincipal == PUERTO_OFF){        
        //La puerta esta cerrada
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
        insertarEvento(atoi(configuracion->id_nodo), fecha, hora, "Aire acondicionado apagado");      

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
        
    //Empezamos a monitorear el A/C
    while(1){
        
        j = 0;
        
        //Chequeamos AC principal
        sensorAACCPrincipal = statusPuerto(puerto_DIO_5);
        sensorAACCBackup = statusPuerto(puerto_DIO_6);
       
        /*
	Código donde se implementa el control automático de la temperatura de los nodos movistar. Se logra lo siguiente:
	
	1) Que cuando la temperatura sea mayor a 30º o si el aire acondicionado principal se apaga, se prenda el aire
	   acondicionado de back up
	2) Que cuando esté prendido el aire acondicionado principal y la temperatura sea mayor a 30º, se prenda el
           aire acondicionado de back up
        3) Que cuando esté funcionando el aire acondicionado principal y la temperatura sea menor que 30º, se apague
	   el aire acondicionado de back up
	*/
        char temperaturaStr[100];

        
        //Leer temperatura...
	pthread_mutex_lock(&mutexTemperatura);
        temperatura = medicionTemperatura_ptr->valor;
        pthread_mutex_unlock(&mutexTemperatura);
        
	//Empieza código de Control Automático de la temperatura
	
	if (sensorAACCPrincipal == PUERTO_OFF || temperatura > 27.0){
            
            hora = obtenerHora();
            fecha = obtenerFecha();
            
            //Cuando cambiamos de estado notificamos
            if(aaCCPrincipalUltMed == ENCENDIDO && sensorAACCPrincipal == PUERTO_OFF){          //solo guardamos si A/C principal esta apagado. Caso contrario prendimos el A/C backup por temperatura.
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C principal apagado. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
            
            if(aaCCBackupUltMed == APAGADO){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C backup encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
	    activarPuerto(puerto_DIO_1);   //Activar el secundario
            aaCCPrincipalUltMed = APAGADO;
            aaCCBackupUltMed = ENCENDIDO;
            free(fecha);
            free(hora);
	}
	else if (sensorAACCPrincipal == PUERTO_ON && temperatura > 27.0){
            
            hora = obtenerHora();
            fecha = obtenerFecha();
	    activarPuerto(puerto_DIO_1);
            
            //Notificamos cambio de estado de los aires acondicionados.
            if(aaCCPrincipalUltMed == APAGADO){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C principal encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
            if(aaCCBackupUltMed == APAGADO){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C backup encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
            aaCCPrincipalUltMed = ENCENDIDO;
            aaCCBackupUltMed = ENCENDIDO;
            free(fecha);
            free(hora);
	}
	else if (sensorAACCPrincipal == PUERTO_ON && temperatura <= 27.0){
            
            hora = obtenerHora();
            fecha = obtenerFecha();
            desactivarPuerto(puerto_DIO_1);
            
            //Notificamos solo si hubo un cambio respecto a la medicion anterior
            if(aaCCPrincipalUltMed == APAGADO){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C principal encendido. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
            if(aaCCBackupUltMed == ENCENDIDO){
                memset(temperaturaStr, 0, 100);
                snprintf(temperaturaStr, 100, "A/C backup apagado. Temperatura: %.2f C", temperatura);
                insertarEvento(atoi(configuracion->id_nodo), fecha, hora, temperaturaStr);
            }
            
            //Cambiamos ultimo estado
            aaCCPrincipalUltMed = ENCENDIDO;
            aaCCBackupUltMed = APAGADO;
            free(fecha);
            free(hora);
	}
	//Termina código de Control Automático de la temperatura
        
        usleep(intervaloMonitoreo*1000);        //convertir usecs a msecs
    }
}


/**
 *Funcion que manejara las solicitudes MODBUS
 */
void monitorModbus(void *sd){
    
    struct configuracionMonitor *conf = (struct configuracionMonitor *)sd;
    //int intervaloMonitoreo = conf->intervaloMonitoreoPuerta;
    //int j = 0;
    
    
    //TODO: obtener configuracion MODBUS del archivo de configuracion
}
        
        

/**
 *Funcion que manejara el envio y recepcion de comandos
 *mediante Unix sockets. Correrá en un thread individual.
 */
void recComandosEnvResp(void *ptr)
{
    struct sockaddr_un local, remoto;
    int tamano_local, tamano_remoto;
    int tRet;
    pthread_t peticiones_thread;

    //Obtenemos un socket
    sd1 = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(sd1 == -1){
        perror("socket");
        
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(1);
    }
    
    //asociamos el socket descriptor 1 a el archivo
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, ARCHIVO_SOCKET_IPC);
    unlink(local.sun_path);
    
    tamano_local = strlen(local.sun_path) + sizeof(local.sun_family); 
    
    if( bind(sd1, (struct sockaddr *) &local, tamano_local) == -1)
    {
        perror("bind");
        
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(1);
    }
    
    //Hacemos el archivo accesible a otros usuarios, para que se puedan comunicar con nosotros. CAMBIO POR VERIFICAR
    int res2 = chmod(ARCHIVO_SOCKET_IPC, S_IRUSR | S_IWUSR | S_IXUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
    
    if(res2 < 0){
        perror("ERROR: ");
    }
    
    //empezamos a escuchar las conexion
    if( listen(sd1, NO_MAX_CONEX) == -1){
        perror("listen");
        exit(1);
    }
      
    //Aceptamos la conexiones entrantes, y generamos un thread que
    //maneje la peticion
    while(1){
        
        //aceptamos una conexion
        tamano_remoto = sizeof(remoto);
        sd2 = accept(sd1, (struct sockaddr *) &remoto, &tamano_remoto);

        //Si hay error salimos
        if(sd2 == -1){
            perror("accept");
            exit(0);
        }
        else printf("INFO: Conectado a un controlador.\n");
        
        //Aqui manejamos las peticiones de los controladores
        //en un nuevo thread    
        tRet = pthread_create(&peticiones_thread, NULL, (void *)manejarComandosControlador, &sd2);
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
 *Procesar comandos. Esta rutina tomara un comando con sus argumentos, y realiza
 *la acción pertinente
 */
int procesarComando(int fd, struct comando *com)
{
    if(com == NULL || fd < 0)
    {
        printf("INFO: Se recibio un comando invalido\n");
        return -1;
    }
    else {
        printf("INFO: Se recibio un comando valido\n");
    }
    
    //printf("comando: %d\n", com->com);
    
    struct respuesta res;
    int lon = 0;
    char *buffer = NULL;
    int longitudMinimaBuffer = 1024;                //--CHECK antes 8192
    
    //Vemos que tipo de comando es
    switch (com->com){
        case SALIR_DAEMON:
            
            //Creamos la respuesta
            res.status = OK;
            res.res = "El monitor de nodos ha sido cerrado.";
            res.long_res = strlen(res.res);
            enviarDatos(fd, &res, RESPUESTA);
            salir(0);
            break;
        case INFORMACION_NODO:
            res.status = OK;
            lon = longitudMinimaBuffer*sizeof(char);
            buffer = malloc(lon);              //CHECK
            if(buffer != NULL){
                memset(buffer, 0, lon);
                sprintf(buffer, "Nombre del nodo: %s\nDireccion IP del monitor: %s", informacion_nodo.id, informacion_nodo.ip);
                res.long_res = strlen(buffer);
                res.res = buffer;
                enviarDatos(fd, &res, RESPUESTA);
                free(buffer);
            }
            break;
        case CAMBIAR_MINIMOS:
            res.status = OK;
            lon = longitudMinimaBuffer*sizeof(char);
            buffer = malloc(lon);              //CHECK
            if(buffer != NULL){
                memset(buffer, 0, lon);
                
                //Creamos el string del comando
                char *comandoCambiarMinimosStr = malloc(longitudMinimaBuffer*sizeof(char));
                if(comandoCambiarMinimosStr != NULL){
                    memset(comandoCambiarMinimosStr, 0, longitudMinimaBuffer);
                    strncat(comandoCambiarMinimosStr, "sed -i \"/valores-min/cvalores-min-permitidos = ", longitudMinimaBuffer);
                    strncat(comandoCambiarMinimosStr, com->argumentos[0], longitudMinimaBuffer);
                    strncat(comandoCambiarMinimosStr, "\" ", longitudMinimaBuffer);
                    strncat(comandoCambiarMinimosStr, configuracion->rutaArchivoConfiguracion, longitudMinimaBuffer);
                    
                    //printf("%s\n", comandoCambiarMinimosStr);
                }
                else return -1;
                
                int resultado = system(comandoCambiarMinimosStr);
                
                if(!resultado){
                    res.status = OK;
                    
                    struct configuracionMonitor* configuracionNueva;
                    configuracionNueva = leerArchivoConfiguracion(strdup(configuracion->rutaArchivoConfiguracion));
                    
                    //Puntero temporal a los minimos anteriores...
                     char ** minimosAnteriores = configuracion->valoresMinimosPermitidosMediciones;
                    
                    //Cambiamos a los nuevos minimos
                    configuracion->valoresMinimosPermitidosMediciones = configuracionNueva->valoresMinimosPermitidosMediciones;
                    convertirMinimosAfloat();
                    
                    //Liberamos la memoria ocupada por los minimos anteriores.
                    int indiceMinimosPer = 0;
                    for(indiceMinimosPer = 0; indiceMinimosPer < configuracion->numeroValoresMinimosPermitidos; indiceMinimosPer++){
                        free(minimosAnteriores[indiceMinimosPer]);
                    }
                    
                    
                    
                    
                    sprintf(buffer, "Minimos cambiados exitosamente.");
                    
                    //TODO: copiar nuevos minimos...
                    
                    //volvemos a cargar el archivo de configuración
                    /*struct configuracionMonitor* configuracionNueva = configuracion;  //Mantenemos un puntero temporal
                    
                    //Cargamos una nueva configuracion, duplicando el string de la ruta del archivo
                    configuracion = leerArchivoConfiguracion(strdup(configuracion->rutaArchivoConfiguracion));   //POSIBLE RACE CONDITION!!! CHECK!!!! SEGMENTATION FAULT??
                    configuracion->interfazRed = obtenerInterfazDeRed();
                    
                    
                    
                    //TODO"Liberamos la configuracion antigua
                    free(configuracionAnterior->id_nodo);
                    free(configuracionAnterior->interfazRed);
                    free(configuracionAnterior->rutaArchivoConfiguracion);
                    free(configuracionAnterior->rutaArchivoColumnasBDADC);
                    free(configuracionAnterior->rutaArchivoColumnasBDDIO);
                    free(configuracionAnterior->usuarioBD);
                    free(configuracionAnterior->claveBD);
                    free(configuracionAnterior->ipServidorBD);
                    free(configuracionAnterior->BD);
                    free(configuracionAnterior->comunidadSNMP);
                    free(configuracionAnterior->nombreSesionSNMP);
                    free(configuracionAnterior->ipServidorActualizaciones);
                    
                    printf("WIiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
                    
                    //Liberamos las direcciones SNMP
                    
                    int indiceSNMP = 0;
                    for(indiceSNMP = 0; indiceSNMP < configuracionAnterior->numeroServidoresSNMP; indiceSNMP++){
                        free(configuracionAnterior->ipServidorSNMP[indiceSNMP]);
                    }
                    
                    //TODO Abrir las nuevas sesiones SNMP...??
                    
                    
                    //Liberamos los valores minimos permitidos
                    int indiceMinimosPer = 0;
                    for(indiceMinimosPer = 0; indiceMinimosPer < configuracionAnterior->numeroValoresMinimosPermitidos; indiceMinimosPer++){
                        free(configuracionAnterior->valoresMinimosPermitidosMediciones[indiceMinimosPer]);
                    }
                    
                    
                    //Liberamos los destinatarios de alertas
                    int indiceDestAlertas = 0;
                    for(indiceDestAlertas = 0; indiceDestAlertas < configuracionAnterior->numeroDestinatariosAlertas; indiceDestAlertas++){
                        free(configuracionAnterior->destinatariosAlertas[indiceDestAlertas]);
                    }
                    
                    //Liberamos los canales activos
                    int indiceCanalesActivos = 0;
                    for(indiceCanalesActivos = 0; indiceCanalesActivos < configuracionAnterior->numeroCanalesActivos; indiceCanalesActivos++){
                        free(configuracionAnterior->canalesActivos[indiceCanalesActivos]);
                    }
                    
                    free(configuracionAnterior);     
                    sprintf(buffer, "Minimos cambiados exitosamente.");
                }
                else{
                    res.status = ERROR;
                    sprintf(buffer, "No se pudieron cambiar los minimos");*/
                }
                
                
                res.long_res = strlen(buffer);
                res.res = buffer;
                enviarDatos(fd, &res, RESPUESTA);
                free(buffer);
                free(comandoCambiarMinimosStr);
                
                //TODO: free argumentos???
                
            }
            break;
        /*case INFORMACION_CONF:
            res.status = OK;
            
            //Obtenemos los servidores SNMP
            int i;
            int lon_str_snmp = sizeof(char)*(configuracion->numeroServidoresSNMP*60);
            char *servidoresSNMP = malloc(lon_str_snmp);
            
            if(servidoresSNMP != NULL){
                memset(servidoresSNMP, 0, lon_str_snmp /(sizeof(char)));            
                for(i = 0; i < configuracion->numeroServidoresSNMP; i++){
                    if(i == 0){
                        strncat(servidoresSNMP, "Servidores SNMP: ", 20);
                    }
                    else{
                        strncat(servidoresSNMP, "                 ", 20);
                    }
                    strncat(servidoresSNMP, configuracion->ipServidorSNMP[i],21);
                    
                    if(i < configuracion->numeroServidoresSNMP - 1){
                        strncat(servidoresSNMP, "\n", 20);
                    }
                    else strncat(servidoresSNMP, "\0", 20);        
                }
            }
            else return -1;
            
            //Buffer de respuesta
            lon = longitudMinimaBuffer*sizeof(char) + lon_str_snmp;
            buffer = malloc(lon);              //CHECK
            if(buffer != NULL){
                memset(buffer, 0, lon);

                char bufferNumero[20];
                memset(bufferNumero, 0, 20);
                sprintf(bufferNumero, "%.2f", ((float)(configuracion->intervaloMonitoreo)) / ((float)(SEGUNDOS_POR_MINUTO)));
                
                char bufferNumero2[20];
                memset(bufferNumero2, 0, 20);
                sprintf(bufferNumero2, "%.2f", ((float)configuracion->intervaloMonitoreoPuerta) / 1000.0f);
                
                snprintf(buffer, lon, "Interfaz de red del monitor: %s\nDireccion IP del monitor: %s\nDireccion IP del servidor de base de datos: %s\nArchivo de columnas de base de datos: %s\n%s\nIntervalo de monitoreo: %s minuto(s)\nIntervalo de monitoreo de puerta: %s segundo(s)",
                         configuracion->interfazRed,
                         informacion_nodo.ip,
                         configuracion->ipServidorBD,
                         configuracion->rutaArchivoColumnasBDADC,
                         servidoresSNMP,
                         bufferNumero,
                         bufferNumero2);
            
                res.long_res = strlen(buffer);
                res.res = buffer;
                enviarDatos(fd, &res, RESPUESTA);
                free(buffer);
                free(servidoresSNMP);
            }
            else return -1;
            break;*/
        case INFORMACION_VAL_MIN:
            buffer = malloc(longitudMinimaBuffer);
            
            if(buffer == NULL) return -1;           //no hay memoria
            
            memset(buffer, 0, longitudMinimaBuffer);
            
            //strcat(buffer, "Valores minimos configurados:\n");
            //int j;
            
            snprintf(buffer, longitudMinimaBuffer, "Valores de niveles de alerta nodo %s:\n"
                     "Temperatura 1: %s C\n"
                     "Temperatura 2: %s C\n"
                     "Temperatura 3: %s C\n"
                     "Humedad: %s %% HR\n"
                     "Voltaje DC 1: %s V\n"
                     "Voltaje DC 2: %s V\n"
                     "Voltaje DC 3: %s V\n"
                     "Voltaje DC 4: %s V\n"
                     "Corriente DC 1: %s A\n"
                     "Corriente DC 2: %s A\n"
                     "Corriente DC 3: %s A\n"
                     "Corriente DC 4: %s A\n"
                     "Corriente AC 1: %s A\n"
                     "Corriente AC 2: %s A\n"
                     "Corriente AC 3: %s A\n"
                     "Corriente AC 4: %s A\n"
                     "Voltaje AC 1: %s V\n"
                     "Voltaje AC 2: %s V\n",
                    informacion_nodo.id,
                    configuracion->valoresMinimosPermitidosMediciones[1],
                    configuracion->valoresMinimosPermitidosMediciones[5],
                    configuracion->valoresMinimosPermitidosMediciones[9],
                    configuracion->valoresMinimosPermitidosMediciones[13],
                    configuracion->valoresMinimosPermitidosMediciones[17],
                    configuracion->valoresMinimosPermitidosMediciones[3],
                    configuracion->valoresMinimosPermitidosMediciones[7],
                    configuracion->valoresMinimosPermitidosMediciones[10],
                    configuracion->valoresMinimosPermitidosMediciones[0],
                    configuracion->valoresMinimosPermitidosMediciones[4],
                    configuracion->valoresMinimosPermitidosMediciones[8],
                    configuracion->valoresMinimosPermitidosMediciones[11],
                    configuracion->valoresMinimosPermitidosMediciones[14],
                    configuracion->valoresMinimosPermitidosMediciones[16],
                    configuracion->valoresMinimosPermitidosMediciones[2],
                    configuracion->valoresMinimosPermitidosMediciones[6],
                    configuracion->valoresMinimosPermitidosMediciones[12],
                    configuracion->valoresMinimosPermitidosMediciones[15]);
            res.status = OK;
            res.res = buffer;
            res.long_res = strlen(buffer);
            enviarDatos(fd, &res, RESPUESTA);
            free(buffer);
            break;
        case MOSTRAR_MEDICIONES:
            buffer = malloc(longitudMinimaBuffer);
            
            if(buffer == NULL){
                return -1;           //no hay memoria
            }
            
            memset(buffer, 0, longitudMinimaBuffer);
            
            float* listaValoresTmp = NULL; 
            int tamanoListaValoresTmp = sizeof(float)*NUMERO_MEDICIONES_ADC;
            
            listaValoresTmp = malloc(tamanoListaValoresTmp);
            
            if(listaValoresTmp == NULL){
                return -1;      //No hay memoria
            }
            
            memset(listaValoresTmp, 0, tamanoListaValoresTmp);
  
            volatile struct medicion* k = listaMediciones;
            int w = 0;
            for(w = 0; w < NUMERO_MEDICIONES_ADC; w++){
                listaValoresTmp[w] = k->valor;                //!!
                k = k->siguiente;
                
            }
            
            //Para AC
            if(listaValoresTmp[12] < 15.00f && listaValoresTmp[15] < 15.00f){
                snprintf(buffer, longitudMinimaBuffer, "Status del nodo %s:\n"
                         "Temperatura 1: %.2f C\n"
                         "Temperatura 2: %.2f C\n"
                         "Temperatura 3: %.2f C\n"
                         "Humedad: %.2f %% HR\n"
                         "Voltaje DC 1: %.2f V\n"
                         "Voltaje DC 2: %.2f V\n"
                         "Voltaje DC 3: %.2f V\n"
                         "Voltaje DC 4: %.2f V\n"
                         "Corriente DC 1: %.2f A\n"
                         "Corriente DC 2: %.2f A\n"
                         "Corriente DC 3: %.2f A\n"
                         "Corriente DC 4: %.2f A\n"
                         "Corriente AC 1: %.2f A\n"
                         "Corriente AC 2: %.2f A\n"
                         "Corriente AC 3: %.2f A\n"
                         "Corriente AC 4: %.2f A\n"
                         "Voltaje AC 1: < 15 V (RMS)\n"
                         "Voltaje AC 2: < 15 V (RMS)\n",
                        informacion_nodo.id,
                        listaValoresTmp[1],
                        listaValoresTmp[5],
                        listaValoresTmp[9],
                        listaValoresTmp[13],
                        listaValoresTmp[17],
                        listaValoresTmp[3],
                        listaValoresTmp[7],
                        listaValoresTmp[10],
                        listaValoresTmp[0],
                        listaValoresTmp[4],
                        listaValoresTmp[8],
                        listaValoresTmp[11],
                        listaValoresTmp[14],
                        listaValoresTmp[16],
                        listaValoresTmp[2],
                        listaValoresTmp[6]);
                
            }
            else if(listaValoresTmp[12] < 15.00f){
                snprintf(buffer, longitudMinimaBuffer, "Status del nodo %s:\n"
                         "Temperatura 1: %.2f C\n"
                         "Temperatura 2: %.2f C\n"
                         "Temperatura 3: %.2f C\n"
                         "Humedad: %.2f %% HR\n"
                         "Voltaje DC 1: %.2f V\n"
                         "Voltaje DC 2: %.2f V\n"
                         "Voltaje DC 3: %.2f V\n"
                         "Voltaje DC 4: %.2f V\n"
                         "Corriente DC 1: %.2f A\n"
                         "Corriente DC 2: %.2f A\n"
                         "Corriente DC 3: %.2f A\n"
                         "Corriente DC 4: %.2f A\n"
                         "Corriente AC 1: %.2f A\n"
                         "Corriente AC 2: %.2f A\n"
                         "Corriente AC 3: %.2f A\n"
                         "Corriente AC 4: %.2f A\n"
                         "Voltaje AC 1: < 15 V (RMS)\n"
                         "Voltaje AC 2: %.2f V (RMS)\n",
                        informacion_nodo.id,
                        listaValoresTmp[1],
                        listaValoresTmp[5],
                        listaValoresTmp[9],
                        listaValoresTmp[13],
                        listaValoresTmp[17],
                        listaValoresTmp[3],
                        listaValoresTmp[7],
                        listaValoresTmp[10],
                        listaValoresTmp[0],
                        listaValoresTmp[4],
                        listaValoresTmp[8],
                        listaValoresTmp[11],
                        listaValoresTmp[14],
                        listaValoresTmp[16],
                        listaValoresTmp[2],
                        listaValoresTmp[6],
                        listaValoresTmp[15]);
                
            }
            else if(listaValoresTmp[15] < 15.00f){
                snprintf(buffer, longitudMinimaBuffer, "Status del nodo %s:\n"
                         "Temperatura 1: %.2f C\n"
                         "Temperatura 2: %.2f C\n"
                         "Temperatura 3: %.2f C\n"
                         "Humedad: %.2f %% HR\n"
                         "Voltaje DC 1: %.2f V\n"
                         "Voltaje DC 2: %.2f V\n"
                         "Voltaje DC 3: %.2f V\n"
                         "Voltaje DC 4: %.2f V\n"
                         "Corriente DC 1: %.2f A\n"
                         "Corriente DC 2: %.2f A\n"
                         "Corriente DC 3: %.2f A\n"
                         "Corriente DC 4: %.2f A\n"
                         "Corriente AC 1: %.2f A\n"
                         "Corriente AC 2: %.2f A\n"
                         "Corriente AC 3: %.2f A\n"
                         "Corriente AC 4: %.2f A\n"
                         "Voltaje AC 1: %.2f V (RMS)\n"
                         "Voltaje AC 2: < 15 V (RMS)\n",
                        informacion_nodo.id,
                        listaValoresTmp[1],
                        listaValoresTmp[5],
                        listaValoresTmp[9],
                        listaValoresTmp[13],
                        listaValoresTmp[17],
                        listaValoresTmp[3],
                        listaValoresTmp[7],
                        listaValoresTmp[10],
                        listaValoresTmp[0],
                        listaValoresTmp[4],
                        listaValoresTmp[8],
                        listaValoresTmp[11],
                        listaValoresTmp[14],
                        listaValoresTmp[16],
                        listaValoresTmp[2],
                        listaValoresTmp[6],
                        listaValoresTmp[12]);
                
            }
            
            else{
                snprintf(buffer, longitudMinimaBuffer, "Status del nodo %s:\n"
                         "Temperatura 1: %.2f C\n"
                         "Temperatura 2: %.2f C\n"
                         "Temperatura 3: %.2f C\n"
                         "Humedad: %.2f %% HR\n"
                         "Voltaje DC 1: %.2f V\n"
                         "Voltaje DC 2: %.2f V\n"
                         "Voltaje DC 3: %.2f V\n"
                         "Voltaje DC 4: %.2f V\n"
                         "Corriente DC 1: %.2f A\n"
                         "Corriente DC 2: %.2f A\n"
                         "Corriente DC 3: %.2f A\n"
                         "Corriente DC 4: %.2f A\n"
                         "Corriente AC 1: %.2f A\n"
                         "Corriente AC 2: %.2f A\n"
                         "Corriente AC 3: %.2f A\n"
                         "Corriente AC 4: %.2f A\n"
                         "Voltaje AC 1: %.2f V (RMS)\n"
                         "Voltaje AC 2: %.2f V (RMS)\n",
                        informacion_nodo.id,
                        listaValoresTmp[1],
                        listaValoresTmp[5],
                        listaValoresTmp[9],
                        listaValoresTmp[13],
                        listaValoresTmp[17],
                        listaValoresTmp[3],
                        listaValoresTmp[7],
                        listaValoresTmp[10],
                        listaValoresTmp[0],
                        listaValoresTmp[4],
                        listaValoresTmp[8],
                        listaValoresTmp[11],
                        listaValoresTmp[14],
                        listaValoresTmp[16],
                        listaValoresTmp[2],
                        listaValoresTmp[6],
                        listaValoresTmp[12],
                        listaValoresTmp[15]);
            }
            
            //printf("%s\n",buffer);
            
            res.status = OK;
            res.res = buffer;
            res.long_res = strlen(buffer);
            enviarDatos(fd, &res, RESPUESTA);
            free(buffer);
            free(listaValoresTmp);
            break;
        case ACTIVAR_RELE:
            
            res.status = OK; 
                   
            //TODO aqui activamos el relé
            int resultado = activarRele(atoi(com->argumentos[0]), informacion_nodo.id, configuracion->rutaArchivoColumnasBDDIO);  //problemas--> no me deja ejecutar comandos una vez que ejecutp esto.
            //int resultado = 0;
            char *respuestaTmp = malloc(sizeof(char)*100);
            if(respuestaTmp == NULL){
                //TODO enviar respuesta de error
                return -1;
            }
            memset(respuestaTmp, 0, 100);
            
            //activarRele devuelve 0 cuando termina correctamente
            if(!resultado){
                snprintf(respuestaTmp, 100, "Se activo el rele %s.",com->argumentos[0]);
            }
            else{
                res.status = ERROR; 
                snprintf(respuestaTmp, 100, "No se pudo activar el rele %s.",com->argumentos[0]);
            }           
            
            res.res = respuestaTmp;
            res.long_res = strlen(res.res);
            enviarDatos(fd, &res, RESPUESTA);
            free(respuestaTmp);
            break;
        case DESACTIVAR_RELE:
            res.status = OK;            //hay que hacer scope con llaves, caso contrario al borrar esto, error de compilacion
            
            //TODO aqui desactivamos el relé
            int resultado2 = desactivarRele(atoi(com->argumentos[0]), informacion_nodo.id, configuracion->rutaArchivoColumnasBDDIO);
            //int resultado2 = 0;
            char *respuestaTmp2 = malloc(sizeof(char)*100);
            
            if(respuestaTmp == NULL){
                //TODO enviar respuesta de error
                return -1;
            }
            memset(respuestaTmp2, 0, 100);
            
            if(!resultado2){
                snprintf(respuestaTmp2, 100, "Se desactivo el rele %s.",com->argumentos[0]);
            }
            else{
                res.status = ERROR;
                snprintf(respuestaTmp2, 100, "No se pudo desactivar el rele %s.",com->argumentos[0]);
            }
            
            res.res = respuestaTmp2;
            res.long_res = strlen(res.res);
            enviarDatos(fd, &res, RESPUESTA);
            free(respuestaTmp2);
            break;
        case AYUDA:
            buffer = malloc(longitudMinimaBuffer);
            
            if(buffer == NULL) return -1;           //no hay memoria
            
            memset(buffer, 0, longitudMinimaBuffer);
            
            //strcat(buffer, "Valores minimos configurados:\n");
            //int j;
            
            snprintf(buffer, longitudMinimaBuffer, "Comandos disponibles:\n\n"
                     "- cerrar monitor: esta comando cierra el proceso de monitoreo.\n\n"
                     "- info nodo: devuelve informacion sobre el nodo, como direccion IP y nombre del nodo.\n\n"
                     "- info configuracion: devuelve informacion de como esta configurado el sistema de monitoreo.\n\n"
                     "- info niveles alertas: devuelve informacion de los valores que activaran las alertas para cada medicion.\n\n"
                     "- status nodo: devuelve las mediciones actuales de voltaje, corriente, temperatura y humedad.\n\n"
                     "- activar rele <numero>: Activa el rele indicado por el numero (de 1 a 4). Ejemplo: activar rele 2.\n\n"
                     "- desactivar rele <numero>: Desactiva el rele indicado por el numero (de 1 a 4). Ejemplo: desactivar rele 2.\n\n"
                     "- cambiar minimos: opcion para cambiar los niveles por debajo de los cuales se enviaran las alertas, para cada medicion.");
            
            res.status = OK;
            res.res = buffer;
            res.long_res = strlen(buffer);
            enviarDatos(fd, &res, RESPUESTA);
            free(buffer);
            break;
        default:
            printf("ALERTA: Se recibio un comando invalido.\n");
            res.status = ERROR;
            res.res = "Comando invalido.";
            res.long_res = strlen(res.res);
            enviarDatos(fd, &res, RESPUESTA);            
            return -1;
    }
    
    return 1;
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
                else if(strstr(linea, IP_SERVIDOR_BASE_DE_DATOS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->ipServidorBD = valores[0];
                        printf("INFO: IP del servidor de la base de datos: %s\n", configuracion->ipServidorBD);
                    }
                    else return NULL;
                }
                else if(strstr(linea, USUARIO_BASE_DE_DATOS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->usuarioBD = valores[0];
                        printf("INFO: Usuario de la base de datos: %s\n", configuracion->usuarioBD);
                    }
                    else return NULL;

                }
                else if(strstr(linea, CLAVE_BASE_DE_DATOS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->claveBD = valores[0];
                    }
                    else return NULL;
                   
                }
                else if(strstr(linea, BASE_DE_DATOS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    
                    if(valores != NULL){
                        configuracion->BD = valores[0];
                        printf("INFO: Base de datos: %s\n", configuracion->BD);
                    }
                    else return NULL;
                    
                }
                else if(strstr(linea, PUERTO_BASE_DE_DATOS) != NULL){
                    
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        int puerto = atoi(valores[0]);
                          
                        if(puerto == 0){
                            configuracion->puertoBD = 0;    //valor predeterminado
                        }
                        else{
                            configuracion->puertoBD = puerto;    //valor predeterminado
                        }
                        printf("INFO: Puerto de la base de datos: %d\n", configuracion->puertoBD);
                    }
                    else return NULL;
                }
                else if(strstr(linea, IP_SERVIDOR_SNMP) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->ipServidorSNMP = valores;
                        configuracion->numeroServidoresSNMP = num; 
                        int i;
                        for(i = 0; i < configuracion->numeroServidoresSNMP; i++){
                            //printf("INFO: IP del servidor SNMP %d: %s\n", (i+1), configuracion->ipServidorSNMP[i]);
                        }
                    }
                    else return NULL;
                }
                else if(strstr(linea, COMUNIDAD_SNMP) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->comunidadSNMP = valores[0];
                        // printf("INFO: Comunidad SNMP: %s\n", configuracion->comunidadSNMP);
                    }
                    else return NULL;
                }
                else if(strstr(linea, SESION_SNMP) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->nombreSesionSNMP = valores[0];
                        //printf("INFO: Sesion SNMP: %s\n", configuracion->nombreSesionSNMP);
                    }
                    else return NULL;
                }
                else if(strstr(linea, SERVIDOR_ACTUALIZACIONES) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->ipServidorActualizaciones = valores[0];
                        printf("INFO: Direccion IP del servidor de actualizaciones: %s\n", configuracion->ipServidorActualizaciones);
                    }
                    else return NULL;
                }
                else if(strstr(linea, VALORES_MINIMOS_PERMITIDOS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        
                        //configuracion->valoresMinimosPermitidosMediciones = valores;
                        configuracion->numeroValoresMinimosPermitidos = num;
        
                        //Asignamos los valores a la configuracion
                        configuracion->valoresMinimosPermitidosMediciones = valores;
                    }
                    else return NULL;
                }
                else if(strstr(linea, DESTINATARIOS_ALERTAS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        //configuracion->valoresMinimosPermitidosMediciones = valores;
                        configuracion->numeroDestinatariosAlertas = num;
        
                        //Asignamos los valores a la configuracion
                        configuracion->destinatariosAlertas = valores;
#ifdef DEBUG
                        int i=0;
                        for(i = 0; i< configuracion->numeroDestinatariosAlertas; i++){
                            printf("Destinatario %d: %s\n", i+1, configuracion->destinatariosAlertas[i]);
                        }
#endif
                    }
                    else return NULL;
                }
                else if(strstr(linea, HABILITAR_ALERTAS) != NULL){
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        
                        //configuracion->valoresMinimosPermitidosMediciones = valores;
                        configuracion->numeroCanalesActivos = num;
                        //Asignamos los valores a la configuracion
                        configuracion->canalesActivos = valores;
                    }
                    else return NULL;
                }
                else if(strstr(linea, PERIODO_ENVIO_EMAILS)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->periodoEnvioEmails = atoi(valores[0]);
                        printf("INFO: Periodo de envio de e-mails de alerta: %d minutos.\n", configuracion->periodoEnvioEmails);
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
                else if(strstr(linea, MODBUS_BAUDRATE)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusBaudrate = atoi(valores[0]);
                        printf("INFO: Baud-rate de modbus: %d:1.\n", configuracion->modbusBaudrate);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_DATABITS)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusDatabits = atoi(valores[0]);
                        printf("INFO: Bits de datos modbus: %d.\n", configuracion->modbusDatabits);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_STOPBITS)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusStopbits = atoi(valores[0]);
                        printf("INFO: Bits de stop modbus: %d.\n", configuracion->modbusStopbits);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_TTY) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusTTY = valores[0];
                        printf("INFO: TTY de modbus: %s\n", configuracion->modbusTTY);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_SLAVE_ID)!= NULL){      
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusSlaveId = atoi(valores[0]);
                        printf("INFO: ID dispositivo esclavo modbus: %d.\n", configuracion->modbusSlaveId);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_PARITY) != NULL){ 
                    valores = obtenerValorConfig(linea, &num);
                    if(valores != NULL){
                        configuracion->modbusParidad = valores[0][0];                   //Valor[0] = string "N\0", por lo tanto tomamos el caracter que necesitamos.
                        printf("INFO: TTY de modbus: %c\n", configuracion->modbusParidad);
                    }
                    else return NULL;
                }
                else if(strstr(linea, MODBUS_MODO_PUERTO)!= NULL){
                    if(modoComunicacion == USAR_MODBUS)
                        valores = obtenerValorConfig(linea, &num);          //Modo 1: RS232, Modo 2: RS485 HD, Modo 3: RS485 FD
                        if(valores != NULL){
                            configuracion->modbusModoPuerto = atoi(valores[0]);
                            if(configuracion->modbusModoPuerto == MODO_RS_232){
                                printf("INFO: comunicacion modbus mediante RS-232\n");
                            }
                            else if(configuracion->modbusModoPuerto == MODO_RS_485_HD){
                                printf("INFO: comunicacion modbus mediante RS-485 Half-Duplex\n");
                            }
                            else if(configuracion->modbusModoPuerto == MODO_RS_485_FD){
                                printf("INFO: comunicacion modbus mediante RS-485 Full-Duplex\n");
                            }
                            else{
                                 printf("ERROR: modo modbus invalio. Saliendo.\n");
                                 exit(-1);
                            }
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

