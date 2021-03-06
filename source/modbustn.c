#include "modbustn.h"


int mcr; 

/**
 * Establece los parametros de la interface serial a nivel de SW.
 *
 */


int establecer_atributos_interface (int fd, int speed, int parity, int modo)
{        
        /*Código tomado de http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c */
    
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        
        if (tcgetattr (fd, &tty) != 0)
        {
            perror("error de tcgetattr");
            return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 1;            // read doesn't block   (if zero). Si n > 0, llamada read se bloqueará hasta que reciba al menos n caracteres
        tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;                  //si parity = 0 --> no parity
        tty.c_cflag &= ~CSTOPB;                 //one stop bit
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                perror("error de tcsetattr");
                return -1;
        }
        
        if(modo == MODO_RS_485_FD){
            mcr = AUTO485FD;
        }
        else if(modo == MODO_RS_485_HD){
             mcr = AUTO485HD;
        }
        
        //A nivel de HW, establecemos conectividad full duplex
        if(modo == MODO_RS_485_FD || modo == MODO_RS_485_HD){
            int status = ioctl(fd,  TIOC_SBCS485, &mcr);
            if( status < 0){
                perror("error de ioctl\n");
                return -1;
            }
        }
        
        return 0;
}


/**
 * Para RS-232 el unico baudrate disponible es 115200
 */
int configurar_puerto_serial(int modo_puerto, int baudrate, char paridad, int stop_bits, int data_bits){
    
    //En reset, el puerto COM2 se configura como RS-232 por defecto.
    
    //Abrimos el acceso a memoria, para poder acceder a los registros
    int fd_com2 = open("/dev/mem", O_RDWR | O_SYNC);
    
    if(fd_com2 == -1){
        printf("ERROR: no se pudo abrir la region de memoria para configurar COM2\n");
        return -1;
    }
    
     //Mapeamos la region de memoria para configurar el modo del puerto...COM2 Support Register
    unsigned char *RS485_SUPPORT_REGISTER = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd_com2, 0x22400000);
        
    if(RS485_SUPPORT_REGISTER == MAP_FAILED ){
        printf("ERROR: no se pudo abrir la region de memoria para configurar COM2\n");
        return -1;
    }
    
    if(modo_puerto == MODO_RS_485_FD || modo_puerto == MODO_RS_485_HD){
    
        if(!((*RS485_SUPPORT_REGISTER) & 0x00000002)){      //check bit 1?
            printf("ERROR: Tarjeta no tiene instalada la opcion RS-485\n");
            munmap((void *)RS485_SUPPORT_REGISTER, getpagesize());
            return -1;
        }
    }
    
    //Mapeamos la region de memoria para configurar el modo del puerto...COM2 Mode Register
    unsigned char *COM2_MODE_REGISTER = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd_com2, 0x22C00000);
        
    if(COM2_MODE_REGISTER == MAP_FAILED ){
        printf("ERROR: no se pudo abrir la region de memoria para configurar COM2\n");
        return -1;
    }
    
    unsigned char *COM2_FORMAT = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd_com2, 0x23000000);     //Si usamos HD con 8 + parity, o 8 bits
    
    if(COM2_FORMAT == MAP_FAILED ){
        printf("ERROR: no se pudo abrir la region de memoria para configurar COM2\n");
        munmap((void *)COM2_MODE_REGISTER, getpagesize());
        return -1;
    }
    
    switch(modo_puerto){
        case MODO_RS_232:
            *COM2_MODE_REGISTER = 0x00;
            break;
        case MODO_RS_485_HD:
            if(baudrate == 9600){
                *COM2_MODE_REGISTER = 0x04;
            }
            else if(baudrate == 19200){
                *COM2_MODE_REGISTER = 0x05;
            }
            else if(baudrate == 57600){
                *COM2_MODE_REGISTER = 0x06;
            }
            else if(baudrate == 115200){
                *COM2_MODE_REGISTER = 0x07;
            }
            else{
                printf("ERROR: baudrate de COM2 invalido\n");
                munmap((void *)COM2_MODE_REGISTER, getpagesize());
                return -1;
            }
            
            if((data_bits == 8 && paridad != 'N') || (data_bits == 8 && stop_bits == 2)){
                *COM2_FORMAT = 0x01;
            }
            else{
                *COM2_FORMAT = 0x00;
            }

            
            break;
        case MODO_RS_485_FD:
            *COM2_MODE_REGISTER = 0x01;
            *COM2_FORMAT = 0x00;
            break;
        default:
            printf("ERROR: modo de puerto serial invalido\n");
            munmap((void *)COM2_MODE_REGISTER, getpagesize());
            munmap((void *)COM2_FORMAT, getpagesize());
            munmap((void *)RS485_SUPPORT_REGISTER, getpagesize());
            return -1;
    }
    
    munmap((void *)COM2_MODE_REGISTER, getpagesize());
    munmap((void *)COM2_FORMAT, getpagesize());
    munmap((void *)RS485_SUPPORT_REGISTER, getpagesize());
    
    
    
    close(fd_com2);
    
    return 0; 
}


void cerrar_modbus_serial(modbus_t *contexto){
    modbus_mapping_free(mapeo_modbus);      //variable global
    modbus_close(contexto);
}


int conectar_modbus_serial(int modo_puerto, int baudrate, char *tty, int data_bits, char paridad, int stop_bits, modbus_t **contexto, int id_esclavo){
    
    //Configuramos el puerto de hardware
    int status = configurar_puerto_serial(modo_puerto, baudrate, paridad, stop_bits, data_bits);
    
    if(status == -1){
        printf("ERROR: No se pudo configurar el hardware del puerto serial\n");
        return -1;
    }
    
    *contexto = modbus_new_rtu(tty, baudrate, paridad, data_bits, stop_bits);
    
    if(*contexto == NULL){
        fprintf(stderr, "Error al crear el contexto modbus\n");
        return -1;
    }
    
    //Establecer nuestra id de esclavo
    
    if(id_esclavo > 0){
        int err = modbus_set_slave(*contexto, id_esclavo);
        
        if(err < 0)
            perror("Error al establecer ID del esclavo");
    }
    


    //El siguiente mapping sera usado.
    
    //-Coils para salidas DIGITALES (RELAYS)
    //DIO0 a DIO3 --> Coils 1 a 4       (00001 a 00004)  --> Actuacion Relés aires acondicionados
    //-Puerta
    //DIO4        --> Discrete input 1 (10001)
    //Combustible --> Discrete input 2 (10002)      
    //Generador   --> Discrete input 3 (10003)
    //AACC P.     --> Discrete input 4 (10004)      
    //AACC B.     --> Discrete input 5 (10005)
    // 
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
    
    //4 bits (r/w), 3 input bits (read only), 0 HR (r/w), 32 input register (read only).
    mapeo_modbus = modbus_mapping_new(NUMERO_COILS, NUMERO_INPUT_BITS, NUMERO_REG, NUMERO_INPUT_REG);     //coils, input bits, reg, input regs

    
    if(mapeo_modbus == NULL){
        printf("ERROR: No se pudo crear el mapeo modbus de los registros\n");
        modbus_free(*contexto);
        return -1;
    }
    
    //Iniciamos la conexion serial
    if(modbus_connect(*contexto) == -1){
        printf("ERROR: No se pudo iniciar la comunicacion modbus serial\n");
        modbus_free(*contexto);
        return -1;
    }
    
    
    //Aqui cambiamos los parametros seriales (si usamos modo RS-485), para activar el Half Duplex Automatico
    //Y los parametros seriales
    int fd  = modbus_get_socket(*contexto);
    
    //Convertimos el baud rate int a speed_t
    speed_t baud_rate;
    
    switch(baudrate){
	
	case 19200:
	    baud_rate = B19200;
	    break;
	case 115200:
	    baud_rate = B115200;
	    break;
	case 9600:
	    baud_rate = B9600;
	    break;
	case 57600:
	    baud_rate = B57600;
	    break;
	default:
	    baud_rate = B19200;
	    break;
	
    }

    
   
    if( establecer_atributos_interface(fd, baud_rate, 0, modo_puerto) < 0){      //HW y SW
        perror("No se pudieron cambiar los parametros seriales\n");
        modbus_free(*contexto);
        return -1;
    }

    
    return 0;
}



/**
 *Configura los registros del puerto COM2 de la tarjeta TS7200 para RS-232, RS-485 HD o RS-485 FD.
 *Esta funcion es usada por conectar_modbus_serial
 */



/**
 *Las funciones mostradas abajo son SOLO para manipular los valores almacenados
 *en la estructura modbus_mapping_t, no para comunicacion con otros dispositivos.
 *Para esto, se usan las funciones de la libreria modbus
 */

int asignarBit(modbus_mapping_t *mapeo, uint8_t valor, int direccion){
    
    if(mapeo->tab_bits == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_bits - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    if(!(valor == ON || valor == OFF)){
        return -1;                  //Solo permitimos valores ON u OFF
    }
    
    mapeo->tab_bits[direccion] = valor;
    
    return 0;
}


int asignarRegistro(modbus_mapping_t *mapeo, uint16_t valor, int direccion){
    
    if(mapeo->tab_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_registers - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    mapeo->tab_registers[direccion] = valor;
    
    return 0;
}


int asignarInputBit(modbus_mapping_t *mapeo, uint8_t valor, int direccion){
    
    if(mapeo->tab_input_bits == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_input_bits - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    if(!(valor == ON || valor == OFF)){
        return -1;                  //Solo permitimos valores ON u OFF
    }
    
    mapeo->tab_input_bits[direccion] = valor;
    
    return 0;
}



int asignarRegistroInput(modbus_mapping_t *mapeo, uint16_t valor, int direccion){
    
    if(mapeo->tab_input_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_input_registers - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    mapeo->tab_input_registers[direccion] = valor;
    
    return 0;
}



uint8_t leerBit(modbus_mapping_t *mapeo,  int direccion){
    
    if(mapeo->tab_bits == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_bits - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    return mapeo->tab_bits[direccion];
    
    
}



uint16_t leerRegistro(modbus_mapping_t *mapeo, int direccion){
    
    if(mapeo->tab_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_registers - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    return mapeo->tab_registers[direccion];
    
}


uint8_t leerInputBit(modbus_mapping_t *mapeo, int direccion){
    
    if(mapeo->tab_input_bits == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_input_bits - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    return mapeo->tab_input_bits[direccion];
    
}



uint16_t leerRegistroInput(modbus_mapping_t *mapeo, int direccion){
    
    if(mapeo->tab_input_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion > (mapeo->nb_input_registers - 1)){
        return -1;                  //estamos tratando de asignar un registro que no existe
    }
    
    return mapeo->tab_input_registers[direccion];

}


int asignarRegistroFloat(modbus_mapping_t *mapeo, float valor, int direccion, int swap){
    
     if(mapeo->tab_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion + 1 > mapeo->nb_registers - 1){
        return -1;                  //No hay registros suficientes para almacenar el valor de 32 bits
    }
    
    float copia_valor = valor;
    
    //Representacion en memoria
    unsigned char valor_mem[4] = {0};         //32 bitss
    
    memcpy(valor_mem, &copia_valor, 4);                                        //obtenemos los bits en memoria (nos saltamos los castings hechos por C)
    //printf("valor 0x%X, %f\n", *(unsigned int*)valor, *valor);          //Si no se hace asi, float es promocianado a DOUBLE! (para hex representation)
    
    unsigned int upper_byte =  0;
    unsigned int lower_byte =  0;
    
    memcpy(&upper_byte, valor_mem+2, 2);
    memcpy(&lower_byte, valor_mem, 2);
    
    //bytes mas significtivos primero
    if(!swap){
        memcpy(&mapeo->tab_registers[direccion], &upper_byte, 2);
        memcpy(&mapeo->tab_registers[direccion+1], &lower_byte, 2);
    }
    else{
        memcpy(&mapeo->tab_registers[direccion], &lower_byte, 2);
        memcpy(&mapeo->tab_registers[direccion+1], &upper_byte, 2);
    }
    
    return 0;
}


int leerRegistroFloat(modbus_mapping_t *mapeo, int direccion, float *valor, int swap){
    
    if(mapeo->tab_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion + 1 > mapeo->nb_registers - 1){
        return -1;                  //No hay registros suficientes para almacenar el valor de 32 bits
    }
    
    unsigned short *addr = (unsigned short *)valor;         //para poder acceder a direcciones de 2 bytes en 2 bytes..
    
    //bytes mas significtivos primero
    if(!swap){

        memcpy(addr+1, &mapeo->tab_registers[direccion], 2);
        memcpy(addr, &mapeo->tab_registers[direccion+1], 2);       
        
    }
    else{
        memcpy(addr, &mapeo->tab_registers[direccion],2);
        memcpy(addr+1, &mapeo->tab_registers[direccion+1],2);
    }
    
    return 0;
}




int asignarRegistroInputFloat(modbus_mapping_t *mapeo, float valor, int direccion, int swap){
    
    if(mapeo->tab_input_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion + 1 > mapeo->nb_input_registers - 1){
        return -1;                  //No hay registros suficientes para almacenar el valor de 32 bits
    }
    
    float copia_valor = valor;
    
    //Representacion en memoria
    unsigned char valor_mem[4] = {0};         //32 bitss
    
    memcpy(valor_mem, &copia_valor, 4);                                        //obtenemos los bits en memoria (nos saltamos los castings hechos por C)
    
    //printf("valor 0x%X, %f\n", *(unsigned int *)valor_mem, copia_valor);          //Si no se hace asi, float es promocianado a DOUBLE! (para hex representation)
    
    unsigned int upper_byte =  0;
    unsigned int lower_byte =  0;
    
    memcpy(&upper_byte, valor_mem+2, 2);
    memcpy(&lower_byte, valor_mem, 2);
    
    //bytes mas significtivos primero
    if(!swap){
        memcpy(&mapeo->tab_input_registers[direccion], &upper_byte, 2);
        memcpy(&mapeo->tab_input_registers[direccion+1], &lower_byte, 2);
        //printf("Alamcenado 0x%X, 0x%X\n", mapeo->tab_input_registers[direccion], mapeo->tab_input_registers[direccion+1]);
    }
    else{
        memcpy(&mapeo->tab_input_registers[direccion], &lower_byte, 2);
        memcpy(&mapeo->tab_input_registers[direccion+1], &upper_byte, 2);
        //printf("Alamcenado 0x%X, 0x%X\n", mapeo->tab_input_registers[direccion+1], mapeo->tab_input_registers[direccion]);
    }
    
    return 0;
}


int leerRegistroInputFloat(modbus_mapping_t *mapeo, int direccion, float *valor, int swap){
    
    if(mapeo->tab_input_registers == NULL){
        return -1;                  //No existen los registros a asignar
    }
    
    if(direccion + 1 > mapeo->nb_input_registers - 1){
        return -1;                  //No hay registros suficientes para almacenar el valor de 32 bits
    }
    
    unsigned short *addr = (unsigned short *)valor;         //para poder acceder a direcciones de 2 bytes en 2 bytes..
        
    //bytes mas significtivos primero
    if(!swap){
        memcpy(addr+1, &mapeo->tab_input_registers[direccion], 2);
        memcpy(addr, &mapeo->tab_input_registers[direccion+1], 2);       
        
    }
    else{
        memcpy(addr, &mapeo->tab_input_registers[direccion],2);
        memcpy(addr+1, &mapeo->tab_input_registers[direccion+1],2);
    }
    
    return 0;
}

