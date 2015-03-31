#ifndef MODBUSTN_H
#define MODBUSTN_H

#include "definiciones.h"
#include <modbus.h>
/*#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>*/


#define MODO_RS_232             1
#define MODO_RS_485_HD          2
#define MODO_RS_485_FD          3

#define COM2                    "/dev/ttyAM1"


int modbus_test();


/**
 *Crea el contexto MODBUS RTU
 */
int conectar_modbus_serial(int modo_puerto, int baudrate, char *tty, int modo_puerto_serial, int data_bits, char paridad, int stop_bits, modbus_t *contexto);


/**
 *Liberar el contexto modbus (libera los recursos usados)
 */
void liberarContextoModbus(modbus_t *contexto);



/**
 *Configura los registros del puerto COM2 de la tarjeta TS7200 para RS-232, RS-485 HD o RS-485 FD.
 *Esta funcion es usada por conectar_modbus_serial
 */
static int configurar_puerto_serial(int modo_puerto, int baudrate, char paridad, int stop_bits, int data_bits);


#endif
