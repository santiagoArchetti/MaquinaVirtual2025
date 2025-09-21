#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

#define REGISTERS_SIZE 32

typedef struct {
    uint32_t registerValue[REGISTERS_SIZE];
} Registers;

extern Registers registers;

void writeRegister(int regIndex, uint32_t value);  
void getRegister(int regIndex, uint32_t* value);
void initRegisters();  // funcion extra para inicializar
int opCodeExists(uint8_t opCode);
int binADecimal(uint32_t op);
void getOperandName(uint32_t name);
#endif
