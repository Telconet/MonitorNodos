#ifndef DEFINICIONES_H
#define DEFINICIONES_H

//los defines de abajo son necesarios para el que unistd.h pueda ser usado.
#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ts_sbc.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include "modbus.h"

//#include <net-snmp/net-snmp-config.h>
//#include <net-snmp/net-snmp-includes.h>
//#include <net-snmp/agent/net-snmp-agent-includes.h>

//#define DEBUG
//#define DEBUG_ADC

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 1025
#endif

//Tamanos
#define MAX_TAMANO_ENTRADA 30
#define TAMANO_MAX_RESPUESTA 2048
#define LONGITUD_MAX_IP 16;


#define ARCHIVO_PROCESS_ID_DAEMON "pidDaemon"
#define ARCHIVO_PROCESS_ID_CONT "pidCont"
#define COMANDO_REMOVER_ARCHIVO "rm -rf "

//Directorios
#define DIRECTORIO_DE_TRABAJO "/tmp"
#define ARCHIVO_SOCKET_IPC "/tmp/socketcommon"

//Parametros SNMP 
#define INTERFAZ_RED_PRINCIPAL "eth"
#define PUERTO_TRAP_SNMP 162

//Comando-Respuesta
#define COMANDO 99999
#define RESPUESTA 66666
#define ACK 33333

//Strings de comandos validos
#define SALIR_DAEMON_STR "cerrar monitor\n"
#define INFORMACION_NODO_STR "info nodo\n"
#define ACTIVAR_RELE_STR "activar rele"
#define DESACTIVAR_RELE_STR "desactivar rele"
#define DESACTIVAR_RELE_STR "desactivar rele"
#define INFORMACION_CONF_STR "info configuracion\n"
#define INFORMACION_VALORES_MIN_STR "info niveles alertas\n"
#define MOSTRAR_MEDICIONES_STR "status nodo\n"
#define CAMBIAR_MINIMOS_STR "cambiar minimos\n"
#define AYUDA_STR "ayuda\n"


//Otras definiciones
#define SEGUNDOS_POR_MINUTO 60
#define ZONA_HORARIA_ACTUAL (-5)
#define ANIO_INICIO_EPOCA 1900

//Mediciones
//#define NUMERO_MEDICIONES 24
#define NUMERO_MEDICIONES_ADC 18
#define NUMERO_MEDICIONES_DIO 5
#define NUMERO_MUESTRAS 512                 //512 256
#define NUMERO_MAXIMO_CANALES_ADC 24

//Datos ADC
#define VOLTAJE_REFERENCIA_2VREF 5.000
#define VOLTAJE_REFERENCIA_VREF ((VOLTAJE_REFERENCIA_2VREF)/(2))
#define NUMERO_ESCALONES 4096
#define VOLTIOS_POR_ESCALON_2VREF ((VOLTAJE_REFERENCIA_2VREF)/(NUMERO_ESCALONES))
#define VOLTIOS_POR_ESCALON_VREF (3.50000f / 4096)


//Traps especificas
#define TRAP_TEMPERATURA_ALTA 301
#define TRAP_HUMEDAD_ALTA 310
#define TRAP_HUMEDAD_BAJA 311
#define TRAP_VOLTAJE_DC_BAJO 321
#define TRAP_CORRIENTE_DC_ALTA 340      //50a en ac, 10A,   22v dc, 46V dc
#define TRAP_VOLTAJE_AC_BAJO 350
#define TRAP_VOLTAJE_AC_ALTO 351
#define TRAP_CORRIENTE_AC_ALTA 360 
#define TRAP_PUERTA_ABIERTA 390
#define TRAP_PUERTA_CERRADA 391
#define TRAP_COMBUSTIBLE 392
#define TRAP_GENERADOR 393

#ifndef SNMP_GENERICTRAP_ENTERSPECIFIC
    #define SNMP_GENERICTRAP_ENTERSPECIFIC 6
#endif


#ifndef SNMP_GENERICTRAP_COLDSTART
    #define SNMP_GENERICTRAP_COLDSTART 0
#endif

//OID de bindings para traps
#define OID_VOLTAJE_DC_1 "1.3.6.1.4.1.65000.1.1"
#define OID_VOLTAJE_DC_2 "1.3.6.1.4.1.65000.1.2"
#define OID_VOLTAJE_DC_3 "1.3.6.1.4.1.65000.1.3"
#define OID_VOLTAJE_DC_4 "1.3.6.1.4.1.65000.1.4"
#define OID_TEMPERATURA_1 "1.3.6.1.4.1.65000.1.5"
#define OID_TEMPERATURA_2 "1.3.6.1.4.1.65000.1.6"
#define OID_TEMPERATURA_3 "1.3.6.1.4.1.65000.1.7"
#define OID_HUMEDAD "1.3.6.1.4.1.65000.1.8"
#define OID_CORRIENTE_DC_1 "1.3.6.1.4.1.65000.1.9"
#define OID_CORRIENTE_DC_2 "1.3.6.1.4.1.65000.1.10"
#define OID_CORRIENTE_DC_3 "1.3.6.1.4.1.65000.1.11"
#define OID_CORRIENTE_DC_4 "1.3.6.1.4.1.65000.1.12"
#define OID_CORRIENTE_AC_1 "1.3.6.1.4.1.65000.1.13"
#define OID_CORRIENTE_AC_2 "1.3.6.1.4.1.65000.1.14"
#define OID_CORRIENTE_AC_3 "1.3.6.1.4.1.65000.1.15"
#define OID_CORRIENTE_AC_4 "1.3.6.1.4.1.65000.1.16"
#define OID_VOLTAJE_AC_1 "1.3.6.1.4.1.65000.1.17"
#define OID_VOLTAJE_AC_2 "1.3.6.1.4.1.65000.1.18"

//PAra RS485
#define TIOC_SBCC485 _IOW('T',0x70,int) /*TS RTS/485 mode Clear*/
#define TIOC_SBCS485 _IOW('T',0x71, int) /*TS RTS/485 mode Set */
#define AUTO485FD 1
#define RTSMODE 2
#define AUTO485HD 4


//Modo comunicacion (MODBUS o TCP)
int modoComunicacion;

//Estructuras
//Medicion
struct medicion{
    int tipo_medicion;
    int id_medicion;
    unsigned char status;
    float valor;
    struct tm *tiempo;
    struct medicion *siguiente;         //apunta a la siguiente medicion
};

//Nodo
struct nodo{
    char *id;
    int lon_ip;
    char *ip;            //IP puede ser de hasta 15 caractares mas caracter nulo
};

//Comando
struct comando{
    int com;
    int numero_argumentos;
    int *long_argumentos;               //Añadido...
    char **argumentos;
};

//respuesta
struct respuesta{
    int status;
    int long_res;
    char *res;
};


//Enumeraciones
enum medidas_sensores { TEMPERATURA_1,
                        TEMPERATURA_2,
                        TEMPERATURA_3,
                        HUMEDAD,
                        VOLTAJE_DC_1,
                        VOLTAJE_DC_2,
                        VOLTAJE_DC_3,
                        VOTAJE_DC_4,
                        CORRIENTE_DC_1,
                        CORRIENTE_DC_2,
                        CORRIENTE_DC_3,
                        CORRIENTE_DC_4,
                        CORRIENTE_AC_1,
                        CORRIENTE_AC_2,
                        CORRIENTE_AC_3,
                        VOLTAJE_AC_1,
                        VOLTAJE_AC_2,
                        VOLTAJE_AC_3,
                        VOLTAJE_AC_4,
                        NIVEL_COMBUSTIBLE_1,
                        SENSOR_PUERTA_ACCESO,
                        RELE_1,
                        RELE_2,
                        RELE_3,
                        RELE_4
};

enum comandos{ SALIR_DAEMON,
               SALIR_CONTROLADOR,
               CAMBIAR_MINIMOS,
               INFORMACION_NODO,
               INFORMACION_CONF,
               INFORMACION_VAL_MIN,
               ACTIVAR_RELE,
               DESACTIVAR_RELE,
               MOSTRAR_MEDICIONES,
               AYUDA
};

enum status_comando{ OK,
                     ERROR,    
};

enum id_medicion{TEMPERATURA,
                 HUMEDAD_RELATIVA,
                 VOLTAJE_DC,
                 CORRIENTE_DC,
                 CORRIENTE_AC,
                 VOLTAJE_AC,
                 NIVEL_COMBUSTIBLE,
                 SENSOR_PUERTA,
                 RELE
};

typedef enum {NINGUNO,
                     BAJO,
                     ENCENDIDO,
                     APAGADO,
                     ABIERTO,
                     CERRADO
} status_medicion;

typedef enum  { VOLTAJE_DC_24V,
                VOLTAJE_DC_48V ,
                VOLTAJE_AC_110V,
                VOLTAJE_AC_220V
} tipo_voltaje;


//Puertos I/O
typedef enum { puerto_DIO_0,
               puerto_DIO_1,
               puerto_DIO_2,
               puerto_DIO_3,
               puerto_DIO_4,
               puerto_DIO_5,
               puerto_DIO_6,
               puerto_DIO_7,
               puerto_DIO_NINGUNO
} puerto_DIO;



//Status de los puertos entrada salida.
typedef enum {  PUERTO_ON,
                PUERTO_OFF
} status_puerto_DIO;



#endif

