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

extern int flag;

void initMemory();
int writeByte(int address, uint8_t value);
int readByte(int address, uint8_t* value);
void memoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress, uint32_t aux);
void readMemory (uint32_t op);
void writeMemory (uint32_t op);
int binADecimal(uint32_t op);
void writeStack(uint32_t SP);
void readStack(uint32_t SP);

#endif