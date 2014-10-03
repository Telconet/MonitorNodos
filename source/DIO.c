#include "DIO.h"



/**
 *
 *Pin 1, 3, 5, 7, 9, 11, 13, 15 --> DIO0, DIO1, DIO2, DIO3, DIO4, DIO5, DIO6, DIO7
 *
 *DDR --> salida = 1, entrada = 0
 *
 *Puerta: DIO0
 *Relays: DIO4 - DIO7
 */

void configurarPuertosDIO(){
    
    volatile unsigned int *PORTDR, *PORTDDR, *GPIOBDB;
    unsigned char *inicio;
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    
    //Inicio de los registros que controlan los puertos I/O
    inicio = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80840000);
    
    //PORT DDR es el registro de direccion del puerto (entrada o salida, bit en 1 salida, 0 entrada)
    PORTDDR = (unsigned int *) (inicio + 0x14);
    
    //PORT DR es el registro de estado de los puerto I/O, si estan como salida, los puertos se activan (active high)
    //escribiendo 1 al bit correspondiente
    PORTDR = (unsigned int *) (inicio + 0x04);
    
    GPIOBDB = (unsigned int *)(inicio + 0xC4); 
    
    //Configuramos los puertos 0-3  como salida y puertos 4 - 7 como entrada para monitoreo de puerta
    //*PORTDDR = 0xf0;
    *PORTDDR = 0x0f;
    
    //Debounce en bit 0
    *GPIOBDB = 0x01;
    
    //Ponemos los puertos en estado logico 0
    *PORTDR = 0x00;
    
    munmap((void *)inicio, getpagesize());
    close(fd);
}

/**
 *Asume que puertos estan configurados
 */
int activarPuerto(puerto_DIO puerto){
    
    volatile unsigned int *PORTDR;
    unsigned char statusActualPuerto, mascara;
    unsigned char *inicio;
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    
    if(fd == -1){
        return -1;
    }
    
    //Inicio de los registros que controlan los puertos I/O
    inicio = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80840000);
    
  
    //PORT DR es el registro de estado de los puerto I/O, si estan como salida, los puertos se activan (active high)
    //escribiendo 1 al bit correspondiente
    PORTDR = (unsigned int *) (inicio + 0x04);
    
    //leemos el status actual del puerto
    statusActualPuerto = *PORTDR;
    
    //creamos la mascara
    switch(puerto){
        case puerto_DIO_0:
            mascara = 0x01;
            break;
        case puerto_DIO_1:
            mascara = 0x02;
            break;
        case puerto_DIO_2:
            mascara = 0x04;
            break;
        case puerto_DIO_3:
            mascara = 0x08;
            break;
        case puerto_DIO_4:
            mascara = 0x10;
            break;
        case puerto_DIO_5:
            mascara = 0x20;
            break;
        case puerto_DIO_6:
            mascara = 0x40;
            break;
        case puerto_DIO_7:
            mascara = 0x80;
            break;
        default:
            mascara = 0x00;
            break;  
    }
 
    //activamos el puerto.
    *PORTDR = statusActualPuerto | mascara;
    
    munmap((void *)inicio, getpagesize());
    close(fd);
    
    return 0;  
}

int desactivarPuerto(puerto_DIO puerto){
    
    volatile unsigned int *PORTDR;
    unsigned char statusActualPuerto, mascara;
    unsigned char *inicio;
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    
    if(fd == -1){
        return -1;
    }
    
    //Inicio de los registros que controlan los puertos I/O
    inicio = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80840000);
    
  
    //PORT DR es el registro de estado de los puerto I/O, si estan como salida, los puertos se activan (active high)
    //escribiendo 1 al bit correspondiente
    PORTDR = (unsigned int *) (inicio + 0x04);
    
    //leemos el status actual del puerto
    statusActualPuerto = *PORTDR;
    
    //creamos la mascara
    switch(puerto){
        case puerto_DIO_0:
            mascara = 0xFE;     //0b1111 1110
            break;
        case puerto_DIO_1:
            mascara = 0xFD;     //0b1111 1101
            break;
        case puerto_DIO_2:
            mascara = 0xFB;     //0b1111 1011
            break;
        case puerto_DIO_3:
            mascara = 0xF7;     //0b1111 0111
            break;
        case puerto_DIO_4:
            mascara = 0xEF;
            break;
        case puerto_DIO_5:
            mascara = 0xDF;
            break;
        case puerto_DIO_6:
            mascara = 0xBF;
            break;
        case puerto_DIO_7:
            mascara = 0x7F;
            break;
        default:
            mascara = 0xFF;
            break;  
    }
 
    //desactivamos el puerto.
    *PORTDR = statusActualPuerto & mascara;
    
    munmap((void *)inicio, getpagesize());
    close(fd);
    return 0;
}


status_puerto_DIO statusPuerto(puerto_DIO puerto){
    
    volatile unsigned int *PORTDR;
    unsigned char statusActualPuerto, mascara;
    unsigned char *inicio;
    
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    
    //Inicio de los registros que controlan los puertos I/O
    inicio = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80840000);
    
  
    //PORT DR es el registro de estado de los puerto I/O, si estan como salida, los puertos se activan (active high)
    //escribiendo 1 al bit correspondiente
    PORTDR = (unsigned int *) (inicio + 0x04);
    
    switch(puerto){
        case puerto_DIO_0:
            mascara = 0x01;
            break;
        case puerto_DIO_1:
            mascara = 0x02;
            break;
        case puerto_DIO_2:
            mascara = 0x04;
            break;
        case puerto_DIO_3:
            mascara = 0x08;
            break;
        case puerto_DIO_4:
            mascara = 0x10;
            break;
        case puerto_DIO_5:
            mascara = 0x20;
            break;
        case puerto_DIO_6:
            mascara = 0x40;
            break;
        case puerto_DIO_7:
            mascara = 0x80;
            break;
        default:
            mascara = 0x00;
            break;  
    }
    
    statusActualPuerto = *PORTDR;
    
#ifdef DEBUG
    printf("puerto PORTDR: %x\n", statusActualPuerto);
    printf("Mascara:       %x\n", mascara);
#endif

    //Hacemos unmap de archivo mapeado a memoria!! Caso contrario el uso continuara incrementando!!
    
    munmap((void *)inicio, getpagesize());
    close(fd);
    
    if(statusActualPuerto & mascara){
        return PUERTO_ON;
    }
    else{
        return PUERTO_OFF;
    } 
}
