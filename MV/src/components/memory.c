#include "memory.h"
#include <stdio.h>
#include <string.h>

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
        printf("Error: Dirección %d fuera de rango (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }
    
    memory.data[address] = value;
    printf("Escrito byte 0x%02X en dirección %d\n", value, address);
    return 1;
}

// Función para leer un byte de la memoria
int readByte(int address, uint8_t* value) {
    if (!memory.initialized) {
        printf("Error: Memoria no inicializada\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: Dirección %d fuera de rango (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }    
    *value = memory.data[address];
    printf("Leído byte 0x%02X desde dirección %d\n", *value, address);
    return 1;
}


void setMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *fisicalAddress, uint8_t *opCode) {
    getLogicalAddress(csValue, IP, logicalAddress); 
    writeRegister(0, logicalAddress);  //escribimos el lar
    getFisicalAddress(logicalAddress, fisicalAddress); 
    
    writeRegister(1, fisicalAddress);  //escribimos el fisical address en el MAR 
    readByte(fisicalAddress, opCode);
    writeRegister(2, &opCode);  //escribimos el opCode en el MBR
}
