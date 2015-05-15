#include "monitordef.h"
#include "mediciones.h"
#include "DIO.h"

//typedef enum {UNO,DOS,TRES,CUATRO} estados;
//estados estado = UNO;
int activarPuerto(puerto_DIO puerto_DIO_0);

/*Socket-client.c
 * tomado de: http://www.codeproject.com/Articles/586000/Networking-and-Socket-programming-tutorial-in-C
 */
int main2(void) {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    char fromUser[1024];
    struct sockaddr_in serv_addr;
    int conn;

    //printf("\nProbando SOCKETS");
    memset(recvBuff, '0', sizeof (recvBuff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    serv_addr.sin_addr.s_addr = inet_addr("172.40.0.10");

    //printf("\nConecctandose...");
    conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
    //printf("\nRespuesta: %d", conn);
    if (conn < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    //printf("\nLeyendo datos");
    while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
        recvBuff[n] = '\0';
        printf("\nServer: %s", recvBuff);
        if (strcmp(recvBuff, "Bye.") == 0) {
            break;
        }
        printf("\nRespuesta: ");
        bzero(fromUser, 1024);
        fgets(fromUser, 1023, stdin);
        //scanf("%100[^\n]%c", fromUser, &enter);
        //printf("\n'%s' -> %d",fromUser,strlen(fromUser));
        n = write(sockfd, fromUser, strlen(fromUser));
        /*
                if (fputs(recvBuff, stdout) == EOF) {
                    printf("\n Error : Fputs error");
                }
                printf("\n");
         */
    }

    if (n < 0) {
        printf("\n FIN \n");
    }

    return 0;
}

/**
 *Rutina para hacer del proceso un daemon. TODO
 *
 */
void daemonize() {

    int i = fork(); //Emancipamos el proceso...
    if (i < 0) exit(1);
    if (i > 0) exit(0);
}

/**
 *Programa principal
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
        printf("ERROR: Existe un error en el formato del archivo de configuracion %s,revise la configuracion del mismo. Este programa se cerrara.\n", rutaArchivoConfiguracion);
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

    //Ya que no tenemos un RTC, actualizamos la fecha silenciosamente
    /*if (system("ntpdate -s -h ntp.telconet.net")) {
        printf("ERROR: No se pudo actualizar la fecha.\n");
    } else {
        printf("INFO: Se ha actualizado la fecha correctamente.\n");
    }*/

    printf("INFO: Direccion IP del monitor de nodo: %s\n", informacion_nodo.ip);

    //Creamos la lista de mediciones 
    listaMediciones = inicializarListaMediciones(NUMERO_MEDICIONES_ADC);
    
    //Creamos un thread que va a llevar el control de los aires acondicionados.
    //La lista de mediciones debe estar ya inicializada.
    /*int tAiresAcondicionados;

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
    }*/

    char *hora = obtenerHora();
    char *fecha = obtenerFecha();

    //Empezamos el monitoreo
    printf("INFO: Hora de inicializacion: %s\n", hora);
    printf("INFO: Fecha de inicializacion: %s\n", fecha);
    printf("INFO: Programa de monitoreo iniciado (PID %d).\n", processID);
    printf("INFO: Monitoreando...\n");

    free(fecha);
    free(hora);


    //*********PRUEBA ADC
    /*
        uint16_t* data_canal_2 = NULL;
        uint16_t* data_canal_1 = NULL;
        int noMuestras = NUMERO_MUESTRAS;
        int canal = 0;
        float voltaje1 = 0.0f;
        float voltaje2 = 0.0f;
        
        int tamanoBuffers = sizeof(uint16_t)*noMuestras;
        data_canal_1 = malloc(tamanoBuffers);
        data_canal_2 = malloc(tamanoBuffers);
     */
    //********PRUEBA ADC
    //canal = 20;         //-->quitar
    //Empezamos el monitoreo
    //float temperaturaHumedad = 0.0f;
    /*Vamos a hacer que el programa de monitoreo corra cada 14.675 minutos aproximadamente. Por esta razón podemos observar
    que en el lazo while se encuentra una comparación < (60 * 13) que indica 60 segundos multipicado por 13 minutos
    pero estos trece minutos no son 13 minutos exactos son los 14.675 minutos mencionados anteriormente. Para esto
    usamos las variables inicio, fin y descanso, y también la función clock que nos cuenta el número de pulsos del reloj interno
    por segundo. Luego de los 14.675 minutos, programamos el crontab de tal forma que cada 15 minutos vuelva levantar
    el daemon de monitoreo.
    */
    
    while (1){
	realizarMediciones(&listaMediciones);
	status_puerto_DIO stp = PUERTO_OFF;//Sensor del aire principal
	status_puerto_DIO sts = PUERTO_OFF;//Sensor del aire secundario
        almacenarMediciones(&listaMediciones, informacion_nodo.id, configuracion->rutaArchivoColumnasBDADC, NUMERO_MEDICIONES_ADC,stp,sts);
        
	//configuracion->valoresMinimosPermitidosMediciones, configuracion->numeroValoresMinimosPermitidos);
        sleep(configuracion->intervaloMonitoreo); //damos tiempo que sensores se activen, etc.
	
        
    }

    return 0;
}

