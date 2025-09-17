#include "directions.h"
#include <stdio.h>
#include "../components/segmentTable.h"

uint32_t getLogicalAddress(uint16_t segment, uint16_t offset) {  //devuelve la dirección lógica (no valida limites)
    uint32_t logicalAddress = (segment << 16) | offset;
    return logicalAddress;
}


uint32_t getFisicalAddress(uint32_t logicalAddress) {
    uint32_t segment = logicalAddress >> 16; 
    uint32_t offset = (uint16_t)logicalAddress & 0xFFFF;
    uint16_t base, length;
    getSegmentRange(segment, &base, &length);
    uint32_t fisicalAddress = base + offset;  //suma bit a bit de resultado entero que da una posicion en la memoria fisica
    return fisicalAddress;
}
    
int isValidAddress(uint32_t fisicalAddress, uint32_t offset, uint16_t segment) { //valida si la dirección física es válida 
    uint16_t base, length;
    getSegmentRange(segment, &base, &length);

    if (fisicalAddress + offset > length + base || fisicalAddress < base) {
        return 0;
    }
    
    return 1;
}