#include "monitordef.h"
#include "mediciones.h"
#include "DIO.h"

/**
 *Programa principal
 *
 *Uso:
 *	0    1           		2    3        4       5
 *./monnod archivo_configuracion  [modbus slave_id baudrate modo]
 */
int main(int argc, char *argv[]) {
 

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
    if (argc < 2) {
        printf("ERROR: No se ha proporcionado la ruta del archivo de configuracion. Este programa se cerrara.\n");
        salir(EXIT_FAILURE);
    }


    char * rutaArchivoConfiguracion = argv[1];
    configuracion = leerArchivoConfiguracion(rutaArchivoConfiguracion);

    if (configuracion == NULL) {
        printf("ERROR: Existe un error en el formato del archivo de configuracion %s, revise la configuracion del mismo. Este programa se cerrara.\n", rutaArchivoConfiguracion);
        salir(EXIT_FAILURE);
    }

    //Solo para informacion
    configuracion->interfazRed = obtenerInterfazDeRed();

    //Esta rutina inicializa todo el sistema de mediciones
    //(asigna memoria, inicializa y configura el ADC, etc)
    inicializarSistemaMediciones(NUMERO_MUESTRAS, NUMERO_MAXIMO_CANALES_ADC);

    //Iniciamos los puerto DIO como salida
    configurarPuertosDIO();
    printf("INFO: Puertos entrada/salida configurados.\n");

    //Obtener informacion del nodo
    //Obtenemos la direccion IP del dispositivo
    char ip_host[NI_MAXHOST];

    if ((obtenerIPHost(ip_host, NI_MAXHOST)) != -1) {
        int longitud_id = strlen(configuracion->id_nodo);
        informacion_nodo.id = malloc(longitud_id + 1);
        memset(informacion_nodo.id, 0, longitud_id + 1);
        strncpy(informacion_nodo.id, configuracion->id_nodo, longitud_id);
        informacion_nodo.ip = ip_host;
        informacion_nodo.lon_ip = strlen(informacion_nodo.ip);
    }

    //Creamos un thread para controlar acceso a las puertas.
    int tPuerta;

    tPuerta = pthread_create(&monPuertaThread, NULL, (void *)monitorPuerta, (void *) configuracion);

    if (tPuerta != 0) {
        perror("ALERTA: No se pudo crear el thread de monitoreo de la puerta de acceso. Saliendo...\n");
    }

    printf("INFO: Direccion IP del monitor de nodo: %s\n", informacion_nodo.ip);
    printf("INFO: IP del servidor de datos: %s\n", configuracion->ip_servidor_datos);

    //Creamos la lista de mediciones 
    listaMediciones = inicializarListaMediciones(NUMERO_MEDICIONES_ADC);

    //Creamos un thread que va a llevar el control de los aires acondicionados.
    //La lista de mediciones debe estar ya inicializada.
    int tAiresAcondicionados;
    int res = pthread_mutex_init(&mutexTemperatura, NULL);
    int res2 = pthread_mutex_init(&mutex_status_AACC, NULL);       //Mutex el status de aires acondicionados...

    if(configuracion->monitoreoAires){
	if(res == 0 && res2 == 0){
	    tAiresAcondicionados = pthread_create(&monAiresAcondcionadosThread, NULL, (void *)monitorAiresAcondicionados, (void *) configuracion);
	    if (tAiresAcondicionados != 0) {
		perror("ALERTA: No se pudo crear el thread de monitoreo de los aires acondicionados. Saliendo...\n");
	    }
	}
	else{
	    perror("ALERTA: No se pudo crear el mutex para el thread de monitoreo de los aires acondicionados. Saliendo...\n");
	    exit(-1);
	}
    }
    
    //Para modbus
    //Verifcamos argumentos.
    int baud_rate = 19200;
    int id_esclavo = 0;
    int modo_puerto = 0;
    modbus_t *contexto_modbus = NULL;
    pthread_t hiloModbus;
	
    if(argc > 2){
	if(strstr(argv[2], "modbus")){
	    
	    if(argc < 6){
		fprintf(stderr, "ERROR: faltan argumentos para modbus\n");
		exit(EXIT_FAILURE);
	    }
	    
	    else{
		baud_rate = atoi(argv[4]);
		id_esclavo = atoi(argv[3]);
		modo_puerto = -1;
	    
		if(strstr(argv[5], "rs485fd")){
		    modo_puerto = MODO_RS_485_FD;
		}
		else if(strstr(argv[5], "rs485hd")){
		    modo_puerto = MODO_RS_485_HD;
		}
		else if(strstr(argv[5], "rs232")){
		    modo_puerto = MODO_RS_232;
		}
		else{
		    printf("ERROR: Modo puerto Modbus invalido\n");
		    exit(-1);
		}
	    
		//Al retornar, contexto_modbus apunta a la estructura creada
		int conn = conectar_modbus_serial(modo_puerto, baud_rate, COM2, 8, 'N', 1, &contexto_modbus, id_esclavo);
		
		//printf("MonitorNodo (after conectar_modbus_serial) context pointer... %p\n", contexto_modbus);
		
		if(contexto_modbus == NULL){
		    printf("ERROR: No se pudo crear el contexto modbus...%d\n", conn);
		    exit(-1);
		}
		else{
		    printf("INFO: Se creo el conteto modbus\n");
		}
	    
		if(conn != 0){
		    perror("Error al crear la conexion modbus");
		    exit(-1);
		}
		else{
		    printf("INFO: Se creo el la conexion serial modbus (COM2 - /dev/ttyAM1)\n");
		}
		
		int res2 = pthread_mutex_init(&mutexModbus, NULL);
		
		if(res2 != 0){
		    perror("ERROR: No se pudo crear el mutex modbus. Saliendo.");
		    exit(-1);
		}
	       
		int tModbus = pthread_create(&hiloModbus, NULL, (void *)monitorModbus, (void *) contexto_modbus);
	    
		if (tModbus != 0) {
		    perror("ERROR: No se pudo crear el thread Modbus. Saliendo...\n");
		    exit(-1);
		}
		
		usandoModbus = 1;			//Para saber si usamos el mutex de modbus
	    }
	}
    }
    


    char *hora = obtenerHora();
    char *fecha = obtenerFecha();

    //Empezamos el monitoreo
    printf("INFO: Hora de inicializacion: %s\n", hora);
    printf("INFO: Fecha de inicializacion: %s\n", fecha);
    printf("INFO: Programa de monitoreo iniciado (PID %d).\n", processID);
    printf("INFO: Monitoreando...\n");

    free(fecha);
    free(hora);

    /*Empezar el monitoreo*/    
    while (1){
	realizarMediciones(&listaMediciones);
    
	pthread_mutex_lock(&mutex_status_AACC);
	
	//Puerto 6000
        almacenarMediciones(&listaMediciones, informacion_nodo.id, configuracion->rutaArchivoColumnasBDADC, NUMERO_MEDICIONES_ADC, status_A_C_principal, status_A_C_backup);
        pthread_mutex_unlock(&mutex_status_AACC);
		
	//configuracion->valoresMinimosPermitidosMediciones, configuracion->numeroValoresMinimosPermitidos);
        sleep(configuracion->intervaloMonitoreo); //damos tiempo que sensores se activen, etc.
	
        
    }

    return 0;
}

