#ifndef MODBUSTN_H
#define MODBUSTN_H

#include "definiciones.h"
//#include <modbus.h>


#define MODO_RS_232             1
#define MODO_RS_485_HD          2
#define MODO_RS_485_FD          3

#define COM2                    "/dev/ttyAM1"


//Contiene los registros, coils, inputs, etc
modbus_mapping_t *mapeo_modbus;


int modbus_test();


/**
 *Crea la conexion MODBUS RTU
 */
int conectar_modbus_serial(int modo_puerto, int baudrate, char *tty, int data_bits, char paridad, int stop_bits, modbus_t *contexto, int id_esclavo);


/**
 *Cierra la conectividad serial de modbus
 */
void cerrar_modbus_serial(modbus_t *contexto);



/**
 *Configura los registros del puerto COM2 de la tarjeta TS7200 para RS-232, RS-485 HD o RS-485 FD.
 *Esta funcion es usada por conectar_modbus_serial
 */
static int configurar_puerto_serial(int modo_puerto, int baudrate, char paridad, int stop_bits, int data_bits);


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
float leerRegistroFloat(modbus_mapping_t *mapeo, int direccion, int swap);

/**
 *Asignar dos registros con un float
 */
int asignarRegistroInputFloat(modbus_mapping_t *mapeo, float valor, int direccion, int swap);

/**
 *Lee dos registros consecutivos como un float
 */
float leerRegistroInputFloat(modbus_mapping_t *mapeo, int direccion, int swap);




#endif
