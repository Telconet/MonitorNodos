#include "modBD.h"



/**
 *Rutina que inserta un registro en la base de datos
 */
int insertarRegistro(char *nombreTabla, char **valores, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts) {
    /*int i;
    int cont;
    
    int longitudFromUser  = 300;
    int longitudrecvBuff = 1024;

    if (nombreTabla != NULL && valores != NULL && numeroValores > 0) {
        int sockfd = 0, n;
        char fromUser[longitudFromUser];
        char recvBuff[longitudrecvBuff];

        struct sockaddr_in serv_addr;
        int conn;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("\n Error : No se pudo crear el socket\n");
            return -1;
        }
        
        //llenamos los buffers de ceros.
        memset(fromUser, 0, longitudFromUser);
        memset(recvBuff, 0, longitudrecvBuff);
        

        strcpy(fromUser, valores[0]);
        for (i = 1; i < numeroValores; i++) {
            snprintf(fromUser, longitudFromUser, "%s,%s", fromUser, valores[i]);
        }
        snprintf(fromUser, longitudFromUser, "%s,%d, %d\n", fromUser, stp, sts);


        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(5000);
        serv_addr.sin_addr.s_addr = inet_addr("172.40.0.10");

        conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (conn < 0) {
            perror("\n Error : Conexion fallida\n");
            close(sockfd);
            return -1;
        }

        cont = 0;
        while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
            recvBuff[n - 1] = '\0';
            printf("\nRcv: '%s'", recvBuff);
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
        }
        close(sockfd);
        return 0;
    }
    return -1;*/
    
    int i;
    int cont;

    if (nombreTabla != NULL && valores != NULL && numeroValores > 0) {
        int sockfd = 0, n;
        char fromUser[300];
        char recvBuff[1024];

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
        serv_addr.sin_port = htons(5000);
        serv_addr.sin_addr.s_addr = inet_addr("172.40.0.10");       //172.40.0.10

        conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (conn < 0) {
            printf("\n Error : Connect Failed \n");
            close(sockfd);
            return -1;
        }

        cont = 0;
        while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
            recvBuff[n - 1] = '\0';
            printf("\nRcv: '%s'", recvBuff);
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
        }

        close(sockfd);
        return 0;
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
