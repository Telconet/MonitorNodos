#include "modbustn.h"



int modbus_test(){
    
    modbus_t *ctx;
    
    ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
    
    if (ctx == NULL) {
        fprintf(stderr, "No se pudo crear el contexto modbus.\n");
        return -1;
    }
    
    return 0;
}

