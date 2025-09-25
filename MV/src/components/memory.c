#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include "../../include/segmentTable.h"
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

//Seteo del MAR y LAR 
void memoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress) {
    *logicalAddress = getLogicalAddress(SegmentValue, OffsetValue);
    writeRegister(0, *logicalAddress);  //escribimos el LAR
    *physicalAddress = getFisicalAddress(*logicalAddress);
    uint32_t marValue = *physicalAddress;
    writeRegister(1, marValue);  //escribimos el MAR con cantidad y direccion fisica
}

//setea configuracion de memoria para lectura, y guarda el dato leido en el MBR
void getMemoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress) {

    memoryAccess(SegmentValue, OffsetValue, logicalAddress, physicalAddress);
    uint8_t data;
    readByte(*physicalAddress, &data);
    writeRegister(2, data); //el dato leido se guarda en el MBR

}

//setea configuracion de memoria para escritura, desde el dato del mbr
void setMemoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress) {
    memoryAccess(SegmentValue, OffsetValue, logicalAddress, physicalAddress);
    uint32_t mbrValue;
    getRegister(2, &mbrValue);
    uint8_t value = (uint8_t)(mbrValue & 0xFF);
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
    
    uint8_t value;
    uint32_t aux = 0x00000000;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;
    
     // Extraer el segundo byte más significativo (bits 16–23)
     uint8_t extractedByte = (op >> 16) & 0xFF;
     uint32_t registerValue;
     getRegister(extractedByte, &registerValue);  
     uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
     uint16_t offset = op & 0xFFFF;                
    
    // Siempre leer 4 bytes de memoria (big-endian)
    for (int i = 0; i < 4; i++ ) {
        getMemoryAccess(segmentRegister, offset + i, &logicalAddress, &fisicalAddress);

        if (isValidAddress(fisicalAddress, 1, segmentRegister)) {
            uint32_t mbrValue;
            getRegister(2, &mbrValue);
            value = (uint8_t)(mbrValue & 0xFF);
            aux = aux | (value << ((4 - 1 - i) * 8));  // aux = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4 big Endian
        }else{
            writeRegister(3,0xFFFFFFFF);
            return;
        }
    }
    *valueAux = aux;
}

void writeMemory (uint8_t sizeOp, uint32_t aux, uint32_t op) {

    uint8_t value;
    uint32_t logicalAddress;
    uint32_t fisicalAddress;
    uint32_t valueToWrite;

    uint8_t extractedByte = (op >> 16) & 0xFF;
    uint32_t registerValue;
    getRegister(binADecimal(extractedByte), &registerValue);  
    uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
    uint16_t offset = op & 0xFFFF;  
    
    if (sizeOp == 1){
        valueToWrite = aux & 0x00FFFFFF;
    }else{
        valueToWrite = aux;
    }
    // Siempre escribir 4 bytes en memoria (big-endian)
    for (int i = 0; i < 4; i++) {
        value = (uint8_t) ((valueToWrite >> ((4 - 1 - i) * 8)) & 0xFF);  // big Endian
        writeRegister(2,value);
        setMemoryAccess(segmentRegister, offset + i, &logicalAddress, &fisicalAddress);
        if (isValidAddress(fisicalAddress, 1, segmentRegister)) {
            writeByte(fisicalAddress, value);
        }else{
            printf("Error: Invalid address\n");
            writeRegister(3,0xFFFFFFFF);
            return;
        }
    }
}
