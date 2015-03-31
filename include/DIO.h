#ifndef DIO_H
#define DIO_H

#include "definiciones.h"
/*#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>*/

void configurarPuertosDIO();

int activarPuerto(puerto_DIO puerto);

int desactivarPuerto(puerto_DIO puerto);

status_puerto_DIO statusPuerto(puerto_DIO puerto);

#endif
