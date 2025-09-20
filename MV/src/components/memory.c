#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

MainMemory memory;  // definición global, celdas de 8 bits

// Función para inicializar la memoria 
void initMemory() {
    // Inicializar toda la memoria con ceros
    memset(memory.data, 0, MEMORY_SIZE);
    memory.initialized = 1;
    printf("Main memory initialized with %d bytes (16 KiB)\n", MEMORY_SIZE);
    printf("Available addresses: 0 to %d\n", MAX_ADDRESS);
}

// Función para escribir un byte en la memoria
int writeByte(int address, uint8_t value) {
    if (!memory.initialized) {
        printf("Error: Memory not initialized\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: address %d out of range (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }
    
    memory.data[address] = value;
    return 1;
}

// Función para leer un byte de la memoria
int readByte(int address, uint8_t* value) {
    if (!memory.initialized) {
        printf("Error: Memory not initialized\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: address %d out of range (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }    
    *value = memory.data[address];
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

/*------------------- Funciones auxiliares -----------------*/

void invertir (uint32_t *valueAux, uint32_t aux){
    
    for (int i = 0; i < 3; i++){        // Invierte el valor leido de la memoria big-endian
        *valueAux = aux & 0x000000FF;
        *valueAux = (*valueAux) << 8;
        aux = aux >> 8;
    }
}

void readMemory (uint8_t sizeOp, uint32_t *valueAux, uint32_t op) {
    
    uint32_t aux = 0x00000000;
    uint32_t value;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;

    getMemoryAccess(26, op, &logicalAddress, &fisicalAddress, &value); // 26 porque el registro 26 es el CS

    for (int i = 0; i < sizeOp ; i++) { // Lectura de memoria
        aux = aux | value;
        aux = aux << 8;
        op += 1;
        getMemoryAccess(26, op, &logicalAddress, &fisicalAddress, &value);  // 26 porque el registro 26 es el CS
    }
    invertir(valueAux, aux);
}

void writeMemory (uint8_t sizeOp, uint32_t aux, uint32_t op) {

    uint32_t value;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;

    for (int i = 0; i < sizeOp ; i++) {
        value = (uint8_t) (aux | 0x00);
        aux = aux >> 8;
        setMemoryAccess(26, op, &logicalAddress, &fisicalAddress, value);
        op += 1;
    }
}

void setCondicion(uint32_t value) {
    if (value == 0)
        writeRegister(17, 0x00000001);         // Setteamos el bit 0 (Z = 1)
    else 
        if (value < 0)
            writeRegister(17, 0x00000002);     // Setteamos el bit 1 (N = 1)
        else
            writeRegister(17, 0x00000000);     // Apagamos los bits si es positivo
}
