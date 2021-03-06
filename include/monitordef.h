#ifndef MONITORDEF_H
#define MONITORDEF_H

#include "definiciones.h"
#include "utilidades.h"
#include "modBD.h"
#include "email.h"
#include "mediciones.h"
#include "ADC.h"
#include "DIO.h"
#include "IPC.h"
#include "modbustn.h"

//Definiciones
#define MAX_BUFFER_SIZE 20
#define NO_MAX_CONEX 20


//Definiciones de opciones de configuracion
#define ID_NODO "id-nodo"
#define INTERVALO_MON "intervalo-monitoreo"
#define INTERVALO_MON_PUERTA "intervalo-monitoreo-puerta"
#define ARCHIVO_COL_BD_ADC "archivo-columnas-BD-ADC"
#define ARCHIVO_COL_BD_DIO "archivo-columnas-BD-DIO"
#define IP_SERVIDOR_SNMP "ip-servidor-snmp"
#define COMUNIDAD_SNMP "comunidad-snmp"
#define SESION_SNMP "nombre-sesion-snmp"
#define BASE_DE_DATOS "nombre-base-de-datos"
#define IP_SERVIDOR_BASE_DE_DATOS "ip-base-de-datos"
#define USUARIO_BASE_DE_DATOS "usuario-base-de-datos"
#define CLAVE_BASE_DE_DATOS "clave-base-de-datos"
#define PUERTO_BASE_DE_DATOS "puerto-base-de-datos"
#define VALORES_MINIMOS_PERMITIDOS "valores-min-permitidos"
#define DESTINATARIOS_ALERTAS "destinatarios-alertas"
#define SERVIDOR_ACTUALIZACIONES "ip-servidor-actualizaciones"
#define HABILITAR_ALERTAS "habilitar-alertas"
#define PERIODO_ENVIO_EMAILS "periodo-envio-emails"
#define RAZON_CT "razon-transformador-ct"
#define IP_SERVIDOR_DATOS "servidor-datos"
#define MONITOREO_AIRES "monitoreo-aires"
#define TEMPERATURA_CRITICA "temperatura-critica"
#define PUERTO_DIO_AC_PRINCIPAL "puerto-dio-ac-principal"
#define PUERTO_DIO_AC_BACKUP "puerto-dio-ac-backup"
#define PUERTO_DATOS "puerto-datos"


//Valore predeterminados de monitoreo
#define INTERVALO_MON_PUERTA_PRED 500          //milisegundos (medio segundo)
#define INTERVALO_MON_PRED 5                   //(5 minutos)

//Version del programa
const volatile static char version[] = "1.2.0";

//Configuracion del monitor
struct configuracionMonitor{
    int intervaloMonitoreo;
    int intervaloMonitoreoPuerta;
    int numeroCanalesActivos;
    float razonCT;
    int monitoreoAires;
    int puertoDIO_ACPrincipal;
    int puertoDIO_ACBackup;
    int puertoDatosServidor;
    float temperaturaCritica;
    char *ip_servidor_datos;
    char *id_nodo;
    char *interfazRed;
    char *rutaArchivoColumnasBDADC;
    char *rutaArchivoColumnasBDDIO;
    char *rutaArchivoConfiguracion;
};


//Variables globales
int sd1, sd2;                                                       //Descriptores de archivo del socket
pid_t processID;                                                    //Nuestro pid
pthread_t monPuertaThread, monAiresAcondcionadosThread;             //thread de comandos, puerta y temporizador
struct nodo informacion_nodo;                                       //Informacion del nodo
struct configuracionMonitor* configuracion;                         //configuracion del monitor
volatile struct medicion *listaMediciones;                          //La lista de las mediciones mas actuales

//Status de los aires acondicionedos
status_puerto_DIO status_A_C_principal;                             //Sensor del aire principal
status_puerto_DIO status_A_C_backup; 	                            //Sensor del aire secundario

pthread_mutex_t mutex_status_AACC;                                  //Mutex el status de aires acondicionados...


//Para modbus
pthread_mutex_t mutexModbus;
int usandoModbus;


//Definicion de funciones
void salir(int status);

//Todos estos tenian tipo void * en vez de void
void manejarComandosControlador(void *sd);


void monitorPuerta(void *sd);

void monitorModbus(void *sd);

void monitorAiresAcondicionados(void *sd);  

void recComandosEnvResp(void *ptr); 

void manejadorSenalSIGTERMSIGINT(int sig);

struct configuracionMonitor* leerArchivoConfiguracion(char *rutaArchivo);

char **obtenerValorConfig(char *linea, int *numeroValores);


#endif

