#include "controladordef.h"


/**
 *Rutina que hara limpieza de arhivos/proceso al salir
 */
void salir(int status){
    printf("Saliendo...\n");
    close(sd1);
    exit(status);  
}

//BORRAR?
void enviarSenalProceso(int senal, int pid){
    
    if(kill(pid, senal) != 0){
        printf("Informacion: El proceso o daemon PID %d no esta corriendo.\n", pid);
    }
}


/**
 *Esta rutina recibe el string del usuario, lo analiza, y crea el comando correspondiente
 */
int crearComando(char *comando_usuario, struct comando *com){
    
    if(comando_usuario == NULL || com == NULL){
        return -1;
    }
    else{        
        if(strcmp(SALIR_DAEMON_STR, comando_usuario) == 0){
            com->com = SALIR_DAEMON;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(INFORMACION_NODO_STR, comando_usuario) == 0){
            com->com = INFORMACION_NODO;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(INFORMACION_CONF_STR, comando_usuario) == 0){
            com->com = INFORMACION_CONF;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(INFORMACION_VALORES_MIN_STR, comando_usuario) == 0){
            com->com = INFORMACION_VAL_MIN;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(MOSTRAR_MEDICIONES_STR, comando_usuario) == 0){
            com->com = MOSTRAR_MEDICIONES;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(AYUDA_STR, comando_usuario) == 0){
            com->com = AYUDA;
            com->numero_argumentos = 0;
            com->long_argumentos = NULL;
            com->argumentos = NULL;
            return 1;
        }
        else if(strcmp(CAMBIAR_MINIMOS_STR, comando_usuario) == 0){
            com->com = CAMBIAR_MINIMOS;
            
            int maxCharListaMinimos = 256;
            
            //TODO obtener los minimos
            //int indiceMinimos = 0;
            int tamanoEntrada = 10;
            char *listaminimos = malloc(sizeof(char)*maxCharListaMinimos);
            memset(listaminimos, 0, maxCharListaMinimos);
            char entrada[tamanoEntrada];
            
            if(listaminimos == NULL){
                return -1;
            }
            
            //Empezamos a obtener los datos en orden
            int i = 0;
            for(i = 0; i < NUMERO_MEDICIONES_ADC; i++){
                
                switch(i){
                    case 0:
                        printf("Valor minimo corriente DC 1: ");
                        break;
                    case 1:
                        printf("Valor minimo temperatura 1: ");
                        break;
                    case 2:
                        printf("Valor minimo corriente AC 3: ");
                        break;
                    case 3:
                        printf("Valor minimo voltaje DC 2: ");
                        break;
                    case 4:
                        printf("Valor minimo corriente DC 2: ");
                        break;
                    case 5:
                        printf("Valor alerta combustible (siempre debe ser 2): ");
                        break;
                    case 6:
                        printf("Valor minimo corriente AC 4: ");
                        break;
                    case 7:
                        printf("Valor minimo voltaje DC 3: ");
                        break;
                    case 8:
                        printf("Valor minimo corriente DC 3: ");
                        break;
                    case 9:
                        printf("Valor alerta generador (siempre debe ser 2): ");
                        break;
                    case 10:
                        printf("Valor minimo voltaje DC 4: ");
                        break;
                    case 11:
                        printf("Valor minimo corriente DC 4: ");
                        break;
                    case 12:
                        printf("Valor minimo voltaje AC 1: ");
                        break;
                    case 13:
                        printf("Valor maximo humedad: ");
                        break;
                    case 14:
                        printf("Valor minimo corriente AC 1: ");
                        break;
                    case 15:
                        printf("Valor minimo voltaje AC 2: ");
                        break;
                    case 16:
                        printf("Valor minimo corriente AC 2: ");
                        break;
                    case 17:
                        printf("Valor minimo voltaje DC 1: ");
                        break;
                    default:
                        break;
                }
                
                //Leemos el valor
                memset(entrada, 0, tamanoEntrada);
                fgets(entrada, tamanoEntrada, stdin);
                
                int w = 0;
                for(w = 0; w < tamanoEntrada; w++){
                    
                    if(!(entrada[w] == 48 || entrada[w] == 49 || entrada[w] == 50|| entrada[w] == 51 || entrada[w] == 52 || entrada[w] == 53
                       || entrada[w] == 54 || entrada[w] == 55 || entrada[w] == 56 || entrada[w] == 57 || entrada[w] == 45 || entrada[w] == 46)){
                        
                        //printf("caracter invalido: %d %c\n", entrada[w], entrada[w]);
                        entrada[w] = 0;
                        
                    }
                }
                
                if(i == NUMERO_MEDICIONES_ADC - 1){
                    strncat(listaminimos, entrada, 6);                  //Solo llenamos 4 digitos más el numero decimal.
                }
                else{
                    strncat(listaminimos, entrada, 6);
                    strncat(listaminimos, ";", 1);
                }
                printf("\n");
            }
            
            com->numero_argumentos = 1;
            com->long_argumentos = malloc(sizeof(int)*com->numero_argumentos);
            com->argumentos = malloc(sizeof(char *)*com->numero_argumentos);
            
            com->argumentos[0] = listaminimos;
            com->long_argumentos[0] = strlen(listaminimos);
            printf("%s\n",listaminimos);
            //exit(0);            //REMOVER, solo para pruebas.
            return 1;
        }
        else if(strstr(comando_usuario, ACTIVAR_RELE_STR) != NULL || strstr(comando_usuario, DESACTIVAR_RELE_STR) != NULL){
            
            if(strstr(comando_usuario, DESACTIVAR_RELE_STR) != NULL){
                com->com = DESACTIVAR_RELE;
            }
            else com->com = ACTIVAR_RELE;
            
            
            com->numero_argumentos = 1;             //unico argumento
            com->long_argumentos = malloc(sizeof(int)*com->numero_argumentos);
            com->argumentos = malloc(sizeof(char *)*com->numero_argumentos);
            
            if(com->long_argumentos == NULL || com->argumentos == NULL){
                return -1;
            }
            
            //Primer argumento es el numero de rele
            com->argumentos[0] = extraerArgumento(comando_usuario, 2, &com->long_argumentos[0]);
            
            if(com->argumentos[0] == NULL){
                return -2;  
            }
            return 1;
        }
        else{
            return -1;         //comando invalido
        }
    } 
}

/**
 *Esta rutina extrae un argumento del comando (si este existiese).
 *El parametro numero_argumento indica cual arguemento debe ser extraido
 *Los argumentos estan separados por un espacio o un tab
 */
char *extraerArgumento(char *comando_usuario, int numero_de_argumento, int *longitud_argumento){
    int i = 0;
    int numero_espacios = 0;
    int lon_com = strlen(comando_usuario);
    char *ptr_str = comando_usuario;
    
    for(i = 0; i < lon_com; i++){
        
        if(*comando_usuario == ' ' || *comando_usuario == '\t'){
            if(i > 0 && i < lon_com - 2 &&                                      //para evitar overflows
               (*(comando_usuario+1) != ' ' || *(comando_usuario+1) != '\t') &&
               (*(comando_usuario-1) != ' ' || *(comando_usuario-1) != '\t')){
                numero_espacios++;
                //printf("Numero de espacios %d\n", numero_espacios);
            }
        }
        
        if(numero_espacios == numero_de_argumento){
            
            comando_usuario++;
            i++;
            break;
        }
        comando_usuario++;
    }

    
    if(numero_espacios < numero_de_argumento){
        return NULL;                //no se encontro el argumento requerido.
    }
    else{
        //Aqui comando usuario ya apunta al inicio
        int j;
        int lon_arg = 0;
        int lon_sep = strlen(comando_usuario);
        char *inicio_argumento = comando_usuario;

#ifdef DEBUG    
        printf("%s\n", comando_usuario);
        printf("%d\n", lon_sep);
#endif
        
        for(j = 0; j < lon_sep; j++){

#ifdef DEBUG
            printf("j - %d: %c\n", j, *comando_usuario);
#endif            
            if(*comando_usuario == ' ' || *comando_usuario == '\t' || *comando_usuario == '\n'){
                break;
            }
            lon_arg++;
            comando_usuario++;
        }
        
        //OK hasta aqui
        
        *longitud_argumento = lon_arg;
        
        char *argumento = malloc((sizeof(char)*lon_arg) + 1);
#ifdef DEBUG
        printf("Yay! longitud_argumento: %d\n", *longitud_argumento);
#endif
        
        if(argumento == NULL){
            
            return NULL;
        }
        
        memset(argumento, 0, lon_arg + 1);
        strncpy(argumento, inicio_argumento, lon_arg);
        
        argumento[lon_arg] = '\0';
        
#ifdef DEBUG
        printf("%s\n", argumento);
#endif
        return argumento;      
    }
}

/** 
 *Rutina para traducir los status de respuesta
 */
char *traducirStatusRespuesta(int codigo)
{
    
    switch(codigo){
        case OK:
            return "OK";
        case ERROR:
            return "ERROR";
        default:
            return NULL;
    }
}

/**
 *Verificar entrada
 */
