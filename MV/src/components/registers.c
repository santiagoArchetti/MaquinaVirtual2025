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
        printf("Error: invalid register index | %d \n",regIndex);
        return;
    }
    registers.registerValue[regIndex] = value;
}

// Función para cargar el valor de un registro
void getRegister(int regIndex, uint32_t* value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: invalid register index | %d \n",regIndex);
        return;
    }
    *value = registers.registerValue[regIndex];
}

int opCodeExists(uint8_t opCode){
    if ( ( opCode >= 0x10 && opCode <= 0x1F ) || ( opCode >= 0x00 && opCode <= 0x08 ) || opCode == 0x0F)
        return 1;
    else
        return 0;
}