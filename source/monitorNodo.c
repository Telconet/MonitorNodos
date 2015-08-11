#include "monitordef.h"
#include "mediciones.h"
#include "DIO.h"

//typedef enum {UNO,DOS,TRES,CUATRO} estados;
//estados estado = UNO;
int activarPuerto(puerto_DIO puerto_DIO_0);


/**
 *Programa principal
 */
int main(int argc, char *argv[]) {
    
    

    
    //MODBUS TEST
    modbus_t *contexto_modbus = NULL;
    int what =  conectar_modbus_serial(MODO_RS_232, 115200, COM2, 8, 'N', 1, &contexto_modbus, 10);
    
    if(what < 0 || contexto_modbus == NULL){
	printf("Error al conectar modbus...");
	exit(-1);
    }
    
    
    /*int fd = open(COM2, O_RDWR);
    
    if( fd < 0){
	fprintf(stderr, "No se pudo abrir el puerto COM2");
    }
    
    char buf[100];
    memset(buf, 0, 100);
    
    snprintf(buf, 100, "Hola!\n");
    
    int err = write(fd, buf, 100);
    
    if(err < 0)
	perror("Error en write: ");
	
	
	
    int res2 = pthread_mutex_init(&mutexModbus, NULL);
	
    if(res2 != 0){
	perror("ERROR: No se pudo crear el mutex modbus. Saliendo.");
	exit(-1);
    }*/
    /*int direccion = atoi(argv[1]);
    printf("Direccion :%d\n", direccion);*/
   
    int tModbus = pthread_create(&hiloModbus, NULL, (void *)monitorModbus, (void *) contexto_modbus);

    if (tModbus != 0) {
	perror("ERROR: No se pudo crear el thread Modbus. Saliendo...\n");
	exit(-1);
    }
    
    //float valor = 3.843f;
    
    //float leido = 4.5f;   			
    
    //int st = asignarRegistroInputFloat(mapeo_modbus, valor, 1, NO_SWAP);
    uint16_t i = 0;
    int bit = 0;
    while(true){
	
	if(bit == 0){
	    
	    printf("fuck\n");
	    if(asignarBit(mapeo_modbus, OFF,0) < 0){
		printf("error al cambiar bit...\n");
	    }
	    asignarBit(mapeo_modbus, OFF,1);
	    asignarBit(mapeo_modbus, OFF,2);
	    asignarBit(mapeo_modbus, OFF,3);
	    asignarInputBit(mapeo_modbus, OFF, 0);
	    asignarInputBit(mapeo_modbus, OFF, 1);
	    asignarInputBit(mapeo_modbus, OFF, 2);
	    bit = 1;
	    printf(" what\n");
	}
	else{
	    asignarBit(mapeo_modbus, ON,0);
	    asignarBit(mapeo_modbus, ON,1);
	    asignarBit(mapeo_modbus, ON,2);
	    asignarBit(mapeo_modbus, ON,3);
	    asignarInputBit(mapeo_modbus, ON, 0);
	    asignarInputBit(mapeo_modbus, ON, 1);
	    asignarInputBit(mapeo_modbus, ON, 2);
	    bit = 0;
	    printf("not\n");
	}
	
	printf("Input regs...\n");
	
	asignarRegistroInputFloat(mapeo_modbus, 2565.3, 0, 1);
	asignarRegistroInput(mapeo_modbus, i + 2, 2);
	asignarRegistroInput(mapeo_modbus, i + 3, 3);
	asignarRegistroInput(mapeo_modbus, i + 4, 4);
	asignarRegistroInput(mapeo_modbus, i + 3, 5);
	asignarRegistroInput(mapeo_modbus, i + 5, 6);
	asignarRegistroInput(mapeo_modbus, i + 6, 7);
	asignarRegistroInput(mapeo_modbus, i + 7, 8);
	asignarRegistroInput(mapeo_modbus, i + 8, 9);
	asignarRegistroInput(mapeo_modbus, i + 9, 10);
	asignarRegistroInput(mapeo_modbus, i + 10, 11);
	asignarRegistroInput(mapeo_modbus, i + 11, 12);
	asignarRegistroInput(mapeo_modbus, i + 12, 13);
	asignarRegistroInput(mapeo_modbus, i + 13, 14);
	asignarRegistroInput(mapeo_modbus, i + 14, 15);
	asignarRegistroInput(mapeo_modbus, i + 15, 16);
    
	i++;
	printf("wow\n");
	
	sleep(2);
    }
    
    
    //printf("0x%X\n", mapeo_modbus->tab_input_registers[direccion]);
    
    
    
        
    /*if(st == -1){
	printf(" Error modbus\n");
    }
    
    printf("Antes leerRegistroInputFloat (valor) 0x%X\n", *(unsigned int*)&leido);
    leerRegistroInputFloat(mapeo_modbus, 1, &leido, NO_SWAP);
    
    printf("Leido (hex) 0x%X\n", *(unsigned int*)&leido);
    printf("Leido (float): %f\n", leido);*/
    
    while(1){
	sleep(1);
    }
    
    exit(-1);
    
    //***********

    //cambiemos el directorio de trabajo
    /*chdir(DIRECTORIO_DE_TRABAJO);

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
    
    //Hay un tercer argumento (modbus)
    if(argc == 3){
	if(!strcmp(argv[2], "modbus")){
	    printf("INFO: Directiva modbus especificada. Se usara comunicacion modbus.\n");
	    modoComunicacion = USAR_MODBUS;
	}
	else{
	    printf("ERROR: Tercer argumento no reconocido. Se usara comuncacion TCP\n");
	    modoComunicacion = USAR_TCP;
	}
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
    
    
    
    //Creamos el thread que será el local de recepcion de comandos
    //y envio de respuestas
    int tRet;

    tRet = pthread_create(&localComandosThread, NULL, (void *)recComandosEnvResp, NULL);

    if (tRet != 0) {
        perror("ERROR: No se pudo crear el thread de comandos. Saliendo...\n");
        salir(0);
    }

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

    //Creamos la lista de mediciones 
    listaMediciones = inicializarListaMediciones(NUMERO_MEDICIONES_ADC);
    
    //Creamos un thread que va a llevar el control de los aires acondicionados.
    //La lista de mediciones debe estar ya inicializada.
    int tAiresAcondicionados;

    int res = pthread_mutex_init(&mutexTemperatura, NULL);
   

    if(!res){
	tAiresAcondicionados = pthread_create(&monAiresAcondcionadosThread, NULL, (void *)monitorAiresAcondicionados, (void *) configuracion);
	if (tAiresAcondicionados != 0) {
	    perror("ALERTA: No se pudo crear el thread de monitoreo de los aires acondicionados. Saliendo...\n");
	}
    }
    else{
	perror("ALERTA: No se pudo crear el mutex para el thread de monitoreo de los aires acondicionados. Saliendo...\n");
	exit(-1);
    }
    
    //Inicializamos el sistema MODBUS
    modbus_t *contexto_modbus = NULL;
    printf("MonitorNodo context pointer... %p\n", contexto_modbus);
    
    //Creamos un thread para modbus
    if(modoComunicacion == USAR_MODBUS){
	
	//Al retornar, contexto_modbus apunta a la estructura creada
	int conn = conectar_modbus_serial(configuracion->modbusModoPuerto, configuracion->modbusBaudrate, configuracion->modbusTTY, configuracion->modbusDatabits,
			    configuracion->modbusParidad, configuracion->modbusStopbits, &contexto_modbus, configuracion->modbusSlaveId);
	
	//printf("MonitorNodo (after conectar_modbus_serial) context pointer... %p\n", contexto_modbus);
	
	if(contexto_modbus == NULL){
	    printf("ERROR: No se pudo crear el contexto modbus...%d\n", conn);
	    exit(-1);
	}
    
	if(conn != 0){
	    perror("Error al crear la conexion modbus");
	    exit(-1);
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


    
    while (1){
	realizarMediciones(&listaMediciones);
	status_puerto_DIO stp = PUERTO_OFF;//Sensor del aire principal
	status_puerto_DIO sts = PUERTO_OFF;//Sensor del aire secundario
	
	if(modoComunicacion == USAR_TCP){
	    almacenarMediciones(&listaMediciones, informacion_nodo.id, configuracion->rutaArchivoColumnasBDADC, NUMERO_MEDICIONES_ADC, stp, sts);
	
	}
	
	
	//configuracion->valoresMinimosPermitidosMediciones, configuracion->numeroValoresMinimosPermitidos);
        sleep(configuracion->intervaloMonitoreo); //damos tiempo que sensores se activen, etc.
	
        
    }*/

    return 0;
}

