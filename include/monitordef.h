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


#define USAR_MODBUS     1
#define USAR_TCP        2


//Definiciones
#define MAX_BUFFER_SIZE 20
#define NO_MAX_CONEX 20


//Definiciones de opciones de configuracion
#define ID_NODO                     "id-nodo"
#define INTERVALO_MON               "intervalo-monitoreo"
#define INTERVALO_MON_PUERTA        "intervalo-monitoreo-puerta"
#define ARCHIVO_COL_BD_ADC          "archivo-columnas-BD-ADC"
#define ARCHIVO_COL_BD_DIO          "archivo-columnas-BD-DIO"
#define IP_SERVIDOR_SNMP            "ip-servidor-snmp"
#define COMUNIDAD_SNMP              "comunidad-snmp"
#define SESION_SNMP                 "nombre-sesion-snmp"
#define BASE_DE_DATOS               "nombre-base-de-datos"
#define IP_SERVIDOR_BASE_DE_DATOS   "ip-base-de-datos"
#define USUARIO_BASE_DE_DATOS       "usuario-base-de-datos"
#define CLAVE_BASE_DE_DATOS         "clave-base-de-datos"
#define PUERTO_BASE_DE_DATOS        "puerto-base-de-datos"
#define VALORES_MINIMOS_PERMITIDOS  "valores-min-permitidos"
#define DESTINATARIOS_ALERTAS       "destinatarios-alertas"
#define SERVIDOR_ACTUALIZACIONES    "ip-servidor-actualizaciones"
#define HABILITAR_ALERTAS           "habilitar-alertas"
#define PERIODO_ENVIO_EMAILS        "periodo-envio-emails"
#define RAZON_CT                    "razon-transformador-ct"
#define MODBUS_BAUDRATE             "modbus-baudrate"
#define MODBUS_TTY                  "modbus-tty"
#define MODBUS_DATABITS             "modbus-databits"
#define MODBUS_STOPBITS             "modbus-stopbits"
#define MODBUS_PARITY               "modbus-paridad"
#define MODBUS_SLAVE_ID             "modbus-id-esclavo"
#define MODBUS_MODO_PUERTO          "modbus-modo-puerto" 


//Valore predeterminados de monitoreo
#define INTERVALO_MON_PUERTA_PRED 500          //milisegundos (medio segundo)
#define INTERVALO_MON_PRED 5                   //(5 minutos)


//Configuracion del monitor
struct configuracionMonitor{
    int intervaloMonitoreo;
    int intervaloMonitoreoPuerta;
    int longitudValoresPemitidos;
    int puertoBD;
    int numeroServidoresSNMP;
    int numeroValoresMinimosPermitidos;
    int numeroDestinatariosAlertas;
    int numeroMaximoControladores;
    int numeroCanalesActivos;
    int periodoEnvioEmails;
    int modbusBaudrate;
    int modbusDatabits;
    int modbusStopbits;
    char modbusParidad;
    int modbusSlaveId;
    int modbusModoPuerto;
    char *modbusTTY;
    float razonCT;
    char *id_nodo;
    char *interfazRed;
    char *rutaArchivoColumnasBDADC;
    char *rutaArchivoColumnasBDDIO;
    char *rutaArchivoConfiguracion;
    char *usuarioBD;
    char *claveBD;
    char *ipServidorBD;
    char *BD;
    char *comunidadSNMP;
    char *nombreSesionSNMP;
    char *ipServidorActualizaciones;
    char **ipServidorSNMP;
    char **valoresMinimosPermitidosMediciones;
    char **destinatariosAlertas;
    char **canalesActivos;
};


//Variables globales
int sd1, sd2;                                                         //Descriptores de archivo del socket
pid_t processID;                                                      //Nuestro pid
pthread_t localComandosThread, monPuertaThread, monAiresAcondcionadosThread, hiloModbus;   //thread de comandos, puerta y temporizador
struct nodo informacion_nodo;                                         //Informacion del nodo
struct configuracionMonitor* configuracion;                           //configuracion del monitor
volatile struct medicion *listaMediciones;                            //La lista de las mediciones mas actuales


//Variables para envio de e-mails de alertas
int alertaEmailEnviada[NUMERO_MEDICIONES_ADC];                          //contiene un bandera de si fue enviado un e-mail, por cada canal medido. Se inicializa en 0
int tiempoDesdeUltimaAlerta[NUMERO_MEDICIONES_ADC];                //Cuanto tiempo ha pasado (en minutos), desde la ultima alerta
pthread_mutex_t mutexModbus;    

//Definicion de funciones

void salir(int status);

//Todos estos tenian tipo void * en vez de void
void manejarComandosControlador(void *sd);


void monitorPuerta(void *sd);

void monitorAiresAcondicionados(void *sd);

void monitorModbus(void *sd);

void recComandosEnvResp(void *ptr);

void manejadorSenalSIGTERMSIGINT(int sig);

int procesarComando(int fd, struct comando *com);

struct configuracionMonitor* leerArchivoConfiguracion(char *rutaArchivo);

char **obtenerValorConfig(char *linea, int *numeroValores);


#endif

