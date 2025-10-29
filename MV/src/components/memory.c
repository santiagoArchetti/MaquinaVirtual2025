#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include "../../include/segmentTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

MainMemory memory;  // definicion global, celdas de 8 bits
int flag;

// Funcion para inicializar la memoria con tamaño dinámico
void initMemory(int memorySize) {
    // Si ya está inicializada, liberar primero
    if (memory.initialized && memory.data != NULL) {
        free(memory.data);
    }
    
    // Asignar memoria dinámicamente
    memory.data = (uint8_t*)malloc(memorySize * sizeof(uint8_t));
    
    if (memory.data == NULL) {
        printf("Error: Could not allocate memory of size %d bytes\n", memorySize);
        memory.initialized = 0;
        memory.size = 0;
        return;
    }
    
    // Inicializar toda la memoria con ceros
    memset(memory.data, 0, memorySize);
    memory.size = memorySize;
    memory.initialized = 1;
    
    printf("Main memory initialized: %d bytes (%.2f KiB)\n", memorySize, memorySize / 1024.0);
    printf("Available addresses: 0 to %d\n", memorySize - 1);
}

// Funcion para liberar la memoria
void freeMemory() {
    if (memory.initialized && memory.data != NULL) {
        free(memory.data);
        memory.data = NULL;
        memory.size = 0;
        memory.initialized = 0;
    }
}

// Funcion para escribir un byte en la memoria
int writeByte(int address, uint8_t value) {
    if (!memory.initialized) {
        printf("Error: Memory not initialized\n");
        return 0;
    }
    
    if (address < 0 || address > memory.size) {
        printf("Error: Address %d out of range (0-%d)\n", address, memory.size - 1);
        setRegister(3,0xFFFFFFFF);
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
    
    if (address < 0 || address >= memory.size) {
        printf("Error: Address %d out of range (0-%d)\n", address, memory.size - 1);
        setRegister(3,0xFFFFFFFF);
        return 0;
    }    
    *value = memory.data[address];
    return 1;
}

//Seteo del MAR y LAR 
void memoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress, uint32_t aux) {
    if (SegmentValue >= 0 && SegmentValue <= 0x8){
        *logicalAddress = getLogicalAddress(SegmentValue, OffsetValue);
        setRegister(0, *logicalAddress);  //escribimos el LAR
        *physicalAddress = getFisicalAddress(*logicalAddress);
        
        // uint32_t marValue = (aux << 16) | (*physicalAddress & 0xFFFF);
        uint32_t marValue = 4 - aux;
        marValue = (marValue << 16) | (*physicalAddress & 0xFFFF);
        setRegister(1, marValue);  //escribimos el MAR con cantidad y direccion fisica
    } else {
        printf("Error: Segmento invalido\n");
        setRegister(3,0xFFFFFFFF);
    }
}

/*------------------- Funciones auxiliares -----------------*/
void readMemory (uint32_t op) {
    
    uint32_t logicalAddress;
    uint32_t physicalAddress;
    
    // Extraer el segundo byte más significativo para saber el registro (bits 16–23)
    uint8_t extractedByte = (op >> 16) & 0xFF;
    uint32_t registerValue;
    getRegister( ((op >> 16) & 0x1F) | 0x00000000, &registerValue);
    uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
    uint16_t offset = (op & 0xFFFF) + (registerValue & 0xFFFF);

    uint8_t data;
    uint32_t mbrValue = 0x0;  // Inicializar mbrValue
    uint32_t aux = ((op >> 22) & 0x3); // sirve para saber si es 'l' (0), 'w' (2) o 'b' (3)
    memoryAccess((uint32_t)segmentRegister, (uint32_t)offset, &logicalAddress, &physicalAddress, aux); //setea configuracion de memoria para lectura
    uint32_t marValue;
    getRegister(1, &marValue);
    int bytesToRead = marValue >> 16;
    // Lectura de bytes de memoria (big-endian)
    if (isValidAddress(physicalAddress, bytesToRead, segmentRegister)) {
        for (int i = 0; i < bytesToRead; i++ ) {
            readByte(physicalAddress + i, &data);
            mbrValue = (mbrValue << 8) | data;
        }
    } else {
        printf("Error: Direccion invalida\n");
        setRegister(3,0xFFFFFFFF);
        return;
    }
    
    switch (bytesToRead){
        case 1: mbrValue = (((mbrValue >> 7) & 0x01) == 0x1) ? (mbrValue | 0xFFFFFF00) : mbrValue; break;
        case 2: mbrValue = (((mbrValue >> 15) & 0x01) == 0x1) ? (mbrValue | 0xFFFF0000) : mbrValue; break;
    }
    
    setRegister(2, mbrValue);
}

void writeMemory (uint32_t op) {

    uint8_t value;
    uint32_t logicalAddress;
    uint32_t physicalAddress;

    // Extraer el segundo byte más significativo para saber el registro (bits 16–23)
    uint8_t extractedByte = (op >> 16) & 0xFF;
    uint32_t registerValue;
    getRegister(((op >> 16) & 0x1F) | 0x00000000, &registerValue);
    uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
    uint16_t offset = (op & 0xFFFF) + (registerValue & 0xFFFF);
     
    uint32_t mbr,mar;
    getRegister(2, &mbr);
    int32_t mbrValue = (int32_t)mbr;  
    uint32_t aux = ((op >> 22) & 0x3); // sirve para saber si es 'l', 'w' o 'b'
    memoryAccess(segmentRegister, offset, &logicalAddress, &physicalAddress, aux);
    getRegister(1, &mar);
    // int bytesToRead = 4 - ((mar >> 16) & 0xFF);
    int bytesToRead = mar >> 16;

    // Escritura de bytes en memoria (big-endian)
    if (isValidAddress(physicalAddress, bytesToRead, segmentRegister)) {
        for (int i = 0; i < bytesToRead; i++) {
            value = (uint8_t) ((mbrValue >> ((bytesToRead - 1 - i) * 8)) & 0xFF);  // big Endian
            writeByte(physicalAddress + i, value);
        }
    } else {
        printf("Error: Direccion invalida\n");
        setRegister(3,0xFFFFFFFF);
        return;
    }
}

void writeStack(uint32_t SP) {

    uint32_t logicalAddress, physicalAddress, mbr;
    SP -= 4;
    memoryAccess((SP >> 16), (SP & 0xFFFF), &logicalAddress, &physicalAddress, 0x0);
    getRegister(2, &mbr);
    for (int i = 0; i < 4; i++){
        writeByte( (physicalAddress + i), (uint8_t) (mbr >> ((3 - i) * 8)) );    // Escritura big-endian
    }
    setRegister(7,SP);
}

void readStack(uint32_t SP){

    uint8_t mbrValue;
    uint32_t logicalAddress, physicalAddress, mbr = 0x0;

    memoryAccess((SP >> 16), (SP & 0xFFFF), &logicalAddress, &physicalAddress, 0x0);

    for (int i = 0; i < 4; i++){
        readByte( physicalAddress + i, &mbrValue);
        mbr = (mbr << 8 | mbrValue);
    }
    SP += 4;
    setRegister(2,mbr);
    setRegister(7,SP);
}
