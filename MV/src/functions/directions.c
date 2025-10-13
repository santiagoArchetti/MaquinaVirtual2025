#include "../../include/directions.h"
#include <stdio.h>
#include "../../include/segmentTable.h"

uint32_t getLogicalAddress(uint16_t segment, uint16_t offset) {  //devuelve la direccion logica (no valida limites)
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
    
int isValidAddress(uint32_t physicalAddress, uint32_t bytesCount, uint16_t segment) { //valida si la direccion fisica es valida 
    uint16_t base, length;
    getSegmentRange(segment, &base, &length);

    // Validar que la direccion fisica este dentro del segmento
    if (physicalAddress < base || physicalAddress >= base + length) {
        return 0;
    }
    
    // Validar que el acceso no se salga del segmento
    if (physicalAddress + bytesCount > base + length) {
        return 0;
    }
    
    return 1;
}