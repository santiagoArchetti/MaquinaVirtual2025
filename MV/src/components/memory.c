#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

MainMemory memory;  // definición global, celdas de 8 bits

// Función para inicializar la memoria 
void initMemory() {
    // Inicializar toda la memoria con ceros
    memset(memory.data, 0, MEMORY_SIZE);
    memory.initialized = 1;
    printf("Memoria principal inicializada con %d bytes (16 KiB)\n", MEMORY_SIZE);
    printf("Direcciones disponibles: 0 a %d\n", MAX_ADDRESS);
}

// Función para escribir un byte en la memoria
int writeByte(int address, uint8_t value) {
    if (!memory.initialized) {
        printf("Error: Memoria no inicializada\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: direccion %d fuera de rango (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }
    
    memory.data[address] = value;
    printf("Escrito byte 0x%02X en direccion %d\n", value, address);
    return 1;
}

// Función para leer un byte de la memoria
int readByte(int address, uint8_t* value) {
    if (!memory.initialized) {
        printf("Error: Memoria no inicializada\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: direccion %d fuera de rango (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }    
    *value = memory.data[address];
    printf("Leído byte 0x%02X irección %d\n", *value, address);
    return 1;
}

void memoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress) {
    *logicalAddress = getLogicalAddress(csValue, IP);
    writeRegister(0, *logicalAddress);  //escribimos el LAR
    *physicalAddress = getFisicalAddress(*logicalAddress);
    
    // MAR: 2 bytes altos = cantidad de bytes (1), 2 bytes bajos = direccion física
    uint32_t marValue = (1 << 16) | (*physicalAddress & 0xFFFF);
    writeRegister(1, marValue);  //escribimos el MAR con cantidad y direccion física
}

void getMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress,  uint8_t *opCode) {
    
    memoryAccess(csValue, IP, logicalAddress, physicalAddress);

    writeRegister(2, *opCode);  //escribimos el opCode en el MBR
    readByte(*physicalAddress, opCode);

}

void setMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress,  uint8_t value) {

    memoryAccess(csValue, IP, logicalAddress, physicalAddress);

    writeRegister(2, value);
    writeByte(*physicalAddress, value);

}