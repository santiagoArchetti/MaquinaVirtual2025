#include "../../include/noOperatorsOperations.h"
#include "../../include/registers.h"
#include <stdio.h>

void op_stop(void) {
    printf("STOP executing\n");
    writeRegister(3, 0xFFFFFFFF); // IP = -1 para detener ejecución
}
