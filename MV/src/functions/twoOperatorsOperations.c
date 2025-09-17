#include "twoOperatorsOperations.h"
#include "registries.h"
#include "include/memory.h"
#include "include/operations.h"
#include "src/functions/operations.c"
#include "src/functions/directions.c"
#include "src/components/memory.c"
#include "include/segmentTable.h"
#include "src/functions/segmentTable.c"
#include "include/registries.h"
#include "src/functions/registries.c"
#include "include/segmentTable.h"
#include "src/functions/segmentTable.c"
#include <stdio.h>

void op_mov(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    if (op1Bytes == 2) {  //si el primero es inmediato
        writeRegister(3,0xFFFFFF);
    } else if (op1Bytes == 1 && op2Bytes == 1) {  //si el primero es registro y el segundo es registro
        uint32_t value;
        getRegister(op2, &value);
        writeRegister(op1,value);
    } else if (op1Bytes == 1 && op2Bytes == 2) {  //si el primero es registro y el segundo es inmediato
        writeRegister(op1,op2);
    } else if (op1Bytes == 1 && op2Bytes == 3) {  //si el primero es inmediato y el segundo es memoria
        uint32_t value;
        uint32_t csValue = getRegister(27);
        uint32_t logicalAddress;
        uint32_t fisicalAddress;
        uint8_t opCode = 0x10;
        setMemoryAccess(csValue, op2, &logicalAddress, &fisicalAddress, &opCode);

        for (int i = 0; i < op2Bytes; i++) {
            readByte(fisicalAddress, &opCode);
            value = value << 8 | opCode;
            fisicalAddress = fisicalAddress + 1;
        }
        writeRegister(op1, value);
    }else if (op1Bytes == 3 && op2Bytes == 1) {  //si el primero es memoria y el segundo es re

void op_add(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 + value2;
    writeRegister(op1, result);
}

void op_sub(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 - value2;
    writeRegister(op1, result);
}

void op_mul(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 * value2;
    writeRegister(op1, result);
}   

void op_div(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 / value2;
    writeRegister(op1, result);
}

void op_cmp(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 - value2;
    writeRegister(op1, result);
}

void op_shl(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 << value2;
    writeRegister(op1, result);
}

void op_shr(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;
    writeRegister(op1, result);
}

void op_sar(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;
    writeRegister(op1, result);
}
    
void op_and(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 & value2;
    writeRegister(op1, result);
}
    
void op_or(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 | value2;
    writeRegister(op1, result);
}
    
void op_xor(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {

    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 ^ value2;
    writeRegister(op1, result);
}
    
void op_swap(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {    
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 ^ value2;
    writeRegister(op1, result);
}
    
void op_ldl(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 << value2;
    writeRegister(op1, result);
}
    
void op_ldh(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;
    writeRegister(op1, result);
}

void op_rnd(uint32_t op1, uint32_t op2,uint8_t op1Bytes,uint8_t op2Bytes) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;
    writeRegister(op1, result);
}

