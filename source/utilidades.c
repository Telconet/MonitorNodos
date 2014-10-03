#include "definiciones.h"


/**
 *Aqui reportamos el PID al proceso controlador del daemon
 *
 */
void reportarPID(char *archivoProceso, int processID){
    
    char processIDStr[6];               //PID en formato string
    
    //Abrimos el archivo y lo creamos si no existe
    int fd = open(archivoProceso, O_WRONLY | O_CREAT);
    
    if(fd == -1){
#ifdef DEBUG
        printf("Linea %d: Error al abrir/crear archivo. Error %d. Saliendo...\n", __LINE__, errno);
#endif
    
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(1);
    }
    
    //Configuramos permiso del archivo como 666 rw-rw-rw
    chmod(archivoProceso, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    //Transformar el PID a string
    sprintf(processIDStr, "%d", processID);
    
    int numbytes = write(fd, processIDStr, strlen(processIDStr));
      
    if(numbytes == -1){
#ifdef DEBUG
        printf("Linea %d: Error al escribir en el archivo de PID. Error de archivo %d. Saliendo...\n", __LINE__, errno);
#endif
        close(fd);                                              //cerramos el archivo
        exit(1);
    }
    else{
#ifdef DEBUG
        printf("Linea %d: Se escribieron %d bytes al archivo %s.\n", __LINE__,numbytes, archivoProceso);
#endif
        close(fd);                                              //cerramos el archivo
    }
}



/**
 *Rutina para verificar que esta corriendo una sola instancia del daemon
 */
void verificarProgramaNoCorriendo(char *archivoProceso){
    
    int buffersize = 6;
    char processIDStr[buffersize];               //PID en formato string
    
    int fd = open(archivoProceso, O_RDONLY);
    
    //Si no existe el archivo, el proceso no esta corriendo
    if(fd == -1){
        printf("INFO: El proceso de monitoreo no esta corriendo. Se crea la nueva instancia.\n");
        return;
    }
    else{
        int pid;
        read(fd, processIDStr, buffersize);        //leemos el PID
        pid = atoi(processIDStr);
        printf("ERROR: Ya esta corriendo una instancia del daemon de monitoreo PID: %d\n", pid);
        close(fd);
        
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(1);
    }
    return;
}



/**
 *Rutina para buscar el PID del daemon de monitoreo
 *
 */
int buscarArchivoPIDProceso(char *archivoProceso){
    
    int fd = open(archivoProceso, O_RDONLY);
    
    if(fd == -1){
        printf("ERROR: El daemon de monitoreo no esta corriendo. Este programa se cerrara. Saliendo...\n");
        exit(1);
        
    }
    else{
        //Obtenemos el id del proceso
        char processIDStr[6];               //PID en formato string
        int pid = 0;
        int numbytes = pread(fd, processIDStr, 4, 0);
        
        //printf("Hola 2\n");
        
        if(numbytes > 0){           
            pid = atoi(processIDStr);
            //printf("hola 3");
            return pid;
        }
        else{
#ifdef DEBUG
            printf("Linea %d: Error al convertir el PID\n", __LINE__);
#endif
            //Removemos el archivo con el pid    
            char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
            strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
            strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
            system(comandoRemover);
            exit(1);
        }
    }
}

/**
 *Esta rutina remueve el salto de línea.
 */
void removerSaltoDeLinea(char *string, int tamano){
    int i;
    
    if(tamano == 0){
        return;
    }
    else{
        for(i=0; i < tamano; i++){
            if(string[i] == '\n'){
                string[i] = '\0';
                return;
            }
        }
    }
}


/**
 *funcion: parse_IP
 *Convierte un direccion IP en formato string a un arreglo de bytes
 *
 */
int parse_IP(char *ip_string, unsigned char *ip)
{
    //unsigned char ip[4] = {0,0,0,0};        //Los cuatro octetos de la dirección IP.
    char octet[3];                          //Un octeto tiene hasta 3 digitos.
    char tmpChar = 1;                           //will hold temporary character
    int index_octet = 0;
    int index_ip_string = 0;
    int index_parsed_ip = 0;
    int index = 0;
    
    
    while(1){
                
        tmpChar = ip_string[index_ip_string];
        if(index_parsed_ip > 3){                    //Encontramos el ultimo octeto, terminar el lazo.
            break;
        }
                    
        if(tmpChar != '.' && index_octet < 3){
            octet[index_octet] = tmpChar;            //añadir caracter al octeto
            index_octet++;
            index_ip_string++;
            
            //printf("%x\n", tmpChar);
        }
        else if(tmpChar == '.'){                     //tenemos un octeto completo
           ip[index_parsed_ip] = atoi(octet);       //parse el octeto, zero es retornado si el octeto es invalido
           //printf("Octet %d\n",ip[index_parsed_ip]);
           octet[0] = octet[1] = octet[2] = 0;      //limpiar octeto
           index_octet = 0;                         //limpiar index_octet
           index_parsed_ip++;                       //proximo octeto
           index_ip_string++;
        }
        else if(index_parsed_ip == 3 && index_octet == 3){          //tenemos un octeto completo
           ip[index_parsed_ip] = atoi(octet);                       //parse el octeto, zero es retornado si el octeto es invalido
           //printf("Octet %d\n",ip[index_parsed_ip]);
           //printf("AGH!\n");
           //invalid character, invalid IP, return 0.0.0.0
           //return -1;
           break;
        }     
    }
    
    return 1;
}


/**
 *Esta rutina obtiene la direccion ip del host
 *REVISAR --> Ahorita buscon con "ethx", buscar con "eth"
 */
int obtenerIPHost(char *host, int maxhost){
    
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    //char host[NI_MAXHOST];
    
           
    //Obtener informacion de interfaz
    if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
    }

    //Recorremos la lista de interfaces   
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        
        family = ifa->ifa_addr->sa_family;

        if(family == AF_INET && strstr(ifa->ifa_name, INTERFAZ_RED_PRINCIPAL) != NULL){      //Buscar interfaz primaria
            
            printf("INFO: Interfaz de red primaria: %s\n", ifa->ifa_name);                    //imprimir nombre de la interfaz
            
            //Convertir la direccion a un string de texto. Almacenar en la variable host
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, maxhost, NULL, 0, NI_NUMERICHOST);
            freeifaddrs(ifaddr);
            return 1;  
        }
    }
    
    return -1;
}

/**
 *Obtiene la interfaz de red
 */
char *obtenerInterfazDeRed(void){
    
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    
    //Obtener informacion de interfaz
    if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
    }

    //Recorremos la lista de interfaces   
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        
        family = ifa->ifa_addr->sa_family;

        if(family == AF_INET && strstr(ifa->ifa_name, INTERFAZ_RED_PRINCIPAL) != NULL){      //Buscar interfaz primaria
            
            int lon_interfaz = strlen(ifa->ifa_name);
            char *interfaz = malloc(lon_interfaz + 1);
            
            if(interfaz != NULL){
                memset(interfaz, 0, lon_interfaz + 1);
                strncpy(interfaz, ifa->ifa_name, lon_interfaz);
                interfaz[lon_interfaz] = '\0';
                return interfaz;
            }
            else return NULL;  
        }
    }
    
    return NULL;
}


/**
 *Obtiene la hora actual
 */
char *obtenerHora(void){
    
    time_t tiempoBruto;

    //Tiempo en segundos desde espoca (Enero 1 1970 00:00)    
    time(&tiempoBruto);
    
    struct tm *tiempo = localtime(&tiempoBruto);
    
    char *hora = malloc(sizeof(char)*12);
    char *tmp = malloc(sizeof(char)*3);
    
    memset(hora, 0, 12);
    
    //sprintf(hora, "%d:%d:%d", tiempo->tm_hour + ZONA_HORARIA_ACTUAL, tiempo->tm_min, tiempo->tm_sec);
    
    memset(tmp, 0, 3);
    
    
    if(tiempo == NULL){
        printf("ERROR: No se pudo obtener el tiempo actual.\n");
    }
    
    //Hora
    if(tiempo->tm_hour <= 9){
        strcat(hora, "0");        
        sprintf(tmp, "%d", tiempo->tm_hour); //+ ZONA_HORARIA_ACTUAL
        strcat(hora, tmp);
    }
    else{
            sprintf(tmp, "%d", tiempo->tm_hour);
            strcat(hora, tmp);
        
    }
    
    strcat(hora, ":");
    memset(tmp, 0, 3);
    
    //Minutos
    if(tiempo->tm_min <= 9){
        strcat(hora, "0");
        sprintf(tmp, "%d", tiempo->tm_min);
        strcat(hora, tmp);
    }
    else{
            sprintf(tmp, "%d", tiempo->tm_min);
            strcat(hora, tmp);
    }
    
    strcat(hora, ":");
    memset(tmp, 0, 3);
    
    //Segundos
    if(tiempo->tm_sec <= 9){
        strcat(hora, "0");
        sprintf(tmp, "%d", tiempo->tm_sec);
        strcat(hora, tmp);
    }
    else{
            sprintf(tmp, "%d", tiempo->tm_sec);
            strcat(hora, tmp);
    }
    
    free(tmp);
    return hora;
}

/**
 *Obtiene la fecha actual.
 *
 */
char *obtenerFecha(void){
    
    time_t tiempoBruto;

    //Tiempo en segundos desde espoca (Enero 1 1970 00:00)    
    time(&tiempoBruto);
    
    struct tm *tiempo = localtime(&tiempoBruto);
    
    char *fecha = malloc(sizeof(char)*12);
    char *tmp = malloc(sizeof(char)*5);
    
    memset(fecha, 0, 12);
    memset(tmp, 0, 5);
    
    //Anio
    sprintf(tmp, "%d", tiempo->tm_year + ANIO_INICIO_EPOCA);
    strcat(fecha, tmp);
    strcat(fecha, "-");
    
    //Mes
    memset(tmp, 0, 5);
    if(tiempo->tm_mon < 9){
        
        strcat(fecha, "0");
        sprintf(tmp, "%d", tiempo->tm_mon + 1);
        strcat(fecha, tmp);
    }
    else{
            sprintf(tmp, "%d", tiempo->tm_mon + 1);
            strcat(fecha, tmp);
    }
    strcat(fecha, "-");
    
    //dia
    memset(tmp, 0, 5);
    if(tiempo->tm_mday < 10){
        
        strcat(fecha, "0");
        sprintf(tmp, "%d", tiempo->tm_mday);
        strcat(fecha, tmp);
    }
    else{
            sprintf(tmp, "%d", tiempo->tm_mday);
            strcat(fecha, tmp);
    }
    
    free(tmp);
    return fecha;
}


/**
 *Obtiene el año actual
 */
char *obtenerAnio(void){
    
    time_t tiempoBruto;

    //Tiempo en segundos desde espoca (Enero 1 1970 00:00)    
    time(&tiempoBruto);
    
    struct tm *tiempo = localtime(&tiempoBruto);
    
    char *anio = malloc(sizeof(char)*10);
    
    if(anio == NULL){
        return NULL;
    }
    
    memset(anio, 0, 10);
    sprintf(anio, "%d", (tiempo->tm_year + ANIO_INICIO_EPOCA));
    
    return anio;
}

/**
 *Rutina para liberar arreglo de strings
 */
void liberarMemoria(void **bloque, int numeroElementos){
    int i = 0;
    
#ifdef DEBUG
    printf("%s - linea %d: numero elementos liberados: %d\n", __FILE__, __LINE__, numeroElementos);
#endif
    for(i = 0; i < numeroElementos; i++){
        free(bloque[i]);
        //printf("Liberado bloque %d: i\n", i);
    }
    
    free(bloque);
}



