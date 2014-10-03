#include "IPC.h"

/**
 *Rutina para envio de respuesta o comnado a traves del socket. Se enviara
 *el valor proporcionado por el que llame esta rutina.
 */
int enviarDatos(int des_archivo, void *valor, int tipo){
    
    if(des_archivo == -1){
        printf("what 1\n");
        return -1;
    }
    
    if(valor == NULL){
        printf("what 2\n");
        return -1;
    }

    //Dependiendo del tipo, manejamos de distinto modo
    if(tipo == COMANDO){
        
        int comando_int = COMANDO;
        struct comando *com_local = (struct comando *)valor;
        
        //Le decimos al daemon que vamos a enviar un comando
        int n = send(des_archivo, &comando_int, sizeof(comando_int), 0);
        
                
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        //enviamos cada uno de los campos de la estructura
        //comando
        n = send(des_archivo, &(com_local->com),sizeof(com_local->com), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        //numero argumentos
        n = send(des_archivo, &(com_local->numero_argumentos), sizeof(com_local->numero_argumentos), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        //Longitud de argumentos        
        int j = 0;
        for(j = 0; j < com_local->numero_argumentos; j++){
            
            int lonarg = com_local->long_argumentos[j];
            
            n = send(des_archivo, &lonarg, sizeof(lonarg), 0);
            
            if( n == -1){
                perror("Fallo envio de mensaje\n");
                return -1;
            }
        }
        
        //argumentos
        //Enviar strings uno por uno
        int i = 0;
        
#ifdef DEBUG
        //printf("%s %d: Numero de argumentos %d\n", __FILE__, __LINE__, com_local->numero_argumentos);
#endif
        
        for(i = 0; i < com_local->numero_argumentos; i++){
            
            n = send(des_archivo, com_local->argumentos[i], strlen(com_local->argumentos[i]), 0);
            //printf("Bytes enviados: %d\n", n);
                                 
            if( n == -1){
                perror("Fallo envio de mensaje\n");
                return -1;
            }
        }        
    }
    else if(tipo == RESPUESTA){
        
        //enviamos una estructura tipo respuesta
        
        int respuesta_int = RESPUESTA;
        struct respuesta *res_local = (struct respuesta *)valor;
        
        //Le decimos al proceso que vamos a enviar una respuesta
        //Le decimos al daemon que vamos a enviar un comando
        int n = send(des_archivo, &respuesta_int, sizeof(respuesta_int), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        
        //Enviar el id de la respuesta
        //enviamos cada uno de los campos de la estructura
        //comando
        n = send(des_archivo, &(res_local->status), sizeof(res_local->status), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        //printf("Respuesta STATUS: %d\n", OK);
        
        //Enviamos la longituda de la respuesta
        n = send(des_archivo, &(res_local->long_res), sizeof(res_local->long_res), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        //printf("LONGITUD: respuesta %d\n", res_local->long_res);
        //Enviar el string del mensaje
        //char *buff = malloc(res_local->long_res*sizeof(char));
        
        n = send(des_archivo, res_local->res, strlen(res_local->res), 0);
                     
        if( n == -1){
            perror("Fallo envio de mensaje\n");
            return -1;
        }
        
        return 1;
    } 
}



/**
 *Rutina para recibir comandos y respuestas. La respuesta se almacenara en
 *en buffer proporcionado
 */
int recibirDatos(int des_archivo, void *buffer ,int tipo){
    
    int n;
    
    if(des_archivo == -1){
        return -1;
    }
    
    if(tipo == COMANDO){
        
        //declaramos la estructura para recibir el resto del comando
        struct comando *com_local = (struct comando *)buffer;
                
        //La notificacion ya la recibimos, a partir de aqui recibimos los parametros
        //comando
        n = recv(des_archivo, &(com_local->com), sizeof(com_local->com), 0);
                     
        if( n == -1){
            perror("Fallo recepcion de mensaje\n");
            return -1;
        }
#ifdef DEBUG
        else printf("recibi el comando %d\n", com_local->com);
#endif
        
        //numero de argumentos
        n = recv(des_archivo, &(com_local->numero_argumentos), sizeof(com_local->numero_argumentos), 0);
                     
        if( n == -1){
            perror("Fallo recepcion de mensaje\n");
            return -1;
        }
#ifdef DEBUG
        else printf("recibi el numero de argumentos %d\n", com_local->numero_argumentos);
#endif
        
        //Longitud de argumentos
        com_local->long_argumentos = malloc((com_local->numero_argumentos) * (sizeof(int)));
        
        int j = 0;
        for(j = 0; j < com_local->numero_argumentos; j++){
            
            int lon;
            
            n = recv(des_archivo, &lon, sizeof(int), 0);
            com_local->long_argumentos[j] = lon;
            
#ifdef DEBUG
            printf("Longitud de argumento %d es %d\n", j, com_local->long_argumentos[j]);
#endif            
            if( n == -1){
                perror("Fallo recepcion de mensaje\n");
                return -1;
            }
        }
        
        //argumentos
        //Enviar strings uno por uno
        int i = 0;
        com_local->argumentos = malloc(com_local->numero_argumentos * sizeof(char*));     //asignamos memoria para los punteros
        
        for(i = 0; i < com_local->numero_argumentos; i++){
            
            char *buff = malloc((com_local->long_argumentos[i] + 1)*sizeof(char));            
            memset(buff, 0, (com_local->long_argumentos[i] + 1));                          //AGREGADO
            n = recv(des_archivo, buff, com_local->long_argumentos[i], 0);
            com_local->argumentos[i] = buff;
            
#ifdef DEBUG
            printf("Argumentos %d es %s\n", i, com_local->argumentos[i]);
#endif            
            if( n == -1){
                perror("Fallo envio de mensaje\n");
                return -1;
            }
        }    
    }
    else if(tipo == RESPUESTA){
        
        int w;
        n = recv(des_archivo, &w, sizeof(w), 0);
        
        //Recibimos una respuesta
        //declaramos la estructura para recibir el resto del comando
        struct respuesta *res_local = (struct respuesta *)buffer; 
            
        printf("Esperando respuesta...\n");
        
        //La notificacion ya la recibimos, a partir de aqui recibimos los parametros
        //comando
        n = recv(des_archivo, &(res_local->status),sizeof(res_local->status), 0);
                     
        if( n == -1){
            perror("Fallo recepcion de mensaje");
            return -1;
        }
#ifdef DEBUG
        //else printf("Recibi respuesta %d\n", res_local->status);
#endif
        
        //numero de argumentos
        n = recv(des_archivo, &(res_local->long_res), sizeof(res_local->long_res), 0);
                     
        if( n == -1){
            perror("Fallo recepcion de mensaje\n");
            return -1;
        }

#ifdef DEBUG
        //else printf("recibi la longitud de respuesta %d\n", res_local->long_res);
#endif
        
        //Recibir el buffer con la respuesta
        char *buff = malloc((res_local->long_res + 1)*sizeof(char));            
        memset(buff, 0, (res_local->long_res + 1));
        n = recv(des_archivo, buff, res_local->long_res, 0);
        res_local->res = buff;
            
#ifdef DEBUG
        //printf("Respuesta: %s\n", res_local->res);
#endif            
        if( n == -1){
            perror("Fallo recepcion de mensaje\n");
            return -1;
        }
    }
    return 1;    
}

