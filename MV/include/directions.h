#include <stdint.h>

#ifndef DIRECTIONS_H
#define DIRECTIONS_H

uint32_t getFisicalAddress(uint32_t logicalAddress);
uint32_t getLogicalAddress(uint16_t segment, uint16_t offset);
int isValidAddress(uint32_t physicalAddress, uint32_t bytesCount, uint16_t segment);
#endif
