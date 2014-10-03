#include "snmpdef.h"

/**
 *Rutina para conectarnos al servidor SNMP
 *
 */
netsnmp_session *inicializarSistemaSnmp(char *ip_servidor,char *nombre, char *comunidad, int versionSNMP){
    
    
    
    netsnmp_session *sesion = malloc(sizeof(netsnmp_session));
    
    if(sesion == NULL){
        printf("ERROR: No se pudo crear la sesion SNMP.\n");
        return NULL;
    }
  
    
    
    //Inicializamos la libreria SNMP
    init_snmp(nombre);
    
    //Iniciamos una sesion con valores por defecto
    snmp_sess_init(sesion);                   
    
    //asignamos los valores de la sesion
    sesion->peername = strdup(ip_servidor);   
    
    //Version SNMP v1
    sesion->version = versionSNMP;

    //nombre de comunidad usado para autenticación
    sesion->community = comunidad;
    sesion->community_len = strlen(sesion->community);
    
    return sesion;
}

/**
 *Establecer session
 */
netsnmp_session *abrirSesion(netsnmp_session *sesion){

    netsnmp_session *ss = snmp_open(sesion);                     

    if (!ss) {
      snmp_sess_perror("ack", sesion);
      printf("ERROR: No se pudo abrir la sesion SNMP\n");
      return NULL;
    }
    
    return ss;
}

/**
 *Rutina que devuelve multiples sesiones SNMP
 */
netsnmp_session **abrirMultiplesSesiones(char **ip_servidor, int numero_servidores, char *nombre, char *comunidad, int versionSNMP){
    
    int i;
    netsnmp_session **sesionesCreacion, **sesiones;
    
    sesionesCreacion = malloc(sizeof(*sesionesCreacion)*numero_servidores);
    sesiones = malloc(sizeof(*sesiones)*numero_servidores);

    if(sesionesCreacion == NULL){
        printf("ERROR: No se pudieron crear las multiples sesiones SNMP.\n");
        return NULL;
    }
    
    if(sesiones == NULL){
        printf("ERROR: No se pudieron crear las sesiones SNMP individuales.\n");
        return NULL;
    }
    
    for(i = 0; i < numero_servidores; i++){
    
        sesionesCreacion[i] = inicializarSistemaSnmp(ip_servidor[i], nombre, comunidad, versionSNMP);
        
        if(sesionesCreacion != NULL){
            sesiones[i] = abrirSesion(sesionesCreacion[i]);
        }
    }
    
  
    
    return sesiones;  
}

/**
 *Rutina para enviar traps
 */
int enviarTrap(netsnmp_session *sesion, char *ip_agente, int trapGenerico, int trapEspecifico, char *OID, float valor){
    
    netsnmp_pdu *pdu;
    int status;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;
    
    //Creamos el trap
    pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    anOID_len = MAX_OID_LEN;
    
    //Error al crear el PDU
    if ( !pdu ) {
            fprintf(stderr, "Fallo al crear PDU SNMP.\n");
            SOCK_CLEANUP;
            
            //Removemos el archivo con el pid    
            char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
            strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
            strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
            system(comandoRemover);
            exit(1);
    }
    
    //Configurar los campos manualmente
    pdu->trap_type = trapGenerico;
    pdu->specific_type = trapEspecifico;

    //TODO Añadir la IP del agente
    //unsigned char ip[4];
    parse_IP(ip_agente, pdu->agent_addr);
    
    //los traps se envian hacia el puerto 162
    
    //oid
    int longValor = 20;
    int longString = 200;
    char *valorStr = malloc(sizeof(char)*longValor);
    
    if(OID != NULL){
        if (!snmp_parse_oid(OID, anOID, &anOID_len)) {
          snmp_perror(OID);
          SOCK_CLEANUP;
          printf("Error\n");
          
          //Removemos el archivo con el pid    
          char comandoRemover[strlen(ARCHIVO_PROCESS_ID_DAEMON) + strlen(COMANDO_REMOVER_ARCHIVO)];
          strcpy(comandoRemover, COMANDO_REMOVER_ARCHIVO);
          strcat(comandoRemover, ARCHIVO_PROCESS_ID_DAEMON);
          system(comandoRemover);
          exit(1);
        }
        
        //anadimos oid
        switch(trapEspecifico){
            case TRAP_CORRIENTE_AC_ALTA:
                snprintf(valorStr, longValor, "%.2f A");
                break;
            case TRAP_CORRIENTE_DC_ALTA:
                snprintf(valorStr, longValor, "%.2f A");
                break;
            case TRAP_VOLTAJE_DC_BAJO:
                snprintf(valorStr, longValor, "%.2f V");
                break;
            case TRAP_HUMEDAD_ALTA:
                snprintf(valorStr, longValor, "%.2f %% HR");
                break;
            case TRAP_TEMPERATURA_ALTA:
                snprintf(valorStr, longValor, "%.2f %% HR");
                break;
            case TRAP_VOLTAJE_AC_BAJO:
                snprintf(valorStr, longValor, "%.2f V RMS");        //check
                break;
            case TRAP_GENERADOR:
                snprintf(valorStr, longString, "Generador encendido.");
                break;
            case TRAP_COMBUSTIBLE:
                snprintf(valorStr, longString, "Nivel de combustible bajo");
                break;
            default:
                break;
        }
        
        
        snmp_add_var(pdu, anOID, anOID_len, 's', valorStr); //Mandamos strings
    }
    
     
    //enviamos el trap
    status = snmp_send(sesion, pdu);
    
    //liberamos la memoria
    free(valorStr);
    
    //TODO LIBERAR el PDU!!!!!!!!!! PROBAR -> Estamos liberando dos veces... snmp_send libera el PDU
    //snmp_free_pdu(pdu);

}

/**
 *Esta rutina cierra la sesion SNMP
 */
void cerrarSistemaSnmp(netsnmp_session *sesion){
    if(sesion != NULL){
        snmp_close(sesion);
    }
}

