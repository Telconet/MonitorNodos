#include "modBD.h"

/**
 *Crear una conexion a una base de datos existente
 */
MYSQL *conectarBD(char *servidor, char *usuario, char *clave, char *BD) {

    MYSQL *conexion = mysql_init(NULL);

    //nadir , mysql_ssl_set(). Debe haber soporte ssl en la libreria.

    if (conexion == NULL) {
        printf("Error al abrir la base de datos.\n");

        //Removemos el archivo con el pid    
        /*char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);*/

        FILE *archivoErrorBD = fopen("/home/root/MonitorNodo/erroBD.txt", "a");
        fputs("No se pudo inicializar MySQL\n", archivoErrorBD);
        fclose(archivoErrorBD);


        return NULL;
    }

    if (mysql_real_connect(conexion, servidor, usuario, clave, BD, configuracion->puertoBD, NULL, 0) == NULL) {
        printf("Error %u: %s\n", mysql_errno(conexion), mysql_error(conexion));

        //Removemos el archivo con el pid    
        /*char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
        strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
        strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
        system(comandoRemover);*/

        FILE *archivoErrorBD = fopen("/home/root/MonitorNodo/erroBD.txt", "a");
        const char *error = mysql_error(conexion);
        fputs(error, archivoErrorBD);
        fclose(archivoErrorBD);

        return NULL;
    }

    return conexion;
}

/**
 *Cierra la conexion a la base de datos;
 */
int cerrarBD(MYSQL **conexion) {
    if (conexion != NULL) {
        mysql_close((*conexion));
    }
    return 0;
}

/**
 *Rutina que crea una tabla, asumiendo que la conexion ya este abierta
 */
int crearTabla(MYSQL **conexion, char *nombreTabla, char **tipoColumnas, int numeroColumnas) {

    if ((*conexion) != NULL && tipoColumnas != NULL && numeroColumnas > 0) {
        char stringConsulta[BUFFER_CONSULTA_MAX];

        //Ponemos en cero el string de la consulta
        memset(stringConsulta, 0, BUFFER_CONSULTA_MAX);

        /*snprintf(stringConsulta, BUFFER_CONSULTA_MAX, "CREATE TABLE ("
                                                      "";*/
        int numeroCaracteres = 0;


        numeroCaracteres = strlen(stringConsulta) + strlen("CREATE TABLE ") + strlen(" (");

        if (numeroCaracteres < BUFFER_CONSULTA_MAX - 1) {
            strcat(stringConsulta, "CREATE TABLE ");
            strcat(stringConsulta, nombreTabla);
            strcat(stringConsulta, " (");
        } else {
            printf("ERROR: Datos de entrada de la consulta SQL son mas grandes que el tamaño del buffer.\n");
            return -1; //Se nos acabo el buffer
        }

        int i;

        //Aqui revisamos que el resto de argumentos no exceda el tamaño del buffer
        for (i = 0; i < numeroColumnas; i++) {
            numeroCaracteres += strlen(tipoColumnas[i]) + 1;
        }

        numeroCaracteres += strlen("ENGINE=INNODB") + strlen("\0");

        if (numeroCaracteres < BUFFER_CONSULTA_MAX) {

            for (i = 0; i < numeroColumnas; i++) {
                strcat(stringConsulta, tipoColumnas[i]);

                if (i < numeroColumnas - 1) {
                    strcat(stringConsulta, ", ");
                } else {
                    strcat(stringConsulta, ")");
                }
            }
            strcat(stringConsulta, "ENGINE=MYISAM");
            strcat(stringConsulta, "\0");
        } else {
            printf("ERROR: Datos de entrada de la consulta SQL son mas grandes que el tamaño del buffer.\n");
            return -1; //Se nos acabo el buffer
        }
#ifdef DEBUG
        printf("numero columnas crear tabla: %d\n", numeroColumnas);
        printf("\n");
        printf("Consulta: %s\n\n", stringConsulta);
        printf("Longitud consulta: %u\n", strlen(stringConsulta));
#endif

        if (mysql_query((*conexion), stringConsulta)) {
#ifdef DEBUG
            printf("Error %u: %s\n", mysql_errno((*conexion)), mysql_error((*conexion)));
            return -1;
#endif
        }

        return 1;
    }
    return -1;
}

/**
 *Rutina que elimina un tabla de la base de datos
 */
int eliminarTabla(MYSQL **conexion, char *nombreTabla) {

    if ((*conexion) != NULL & nombreTabla != NULL) {
        char stringConsulta[BUFFER_CONSULTA_MAX];

        //Ponemos en cero el string de la consulta
        memset(stringConsulta, 0, BUFFER_CONSULTA_MAX);

        snprintf(stringConsulta, BUFFER_CONSULTA_MAX, "DROP TABLE %s", nombreTabla);

        /*strcat(stringConsulta, "DROP TABLE ");
        strcat(stringConsulta, nombreTabla);*/

        if (mysql_query((*conexion), stringConsulta)) {
            printf("Error %u: %s\n", mysql_errno((*conexion)), mysql_error((*conexion)));
            return -1;
        }

        return 1;
    }
    return -1;
}

/**
 *Rutina que inserta un registro en la base de datos
 */
int insertarRegistro(char *nombreTabla, char **valores, int numeroValores, status_puerto_DIO stp, status_puerto_DIO sts) {
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
            return 1;
        }

        strcpy(fromUser, valores[0]);
        for (i = 1; i < numeroValores; i++) {
            sprintf(fromUser, "%s,%s", fromUser, valores[i]);
        }
        sprintf(fromUser, "%s,%d,%d\n", fromUser,stp,sts);
        //printf("\nQuery: '%s'->%d\n", fromUser, strlen(fromUser));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(5000);
        serv_addr.sin_addr.s_addr = inet_addr("172.40.0.10");

        conn = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
        if (conn < 0) {
            printf("\n Error : Connect Failed \n");
            return 1;
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
/*
        cont = 0;
        do {
            write(sockfd, fromUser, strlen(fromUser));
            n = read(sockfd, recvBuff, sizeof (recvBuff) - 1);
            recvBuff[n - 1] = '\0';
            printf("\nRcv: %s", recvBuff);
            cont++;
        } while (strcmp(recvBuff, "OK") != 0 && cont < 3);
*/

        /*
                i = 1;
                while ((n = read(sockfd, recvBuff, sizeof (recvBuff) - 1)) > 0) {
                    recvBuff[n] = '\0';
                    printf("\nRcv: %s", recvBuff);
                    if (strcmp(recvBuff, "Inicio") == 0) {
                        write(sockfd, valores[0], strlen(valores[0]));
                    } else if (strcmp(recvBuff, "Next") == 0) {
                        if (i == numeroValores) {
                            write(sockfd, "FIN", 3);
                        } else {
                            write(sockfd, valores[i], strlen(valores[i]));
                            i++;
                        }
                    } else if (strcmp(recvBuff, "OK") == 0) {
                        break;
                    }
                }
         */

        return 1;
    }
    return -1;
}

int insertarRegistro2(MYSQL **conexion, char *nombreTabla, char **valores, int numeroValores) {

    if ((*conexion) != NULL & nombreTabla != NULL && valores != NULL && numeroValores > 0) {

        char stringConsulta[BUFFER_CONSULTA_MAX];

        //Ponemos en cero el string de la consulta
        memset(stringConsulta, 0, BUFFER_CONSULTA_MAX);

        int numeroCaracteres = strlen("INSERT INTO ") + strlen(nombreTabla) + strlen(" (");

        if (numeroCaracteres < BUFFER_CONSULTA_MAX) {
            strcat(stringConsulta, "INSERT INTO ");
            strcat(stringConsulta, nombreTabla);
            strcat(stringConsulta, " (");
        } else {
            printf("ERROR: Datos de entrada de la consulta SQL son mas grandes que el tamaño del buffer.\n");
            return -1; //Se nos acabo el buffer
        }


        int i = 0;
        int numeroColumnas = 0;
        int numTipoColumnas = 0;

        //Obtenemos el numero de columnas
        char **columnas = obtenerNombreColumnas(conexion, nombreTabla, &numeroColumnas); //?? posible falla?
        char **tipoColumnas = obtenerTipoColumnas(conexion, nombreTabla, &numTipoColumnas); //posible falla?

        if (columnas == NULL) {
            liberarMemoria(columnas, numeroColumnas);
            liberarMemoria(tipoColumnas, numTipoColumnas);
            return -1;
        }

        if (tipoColumnas == NULL) {
            liberarMemoria(columnas, numeroColumnas);
            liberarMemoria(tipoColumnas, numTipoColumnas);
            return -1;
        }

        //Seguimos contando los caracteres
        for (i = 0; i < numeroColumnas; i++) { //numeroColumnas??
            numeroCaracteres += strlen(columnas[i]) + 1; //-> SEG FAULT!!!
        }

        numeroCaracteres += strlen(") VALUES (");

        if (numeroCaracteres < BUFFER_CONSULTA_MAX) {
            //Creamos el string de la consulta
            for (i = 0; i < numeroColumnas; i++) {

                strcat(stringConsulta, columnas[i]);

                if (i < numeroColumnas - 1) {
                    strcat(stringConsulta, ", ");
                } else {
                    strcat(stringConsulta, ") VALUES (");
                }
            }
        } else {
            printf("ERROR: Datos de entrada de la consulta SQL son mas grandes que el tamaño del buffer.\n");
            return -1; //Se nos acabo el buffer
        }


        //Seguimos contando los caracteres
        for (i = 0; i < numeroValores; i++) {
            numeroCaracteres += strlen(valores[i]) + 2;
        }

        if (numeroCaracteres < BUFFER_CONSULTA_MAX) { //Seg fault?

            //Concatenamos los valores a ser ingresados
            for (i = 0; i < numeroValores; i++) {

#ifdef DEBUG
                printf("%s - linea %d: tipo de columna: %s\n", __FILE__, __LINE__, tipoColumnas[i]); //no se muestran los tipos de columna?
#endif
                if (necesitaApostrofe(tipoColumnas[i])) {
                    strcat(stringConsulta, "\'");
                    strcat(stringConsulta, valores[i]);
                    strcat(stringConsulta, "\'");
                } else strcat(stringConsulta, valores[i]);

                if (i < numeroValores - 1) {
                    strcat(stringConsulta, ", ");
                } else {
                    strcat(stringConsulta, ")");
                }
            }
        } else {
            printf("ERROR: Datos de entrada de la consulta SQL son mas grandes que el tamaño del buffer.\n");
            return -1; //Se nos acabo el buffer
        }

#ifdef DEBUG
        printf("Consulta: %s\n", stringConsulta);
        printf("Longitud consulta: %d\n", strlen(stringConsulta));
#endif
        //Ejecutamos la consulta
        if (mysql_query((*conexion), stringConsulta)) {
            printf("Error %u: %s\n", mysql_errno((*conexion)), mysql_error((*conexion)));
            return -1;
        }


        /*for(i = 0; i < numeroColumnas; i++){
           free(columnas[i]);
        }
        free(columnas);*/
        liberarMemoria(columnas, numeroColumnas);
#ifdef DEBUG
        printf("%s - linea %d: Memoria de columnas liberada.\n", __FILE__, __LINE__);
#endif
        /*for(i = 0; i < numTipoColumnas; i++){
           free(tipoColumnas[i]);
        }
        free(tipoColumnas);*/
        liberarMemoria(tipoColumnas, numTipoColumnas);
#ifdef DEBUG
        printf("%s - linea %d: Memoria de tipo de columnas liberada.\n", __FILE__, __LINE__);
#endif
        return 1;
    }
    return -1;
}

/**
 *Esta rutina lee las columnas de una tabla
 */
char **obtenerNombreColumnas(MYSQL **conexion, char *nombreTabla, int *numeroColumnas) {

    if ((*conexion) != NULL && nombreTabla != NULL) {
        char stringConsulta[BUFFER_CONSULTA_MAX];

        //Ponemos en cero el string de la consulta
        memset(stringConsulta, 0, BUFFER_CONSULTA_MAX);

        snprintf(stringConsulta, BUFFER_CONSULTA_MAX, "SELECT * FROM %s LIMIT 0,2", nombreTabla);

        /*strcat(stringConsulta, "SELECT * FROM ");
        strcat(stringConsulta, nombreTabla);
        strcat(stringConsulta, " LIMIT 0,2");*/

        if (mysql_query((*conexion), stringConsulta)) {
            printf("Error %u: %s\n", mysql_errno((*conexion)), mysql_error((*conexion)));
            return NULL;
        }

        //Obtenemos el resultado de la consulta        
        MYSQL_RES *resultado = mysql_store_result((*conexion));
        int numero_campos = mysql_num_fields(resultado);

        //Crear el arreglo con los nombres de las columnas
        char **nombreColumnas = malloc(numero_campos * sizeof (*nombreColumnas));

        if (nombreColumnas != NULL) {

            MYSQL_FIELD *campo;
            int i = 0;

            while (campo = mysql_fetch_field(resultado)) {
#ifdef DEBUG                
                printf("%s\n", campo->name);
#endif
                int lonNombre = strlen(campo->name);
                nombreColumnas[i] = malloc((sizeof (char) *lonNombre) + 1); //+1?

                if (nombreColumnas[i] == NULL) return NULL; //hubo un fallo grave

                //Ponemos en cero los contenidos de la memoria
                memset(nombreColumnas[i], 0, lonNombre); //strncpy error?

                //copiamos el texto
                strcpy(nombreColumnas[i], campo->name);
                i++;
            }

            *numeroColumnas = i; //devolvemos el numero de columnas

            mysql_free_result(resultado); //liberams la memoria?
            return nombreColumnas;
        }

#ifdef DEBUG
        printf("Numero de campos de la tabla: %d\n", numero_campos);
#endif
    }

    return NULL;
}

/**
 *Rutina que obtiene el tipo de de las columnas de la tabla
 */
char **obtenerTipoColumnas(MYSQL **conexion, char *nombreTabla, int *numeroColumnas) {

    if ((*conexion) != NULL && nombreTabla != NULL) {
        char stringConsulta[BUFFER_CONSULTA_MAX];

        //Ponemos en cero el string de la consulta
        memset(stringConsulta, 0, BUFFER_CONSULTA_MAX);

        snprintf(stringConsulta, BUFFER_CONSULTA_MAX, "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = \'%s\'", nombreTabla);

        /*strcat(stringConsulta, "SELECT COLUMN_TYPE FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = \'");
        strcat(stringConsulta, nombreTabla);
        strcat(stringConsulta, "\'");
         */

        if (mysql_query((*conexion), stringConsulta)) {
            printf("Error %u: %s\n", mysql_errno((*conexion)), mysql_error((*conexion)));
            return NULL;
        }

        //Obtenemos el resultado de la consulta        
        MYSQL_RES *resultado = mysql_store_result((*conexion));

        int numero_campos = mysql_num_fields(resultado);
        int numero_filas = mysql_num_rows(resultado); //numero de filas

#ifdef DEBUG     
        printf("Numero de campos: %d\n", numero_campos);
        printf("Numero de filas: %d\n", numero_filas);
#endif

        //Crear el arreglo con los nombres de las columnas
        //char **nombreColumnas = malloc(numero_filas*sizeof(*nombreColumnas));              //EPIC FAIL!!!!!!! numero_campos = 1!!!

        char **nombreColumnas = malloc(numero_filas * sizeof (char *));

        if (nombreColumnas != NULL) {

            MYSQL_ROW fila;
            int i = 0;

            while (fila = mysql_fetch_row(resultado)) { //???   REVISAR

                unsigned long *lonNombre = mysql_fetch_lengths(resultado);

                nombreColumnas[i] = strdup(fila[0]); //DUP es fundamental!!!!!

                //nombreColumnas[i] = malloc((sizeof(char *)));               //Si no asignamos, tenemor un error donde no retorna los strings.    
                //if(nombreColumnas[i] == NULL) return NULL;                  //hubo un fallo grave

                //char *tmpPtr = nombreColumnas[i];                           //no perdemos la referencia a la memoria?? REVISAR Y PROBAR MINUCIOSAMENTE

                //copiamos el texto
                //char *w = strdup(fila[0]);                                  //Error al usar free (memoria corrompida)

                //nombreColumnas[i] = w;                                      //aqui se pierde la referencia al malloc anterior inmediato.

                //free(tmpPtr);                                               //Pero como tenemos otra referencia, lo recuperamos. REVISAR. POSIBLE CAUSA DE ERROR. PROBAR MINUCIOSAMENTE.
#ifdef DEBUG
                printf("%s - linea %d: Tipo columna %d: %s\n", __FILE__, __LINE__, i, nombreColumnas[i]);
#endif
                i++;
            }

            *numeroColumnas = i; //devolvemos el numero de columnas
            mysql_free_result(resultado); //liberamos la memoria?
            return nombreColumnas;
        }

#ifdef DEBUG
        //printf("Numero de campos de la tabla: %d\n", numero_campos);
#endif
    }

    return NULL;
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
