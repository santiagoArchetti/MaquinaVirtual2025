#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "include/operations.h"
#include "src/functions/operations.c"
#include "src/functions/directions.c"

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
void setMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *localAddress, uint32_t *fisicalAddress, uint8_t *opCode);
#endif