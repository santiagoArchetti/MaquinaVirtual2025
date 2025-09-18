#include "../../include/twoOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/operations.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <stdlib.h>

void op_mov(uint32_t op1, uint32_t op2) {
    
    // Obtenemos el tamaño del operando 1
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){                            // Inmediato en el primer operando
        // Lanzar error
    } else{

        uint8_t sizeOp2 = op2 >> 24;        
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro

            uint32_t value;
            getRegister(op2,&value);
            writeRegister(op1,value);

        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // De memoria a registro
            
            uint32_t csValue;
            uint32_t logicalAddress;
            uint32_t fisicalAddress;
            uint8_t opCode;
            setMemoryAccess(csValue, op2, &logicalAddress, &fisicalAddress, &opCode);

            for (int i = 0; i < sizeOp2; i++){
                
            }

        } else if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // Inmediato a registro
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // Inmediato a memoria
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De Memoria a memoria
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a memoria
            
        }
    }
    
    
    // MOV: copia el valor de op2 a op1
    
    writeRegister(op1, op2);

    // Falta los casos especiales, solo guarda en los registros
}

void op_add(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 + value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_sub(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 - value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_mul(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 * value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_div(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    
    if (value2 == 0) {
        printf("Error: División por cero\n");
        writeRegister(3, 0xFFFFFFFF);  // Detener ejecución
        return;
    }
    
    result = value1 / value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_cmp(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 - value2;
    
    // CMP no modifica op1, solo actualiza flags
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_shl(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 << value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_shr(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_sar(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 >> value2;  // En C, >> es arithmetic shift para signed
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_and(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 & value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_or(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 | value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_xor(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2, result;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    result = value1 ^ value2;
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}

void op_swap(uint32_t op1, uint32_t op2) {
    uint32_t value1, value2;
    getRegister(op1, &value1);
    getRegister(op2, &value2);
    
    // Intercambiar valores
    writeRegister(op1, value2);
    writeRegister(op2, value1);
}

void op_ldl(uint32_t op1, uint32_t op2) {
    // LDL: cargar parte baja (16 bits menos significativos)
    uint32_t value;
    getRegister(op2, &value);
    uint32_t lowPart = value & 0xFFFF;
    writeRegister(op1, lowPart);
}

void op_ldh(uint32_t op1, uint32_t op2) {
    // LDH: cargar parte alta (16 bits más significativos)
    uint32_t value;
    getRegister(op2, &value);
    uint32_t highPart = (value >> 16) & 0xFFFF;
    writeRegister(op1, highPart);
}

void op_rnd(uint32_t op1, uint32_t op2) {
    // RND: generar número aleatorio (simplificado)
    uint32_t randomValue = (uint32_t)rand() % (op2 + 1);
    writeRegister(op1, randomValue);
}