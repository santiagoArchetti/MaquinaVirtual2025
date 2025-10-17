#include "../../include/memory.h"
#include "../../include/registers.h"
#include "../../include/directions.h"
#include "../../include/segmentTable.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

MainMemory memory;  // definicion global, celdas de 8 bits
int flag;

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
void memoryAccess(uint32_t SegmentValue, uint32_t OffsetValue, uint32_t *logicalAddress, uint32_t *physicalAddress, uint32_t aux) {
    *logicalAddress = getLogicalAddress(SegmentValue, OffsetValue);
    setRegister(0, *logicalAddress);  //escribimos el LAR
    *physicalAddress = getFisicalAddress(*logicalAddress);
    
    uint32_t marValue = (aux << 16) | (*physicalAddress & 0xFFFF);
    setRegister(1, marValue);  //escribimos el MAR con cantidad y direccion fisica
}

/*------------------- Funciones auxiliares -----------------*/

void readMemory (uint32_t op) {
    
    uint32_t logicalAddress;
    uint32_t physicalAddress;
    
    // Extraer el segundo byte más significativo para saber el registro (bits 16–23)
    uint8_t extractedByte = (op >> 16) & 0x1F;
    uint32_t registerValue;
    getRegister(extractedByte, &registerValue);
    uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
    uint16_t offset = (op & 0xFFFF) + (registerValue & 0xFFFF);

    uint8_t data;
    uint32_t mbrValue = 0;  // Inicializar mbrValue
    uint32_t aux = ((op >> 22) & 0x3); // sirve para saber si es 'l', 'w' o 'b'
    memoryAccess(segmentRegister, offset, &logicalAddress, &physicalAddress, aux); //setea configuracion de memoria para lectura
    
    uint32_t marValue;
    getRegister(1, &marValue);
    int bytesToRead = 4 - ((marValue >> 16) & 0xFF);

    // Lectura de bytes de memoria (big-endian)
    for (int i = 0; i < bytesToRead; i++ ) {       
        if (isValidAddress(physicalAddress + i, 1, segmentRegister)) {
            readByte(physicalAddress + i, &data);
            mbrValue = (mbrValue << 8) | data;
        }else{
            setRegister(3,0xFFFFFFFF);
            return;
        }
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
    getRegister(binADecimal(extractedByte), &registerValue);
    uint16_t segmentRegister = (uint16_t)(registerValue >> 16);
    uint16_t offset = (op & 0xFFFF) + (registerValue & 0xFFFF);
     
    uint32_t mbr,mar;
    getRegister(2, &mbr);
    int32_t mbrValue = (int32_t)mbr;  
    uint32_t aux = ((op >> 22) & 0x3); // sirve para saber si es 'l', 'w' o 'b'
    memoryAccess(segmentRegister, offset, &logicalAddress, &physicalAddress, aux);
    getRegister(1, &mar);
    int bytesToRead = 4 - ((mar >> 16) & 0xFF);
    
    // Escritura de bytes en memoria (big-endian)
    for (int i = 0; i < bytesToRead; i++) {
        value = (uint8_t) ((mbrValue >> ((bytesToRead - i) * 8)) & 0xFF);  // big Endian
        if (isValidAddress(physicalAddress + i, 1, segmentRegister)) {
            writeByte(physicalAddress + i, value);
        }else{
            printf("Error: Direccion invalida\n");
            setRegister(3,0xFFFFFFFF);
            return;
        }
    }
}

void writeStack(uint32_t SP) {

    uint32_t logicalAddress, physicalAddress, mbr;

    SP -= 4;
    memoryAccess((SP >> 16), (SP & 0xFFFF), &logicalAddress, &physicalAddress, 0x4);
    getRegister(2, &mbr);

    for (int i = 0; i < 4; i++)
        writeByte( (physicalAddress + i), (uint8_t) (mbr >> (3 - i)) );    // Escritura big-endian
    
    setRegister(7,SP);
}

void readStack(uint32_t SP){

    uint8_t mbrValue;
    uint32_t logicalAddress, physicalAddress, mbr = 0x0;

    memoryAccess((SP >> 16), (SP & 0xFFFF), &logicalAddress, &physicalAddress, 0x4);

    for (int i = 0; i < 4; i++){
        readByte( physicalAddress + i, &mbrValue);
        SP--;
        mbr = (mbr << 8 | mbrValue);
    }
    setRegister(2,mbr);
    setRegister(7,SP);
}
