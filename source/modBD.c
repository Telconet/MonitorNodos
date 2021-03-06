#include "modBD.h"



/**
 *Rutina que inserta un registro en la base de datos
 */
int insertarRegistro(char *nombreTabla, char **valores, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts) {
    
    int i;
    int cont;

    if (nombreTabla != NULL && valores != NULL && numeroValores > 0 && configuracion->ip_servidor_datos != NULL) {
        int sockfd = 0;
        char fromUser[300];
        
        memset(fromUser, 0, 300);

        struct sockaddr_in serv_addr;
        int conn;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Error : Could not create socket \n");
            close(sockfd);
            return -1;
        }

        strcpy(fromUser, valores[0]);
        for (i = 1; i < numeroValores; i++) {
            sprintf(fromUser, "%s,%s", fromUser, valores[i]);
        }
        sprintf(fromUser, "%s,%d,%d\n", fromUser,stp,sts);
        //printf("\nQuery: '%s'->%d\n", fromUser, strlen(fromUser));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(configuracion->puertoDatosServidor);
        serv_addr.sin_addr.s_addr = inet_addr(configuracion->ip_servidor_datos);       //172.40.0.10

        conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (conn < 0) {
            printf("\n Error : Connect Failed \n");
            close(sockfd);
            return -1;
        }

        cont = 0;
        
        while(cont < 3){                                                //Maximo 3 reintentos
            
            
            
            int status = write(sockfd, fromUser, strlen(fromUser));     //Enviamos los datos de mediciones
            
            if(status < 0){
                perror("ERROR: No se pudo enviar la informacion.\n");
            }
            else{
                
                close(sockfd);
                return 0;
            }
            
            cont++;
        }
        /*while (1) {
            
            //int status = system("date > /tmp/log_recv");
            //printf("system() status: %d", status);
            //fflush( stdout );
            n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
            //system("date >> /tmp/log_recv");
            
            if(n <= 0){
                //Coneccion cerrada.
                break;
            }
            else{
            
                recvBuff[n - 1] = '\0';
                if(strcmp(recvBuff, "Inicio")==0){
                    //system("date > /tmp/log_write1");
                    write(sockfd, fromUser, strlen(fromUser));
                    //system("date >> /tmp/log_write1");
                }else if(strcmp(recvBuff, "OK")==0){
                    break;
                }else{
                    cont++;
                    if(cont>3){
                        break;
                    }else{
                        //system("date > /tmp/log_write2");
                        write(sockfd, fromUser, strlen(fromUser));
                        //system("date >> /tmp/log_write2");
                    }
                }
            }
        }*/
        /*while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
            recvBuff[n - 1] = '\0';
            printf("\nRcv: '%s'", recvBuff);
            if(strcmp(recvBuff, "Inicio")==0){
               int res =  write(sockfd, fromUser, strlen(fromUser));
               printf("res: %d\n", res);
            }else if(strcmp(recvBuff, "OK")==0){
                break;
            }else{
                cont++;
                if(cont>3){
                    break;
                }else{
                    write(sockfd, fromUser, strlen(fromUser));
                }
            }
        }*/

        close(sockfd);
        return -1;
    }
    return -1;
}

/*almacena evento de puerta/rele en a base de datos*/

int insertarEvento(int id_nodo, char *fecha, char *hora, char *evento){
        
    int cont;

    if (evento != NULL && fecha != NULL && hora != NULL && configuracion->ip_servidor_datos != NULL) {
        int sockfd = 0;
        char fromUser[300];

        struct sockaddr_in serv_addr;
        int conn;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Error : Could not create socket \n");
            close(sockfd);
            return -1;
        }
        
        memset(fromUser, 0, 300);
        sprintf(fromUser, "%d,%s,%s,%s\n",id_nodo, fecha, hora, evento);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(configuracion->puertoDatosServidor);
        serv_addr.sin_addr.s_addr = inet_addr(configuracion->ip_servidor_datos);       //172.40.0.10

        conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (conn < 0) {
            printf("\n Error : Connect Failed \n");
            close(sockfd);
            return -1;
        }

        cont = 0;
        /*while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
            recvBuff[n - 1] = '\0';
            if(strcmp(recvBuff, "Inicio")==0){
                write(sockfd, fromUser, strlen(fromUser));
            }else if(strcmp(recvBuff, "OK")==0){
                break;
            }else{
                cont++;
                if(cont>3){
                    break;
                }else{
                    write(sockfd, fromUser, strlen(fromUser));
                }
            }
        }*/
        
        while(cont < 3){                                                //Maximo 3 reintentos
            
            int status = write(sockfd, fromUser, strlen(fromUser));     //Enviamos los datos de mediciones
            
            
            if(status < 0){
                perror("ERROR: No se pudo enviar la informacion.\n");
            }
            else{
                
                close(sockfd);
                return 0;
            }
            cont++;
        }

        close(sockfd);
        return -1;
    }
    return -1;
}


/**
 *Rutina que le el tipo de columnas. Usado para la creacion de tablas
 */
char **leerArchivoTipoColumnas(char *rutaArchivo, int *numeroColumnas) {

    FILE *archivo = fopen(rutaArchivo, "r");
    char linea[500];
    char **columnas = NULL;

    if (archivo != NULL) {

        //Primera linea es el numero de columans
        int numCol = atoi(fgets(linea, 500, archivo));

        if (numCol > 0) {
            *numeroColumnas = numCol;
            columnas = malloc(numCol * sizeof (*columnas)); //Asignamos memoria para los punteros

            int i = 0;

            //Las siguientes lineas contienen las columnas
            while (fgets(linea, 500, archivo) != NULL) {
                removerSaltoDeLinea(linea, 500); //Removemos el salto de linea, y lo cambiamos por el caracter nulo

                int longitudLinea = strlen(linea) + 1; //espacio para el caracter nulo
                columnas[i] = malloc(sizeof (char) *longitudLinea);

                strncpy(columnas[i], linea, longitudLinea);
                i++;
            }
            fclose(archivo);
            return columnas;
        }
        return NULL;
    } else {

        printf("ERROR: No se pudo abrir el archivo: %s\n", rutaArchivo);
        return NULL;
    }
}

/**
 *Rutina que verifica si el tipo de valor necesita apostro
 */
int necesitaApostrofe(char *tipo) {

    if (strstr(tipo, "text") != NULL || strstr(tipo, "char") != NULL
            || strstr(tipo, "time") != NULL || strstr(tipo, "date") || strstr(tipo, "blob") != NULL) {
        return 1;
    } else return 0;
}
