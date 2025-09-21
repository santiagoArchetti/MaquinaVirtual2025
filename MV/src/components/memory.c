#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

MainMemory memory;  // definicion global, celdas de 8 bits

// Funcion para inicializar la memoria 
void initMemory() {
    // Inicializar toda la memoria con ceros
    memset(memory.data, 0, MEMORY_SIZE);
    memory.initialized = 1;
    printf("Main memory initialized: %d bytes (16 KiB)\n", MEMORY_SIZE);
    printf("Available addresses: 0 to %d\n", MAX_ADDRESS);
}

// Funcion para escribir un byte en la memoria
int writeByte(int address, uint8_t value) {
    if (!memory.initialized) {
        printf("Error: Memory not initialized\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: Address %d out of range (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }
    
    memory.data[address] = value;
    return 1;
}

// Funcion para leer un byte de la memoria
int readByte(int address, uint8_t* value) {
    if (!memory.initialized) {
        printf("Error: Memory not initialized\n");
        return 0;
    }
    
    if (address < 0 || address > MAX_ADDRESS) {
        printf("Error: Address %d out of range (0-%d)\n", address, MAX_ADDRESS);
        return 0;
    }    
    *value = memory.data[address];
    return 1;
}

void memoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress, uint8_t size) {
    *logicalAddress = getLogicalAddress(csValue, IP);
    writeRegister(0, *logicalAddress);  //escribimos el LAR
    *physicalAddress = getFisicalAddress(*logicalAddress);
    
    // MAR: 2 bytes altos = cantidad de bytes (1), 2 bytes bajos = direccion fisica
    uint32_t marValue = (size << 16) | (*physicalAddress & 0xFFFF);
    writeRegister(1, marValue);  //escribimos el MAR con cantidad y direccion fisica
}

void getMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress,  uint8_t *opCode, uint8_t size) {
    
    memoryAccess(csValue, IP, logicalAddress, physicalAddress, size);

    readByte(*physicalAddress, opCode); // leo byte desde memoria
    writeRegister(2, *opCode);  //escribimos el opCode en el MBR

}

void setMemoryAccess(uint32_t csValue, uint32_t IP, uint32_t *logicalAddress, uint32_t *physicalAddress,  uint8_t value, uint8_t size) {

    memoryAccess(csValue, IP, logicalAddress, physicalAddress, size);

    writeRegister(2, value);
    writeByte(*physicalAddress, value);

}

/*------------------- Funciones auxiliares -----------------*/

void readMemory (uint8_t sizeOp, uint32_t *valueAux, uint32_t op) {
    
    uint8_t value = 0;
    uint32_t aux = 0x00000000;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;
    uint32_t csValue;

    getRegister(26, &csValue); // obtengo valor del registro CS

    aux = 0;
    for (int i = 0; i < sizeOp ; i++) { // Lectura de memoria
        getMemoryAccess(csValue, op, &logicalAddress, &fisicalAddress, &value, sizeOp);  // leo un byte en posiicon op en dir logica
        aux = aux << 8 | (uint32_t) value; // acumulo ya ordenado (para no invertir)
        op += 1;
    }

    *valueAux = aux;
}

void writeMemory (uint8_t sizeOp, uint32_t aux, uint32_t op) {

    uint8_t value;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;
    uint32_t csValue;

    getRegister(26, &csValue); // obtengo valor del registro

    for (int i = sizeOp - 1; i >= 0; i--) {
        value = (uint8_t) ((aux >> (8 * i)) & 0xFF); // para guardar primero bye + signif
        setMemoryAccess(csValue, op, &logicalAddress, &fisicalAddress, value, sizeOp);
        op += 1;
    }
}

void setCondicion(uint32_t value) {

    int32_t signedValue = (int32_t) value; //para interpretar el signo
    uint32_t cc = 0;

    uint32_t Z = (signedValue == 0) ? 1 : 0; // Z va a ser 1 si el valor es 0
    uint32_t N = (signedValue < 0) ? 1 : 0; // N va a ser 1 si el valor es negativo

    // Guardo en bits mas significativos del registro CC
    cc = (N << 31) | (Z << 30);

    writeRegister(17, cc);
}
