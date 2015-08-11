#ifndef MODBUSTN_H
#define MODBUSTN_H

#include "definiciones.h"
//#include <modbus.h>


#define MODO_RS_232             1
#define MODO_RS_485_HD          2
#define MODO_RS_485_FD          3

#define COM2                    "/dev/ttyAM1"

#define SWAP    1
#define NO_SWAP 0



//Direcciones de las mediciones (coils, regs, etc)

//El siguiente mapping sera usado.
    
    //-Coils para salidas DIGITALES (RELAYS)
    //DIO0 a DIO3 --> Coils 1 a 4
    //-Puerta
    //DIO4        --> Discrete input 1 (10001)
    //Combustible --> Discrete input 2 (10002)      
    //Generador   --> Discrete input 3 (10003)
    //-Medidas ananlogas, usamos dos registros (input reg -> read only) consecutivos por medicion
    //Medicion              Canal Software          Registros MODBUS (2 por medicion, 32 bits)
    //I DC 1                    0                       40001 - 40002
    //I DC 2                    4                       40003 - 40004
    //I DC 3                    8                       40005 - 40006
    //I DC 4                   12                       40007 - 40008
    //I AC 1                   16                       40009 - 40010
    //I AC 2                   20                       40011 - 40012
    //I AC 3                    2                       40013 - 40014
    //I AC 4                    6                       40015 - 40016
    
    //Temp 1                    1                       40017 - 40018
    //V AC 1                   13                       40019 - 40020
    //V AC 2                   17                       40021 - 40022
    //V DC 1                   21                       40023 - 40024
    //V DC 2                    3                       40025 - 40026
    //V DC 3                    7                       40027 - 40028
    //V DC 4                   11                       40029 - 40030
    //Humedad                  15                       40031 - 40032
    
#define COIL_RELE_0         0
#define COIL_RELE_1         1
#define COIL_RELE_2         2
#define COIL_RELE_3         3

#define INPUT_BIT_PUERTA             0
#define INPUT_BIT_COMBUSTIBLE        1
#define INPUT_BIT_GENERADOR          2

#define REGISTRO_INPUT_I_DC_1        1
#define REGISTRO_INPUT_I_DC_2        3
#define REGISTRO_INPUT_I_DC_3        5
#define REGISTRO_INPUT_I_DC_4        7
#define REGISTRO_INPUT_I_AC_1        9
#define REGISTRO_INPUT_I_AC_2        11
#define REGISTRO_INPUT_I_AC_3        13
#define REGISTRO_INPUT_I_AC_4        15
#define REGISTRO_INPUT_TEMPERATURA   17
#define REGISTRO_INPUT_V_AC_1        19
#define REGISTRO_INPUT_V_AC_2        21
#define REGISTRO_INPUT_V_DC_1        23
#define REGISTRO_INPUT_V_DC_2        25
#define REGISTRO_INPUT_V_DC_3        27
#define REGISTRO_INPUT_V_DC_4        29
#define REGISTRO_INPUT_HUMEDAD       31


//Contiene los registros, coils, inputs, etc
modbus_mapping_t *mapeo_modbus;


int modbus_test();


/**
 *Crea la conexion MODBUS RTU
 */
int conectar_modbus_serial(int modo_puerto, int baudrate, char *tty, int data_bits, char paridad, int stop_bits, modbus_t **contexto, int id_esclavo);


/**
 *Cierra la conectividad serial de modbus
 */
void cerrar_modbus_serial(modbus_t *contexto);



/**
 *Configura los registros del puerto COM2 de la tarjeta TS7200 para RS-232, RS-485 HD o RS-485 FD.
 *Esta funcion es usada por conectar_modbus_serial
 */

int configurar_puerto_serial(int modo_puerto, int baudrate, char paridad, int stop_bits, int data_bits);


/**
 *Asignamos un bit register
 */
int asignarBit(modbus_mapping_t *mapeo, uint8_t valor, int direccion);


/**
 *Asignar un registro
 */
int asignarRegistro(modbus_mapping_t *mapeo, uint16_t valor, int direccion);

/**
 *Asignamos un input bit 
 */
int asignarInputBit(modbus_mapping_t *mapeo, uint8_t valor, int direccion);


/**
 *Asignar un registro input
 */
int asignarRegistroInput(modbus_mapping_t *mapeo, uint16_t valor, int direccion);


/**
 *Lee un bit register
 */
uint8_t leerBit(modbus_mapping_t *mapeo,  int direccion);


/**
 *Lee un registro
 */
uint16_t leerRegistro(modbus_mapping_t *mapeo, int direccion);

/**
 *Lee un input bit
 */
uint8_t leerInputBit(modbus_mapping_t *mapeo,  int direccion);


/**
 *Lee un input register
 */
uint16_t leerRegistroInput(modbus_mapping_t *mapeo, int direccion);

/**
 *Asignar dos registros con un float
 */
int asignarRegistroFloat(modbus_mapping_t *mapeo, float valor, int direccion, int swap);

/**
 *Lee dos registros consecutivos como un float
 */
int leerRegistroFloat(modbus_mapping_t *mapeo, int direccion, float *valor, int swap);

/**
 *Asignar dos registros con un float
 */
int asignarRegistroInputFloat(modbus_mapping_t *mapeo, float valor, int direccion, int swap);

/**
 *Lee dos registros consecutivos como un float
 */
int leerRegistroInputFloat(modbus_mapping_t *mapeo, int direccion, float *valor, int swap);




#endif
