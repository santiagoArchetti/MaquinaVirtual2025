#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define DEFAULT_MEMORY_SIZE 16384

typedef struct {
    uint8_t *data;          // Puntero dinámico en lugar de array fijo
    int size;               // Tamaño actual de la memoria
    int initialized;
} MainMemory;

extern MainMemory memory;

extern int flag;

void initMemory(int memorySize);
void freeMemory();
int writeByte(int address, uint8_t value);
int readByte(int address, uint8_t* value);
void memoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress, uint32_t aux);
void readMemory (uint32_t op);
void writeMemory (uint32_t op);
int binADecimal(uint32_t op);
void writeStack(uint32_t SP);
void readStack(uint32_t SP);

#endif