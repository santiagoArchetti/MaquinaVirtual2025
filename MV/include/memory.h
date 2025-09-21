#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 16384
#define MAX_ADDRESS 16383

typedef struct {
    uint8_t data[MEMORY_SIZE];
    int initialized;
} MainMemory;

extern MainMemory memory;

void initMemory();
int writeByte(int address, uint8_t value);
int readByte(int address, uint8_t* value);
void memoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress,uint8_t size);
void getMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress, uint8_t *opCode,uint8_t size);
void setMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress, uint8_t value,uint8_t size);
void readMemory (uint8_t sizeOp, uint32_t *valueAux, uint32_t op);
void writeMemory (uint8_t sizeOp, uint32_t aux, uint32_t op);
void setCondicion(uint32_t value);
int binADecimal(uint32_t op);
#endif