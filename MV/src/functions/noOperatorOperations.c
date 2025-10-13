#include "../../include/noOperatorsOperations.h"
#include "../../include/registers.h"
#include <stdio.h>

void op_stop(void) {
    printf("STOP executing\n");
    writeRegister(3, 0xFFFFFFFF); // IP = -1 para detener ejecucion
}

void op_ret(){
    uint32_t IP;
    op_pop(IP);             // solo llamo para que se cambie SP y mbr
    getRegister(2,&IP);     // obtengo el valor que quedo en mbr (tope de pila), lo guardo en variable IP
    setRegister(3,IP);      // guardo en el IP el valor nuevo
}