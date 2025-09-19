#include "../../include/registers.h"
#include <stdio.h>

Registers registers;  // definición global

void initRegisters() { // inicializamos en 0 todos los registros
    for (int i = 0; i < REGISTERS_SIZE; i++) {
        registers.registerValue[i] = 0;
    }
}

// Función para escribir un registro
void writeRegister(int regIndex, uint32_t value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: invalid register index\n");
        return;
    }
    registers.registerValue[regIndex] = value;
}

// Función para cargar el valor de un registro
void getRegister(int regIndex, uint32_t* value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: invalid register index\n");
        return;
    }
    *value = registers.registerValue[regIndex];
}
