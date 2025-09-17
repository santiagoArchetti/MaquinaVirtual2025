#ifndef REGISTRIES_H
#define REGISTRIES_H

#include <stdint.h>

#define REGISTRIES_SIZE 32

typedef struct {
    uint32_t registerValue[REGISTRIES_SIZE];
} Registers;

extern Registers registers;

void writeRegister(int register, uint32_t value);  
void getRegister(int register, uint32_t* value);
void initRegisters();  // función extra para inicializar

#endif
