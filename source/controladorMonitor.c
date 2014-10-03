#include "controladordef.h"


/**
 *Programa principal
 *
 */
int main(){
    
    //int primeraInicializacion = 0;              Aqui reportamos el PID al thread del daemon
    char entrada[MAX_TAMANO_ENTRADA];
    
    //Configurampos el directorio de trabajo
    chdir(DIRECTORIO_DE_TRABAJO);
    
    daemonPID = buscarArchivoPIDProceso(ARCHIVO_PROCESS_ID_DAEMON);
    //printf("hola\n");
    
    //Inicio
    int tmpPID = getpid();
    printf("\n");
    printf("Bienvenido al programa de control del sistema de monitoreo (PID %d).\n", tmpPID);
    
    if(tmpPID < 1000){
        printf("-----------------------------------------------------------------------\n\n");
    }
    else if(tmpPID < 10000){
        printf("----------------------------------------------------------------------\n\n");
    }
    else printf("----------------------------------------------------------------------\n\n");
    
    printf("Se encontro el daemon de monitoreo con PID %d.\n", daemonPID);
    
    //Obtenemos el socket para mandar los comandos al daemon
    struct sockaddr_un local;
    int tamano_local;
    char mensaje[6];

    //Obtenemos un socket
    sd1 = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(sd1 == -1){
        perror("socket cliente");
        
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(1);
    }  
    
    printf("Conectando al daemon de monitoreo... ");
    
    //asociamos el socket descriptor 1 a el archivo
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, ARCHIVO_SOCKET_IPC);
    tamano_local = strlen(local.sun_path) + sizeof(local.sun_family);
    
    if( connect(sd1, (struct sockaddr *)&local, tamano_local) == -1){
        perror("error al conectar al socket");
        
        //Removemos el archivo con el pid    
        char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);
        exit(0);
    }
    
    //enviamos el PID
    sprintf(mensaje,"%u", getpid());
    int be = send(sd1, mensaje, strlen(mensaje), 0);
    printf("Conectado.\n");
    
    //Empezamos a leer entrada de usurio
    while(1){
        
        printf("Ingrese el comando: ");
        fgets(entrada, MAX_TAMANO_ENTRADA, stdin);
        
        if(!strcmp("salir\n",entrada))   //resultado de la comparacion es 0, strings son iguales
        {
            salir(0);
        }
        else{

            //Enviamos el comando, si es válido.
            struct comando mi_comando;
            int com_res = crearComando(entrada, &mi_comando);
            
            if(com_res == 1){
                
                int n = enviarDatos(sd1, &mi_comando, COMANDO);
                
                //TODO: Liberar argumentos de comando
                
                //Esperamos la respuesta
                struct respuesta res;            
                recibirDatos(sd1, &res, RESPUESTA);
                   
                //Mostramos la respuesta
                printf("%s: \n", traducirStatusRespuesta(res.status));
                printf("%s\n", res.res);
                
                //Caso especial -> cuando cerramos el monitor, notificamos y salimos
                if(strstr(res.res, "monitor de nodos ha sido cerrado") != NULL){
                    printf("\b ");
                    printf("El controlador del monitor se cerrara.\n");
                    free(res.res);
                    salir(0);
                }
                
                free(res.res);
            }
            else if(com_res == -1){
                printf("ERROR:\nComando invalido.\n");
            }
            else if(com_res == -2){
                printf("ERROR:\nFaltan argumentos en el comando.\n");
            }
            
        }
    }
}
