#include "noOperatorOperations.h"
#include "registries.h"
#include <stdio.h>

void op_stop(void) {
    printf("STOP ejecutando\n");
    writeRegister(3, 0xFFFFFFFF); // IP = -1 para detener ejecución
}
