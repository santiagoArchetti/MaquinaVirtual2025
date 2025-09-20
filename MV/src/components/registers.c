#include "../../include/registers.h"
#include <stdio.h>
#include <math.h>

Registers registers;  // definicion global

void initRegisters() { // inicializamos en 0 todos los registros
    for (int i = 0; i < REGISTERS_SIZE; i++) {
        registers.registerValue[i] = 0;
    }
}

// Funcion para escribir un registro
void writeRegister(int regIndex, uint32_t value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: invalid register index | %d \n",regIndex);
        return;
    }
    registers.registerValue[regIndex] = value;
}

// Funcion para cargar el valor de un registro
void getRegister(int regIndex, uint32_t* value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: invalid register index | %d \n",regIndex);
        return;
    }
    *value = registers.registerValue[regIndex];
}

int opCodeExists(uint8_t opCode){
    opCode = opCode & 0x1F;
    if ( ( opCode >= 0x10 && opCode <= 0x1F ) || ( opCode >= 0x00 && opCode <= 0x08 ) || opCode == 0x0F)
        return 1;
    else{
        writeRegister(3,0xFFFFFFFF);
        return 0;
    }
}

int binADecimal(uint32_t op) {
    op = op & 0x0000003F;   // Sacamos el tipo de operando (3F por los 32 registros)
    int sum = 0;
    for (int i = 0 ; i < 6 ; i++){
        if ( (op & 0x00000001) == 0x00000001 && sum < 32)
            sum += pow(2, i);
        op = op >> 1;
    }
    return sum;
}