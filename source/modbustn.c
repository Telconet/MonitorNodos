#include "modbustn.h"



int modbus_test(){
    
    //primero configu
    
    modbus_t *ctx;
    
    ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'N', 8, 1);
    
    if (ctx == NULL) {
        fprintf(stderr, "No se pudo crear el contexto modbus.\n");
        return -1;
    }
    
    return 0;
}


static int configurar_puerto_serial(int modo_puerto, int baudrate, char paridad, int stop_bits, int data_bits){
    
    //En reset, el puerto COM2 se configura como RS-232 por defecto.
    
    //Abrimos el acceso a memoria, para poder acceder a los registros
    int fd_com2 = open("/dev/mem", O_RDWR | O_SYNC);
    
    if(fd_com2 == -1){
        printf("ERROR: no se pudo abrir la region de memoria para configurar COM2\n");
        return -1;
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
            
            break;
        case MODO_RS_485_FD:
            *COM2_MODE_REGISTER = 0x01;
            break;
        default:
            printf("ERROR: modo de puerto serial invalido\n");
            munmap((void *)COM2_MODE_REGISTER, getpagesize());
            munmap((void *)COM2_FORMAT, getpagesize());
            return -1;
    }
    
    munmap((void *)COM2_MODE_REGISTER, getpagesize());
    munmap((void *)COM2_FORMAT, getpagesize());
    
    close(fd_com2);
    
    return 0;
    
}

void liberarContextoModbus(modbus_t *contexto){
    
    if(contexto != NULL){
        modbus_free(contexto);
    }
}

void cerrar_modbus_serial(modbus_t *contexto){
    modbus_close(contexto);
}


int conectar_modbus_serial(int modo_puerto, int baudrate, char *tty, int data_bits, char paridad, int stop_bits, modbus_t *contexto, int id_esclavo){
    
    //Configuramos el puerto de hardware
    int status = configurar_puerto_serial(modo_puerto, baudrate, paridad, stop_bits, data_bits);
    
    if(status == -1){
        printf("ERROR: No se pudo configurar el hardware del puerto serial\n");
        return -1;
    }
    
    contexto = modbus_new_rtu(tty, baudrate, paridad, data_bits, stop_bits);
    
    if (contexto == NULL) {
        printf("ERROR: No se pudo crear el contexto modbus.\n");
        return -1;
    }
    
    //Establecer nuestra id de esclavo
    modbus_set_slave(contexto, id_esclavo);
    
    //TODO crear mapping de bits y registers
    
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
    
    //4 bits (r/w), 3 input bits (read only), 0 HR (r/w), 16 input register (read only).
    mapeo_modbus = modbus_mapping_new(4, 3, 0, 16);
    
    
    if(mapeo_modbus == NULL){
        printf("ERROR: No se pudo crear el mapeo modbus de los registros\n");
        liberarContextoModbus(contexto);
        return -1;
    }
    
    //Iniciamos la conexion serial
    if(modbus_connect(contexto) == -1){
        printf("ERROR: No se pudo iniciar la comunicacion modbus serial\n");
        liberarContextoModbus(contexto);
        return -1;
    }
    
    return 0;
    
}

