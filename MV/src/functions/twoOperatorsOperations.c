#include "../../include/twoOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/operations.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <stdlib.h>

/*------------------- Funciones principales -----------------*/

void op_mov(uint32_t op1, uint32_t op2) {
    
    // Obtenemos el tamaño del operando 1
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){                            // Inmediato en el primer operando
        writeRegister(3,0xFFFFFFFF);
    } else {
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        uint8_t sizeOp2 = op2 >> 24;        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro

            uint32_t value;
            getRegister(reg2,&value);
            writeRegister(reg1,value);

        } else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            
            writeRegister(reg1,op2);

        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            
            uint32_t logicalAddress;
            uint32_t fisicalAddress;
            uint8_t firstByte;
            uint8_t secondByte;
            firstByte = (op2 >> 8) & 0xFF;
            writeRegister(2, (uint32_t)firstByte); //escribimos el dato en el mbr
            setMemoryAccess(26, op1, &logicalAddress, &fisicalAddress); 
            secondByte = op2 & 0xFF; 
            writeRegister(2, (uint32_t)secondByte); //escribimos el dato en el mbr
            setMemoryAccess(26, op1, &logicalAddress, &fisicalAddress); 

        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria

            uint32_t value;
            uint32_t aux;

            getRegister(reg2, &value);
            invertir(&aux, value);
            writeMemory(sizeOp1, aux, op1);

        } else {
            
            uint32_t aux = 0x00000000;
            
            readMemory(sizeOp2, &aux, op2);
            
            if ( sizeOp1 == 3 ){     // De Memoria a memoria

                writeMemory(sizeOp1, aux, op1);

            } else if ( sizeOp1 == 1 ){     // De memoria a registro
                writeRegister(reg1,aux);
            }
        }
    }
}

void op_add(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a + b);
    
        } else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2 & 0x00FFFFFF;        // Para generalizar setCondicion
            writeRegister(reg1, a + b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2 & 0x00FFFFFF;       // La mascara es para sacarle el codigo de operando
            writeMemory(sizeOp1, a + b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a + b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a + b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registr
            readMemory(sizeOp2, &a, op2);
            getRegister(reg1, &b);
            writeRegister(reg1, a + b);
        }
        setCondicion(a + b);
    }
}

void op_sub(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, b - a);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, b - a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b - a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1,  b - a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b - a, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1,  b - a);
        }
        setCondicion(b - a);
    }
}

void op_mul(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, b * a);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, b * a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b * a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1,  b * a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b * a, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1,  b * a);
        }
        setCondicion(b * a);
    }
}

void op_div(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, b / a);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, b / a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b / a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1,  b / a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b / a, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1,  b / a);
        }
        setCondicion(b / a);
        writeRegister(16, b % a);   // Guardamos el resto de la division en AC
    }
}

void op_cmp(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
        }
        setCondicion(b - a);
    }
}

void op_shl(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a << b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a << b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a << b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a << b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a << b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a << b);
        }
        setCondicion(b << a);
    }
}

void op_shr(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a >> b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a >> b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a >> b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a >> b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a >> b);
        }
        setCondicion(a >> b);
    }
}
// Iguales -- Ver
void op_sar(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a >> b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a >> b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a >> b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a >> b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a >> b);
        }
        setCondicion(a >> b);
    }
}

void op_and(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a & b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a & b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a & b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a & b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a & b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a & b);
        }
        setCondicion(a & b);
    }
}

void op_or(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a | b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a | b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a | b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a | b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a | b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a | b);
        }
        setCondicion(a | b);
    }
}

void op_xor(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, a ^ b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, a ^ b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a ^ b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, a ^ b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a ^ b, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, a ^ b);
        }
        setCondicion(a ^ b);
    }
}

void op_swap(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;
    uint8_t sizeOp2 = op2 >> 24;

    if ( sizeOp1 == 2 || sizeOp2 == 2){ // Porque no se puede intercambiar con un valor inmediato
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            writeRegister(reg1, b);
            writeRegister(reg2, a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, b, op1);
            writeRegister(reg2, a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, b, op1);
            writeMemory(sizeOp2, a, op2);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, b);
            writeMemory(sizeOp2, a, op2);
        }
        // SWAP no modifica CC - solo intercambia valores
    }
}

void op_ldl(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            
            // Mantiene lo que estaba en la parte alta y cambia la parte baja
            writeRegister(reg1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
            
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, (a & 0xFFFF0000) | (b & 0x0000FFFF), op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, (a & 0xFFFF0000) | (b & 0x0000FFFF), op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,(a & 0xFFFF0000) | (b & 0x0000FFFF), op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
        }
        setCondicion((a & 0xFFFF0000) | (b & 0x0000FFFF));
    }
}

void op_ldh(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg1, &a);
            getRegister(reg2, &b);
            
            // Mantenemos lo que estaba en la parte baja y cambiamos la parte alta
            writeRegister(reg1, (a & 0x0000FFFF) | (b & 0xFFFF0000));    
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister(reg1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(reg1, (a & 0x0000FFFF) | (b & 0xFFFF0000));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, (a & 0x0000FFFF) | (b & 0xFFFF0000), op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(reg2, &b);
            writeMemory(sizeOp1, (a & 0x0000FFFF) | (b & 0xFFFF0000), op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,(a & 0x0000FFFF) | (b & 0xFFFF0000), op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(reg1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(reg1, (a & 0x0000FFFF) | (b & 0xFFFF0000));
        }
        setCondicion((a & 0x0000FFFF) | (b & 0xFFFF0000));
    }
}

void op_rnd(uint32_t op1, uint32_t op2) {
        
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        int reg1 = binADecimal(op1);
        int reg2 = binADecimal(op2);
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister(reg2, &b);
            a = (uint32_t)rand() % (b + 1);
            writeRegister(reg1, a);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registr
            a = (uint32_t)rand() % (op2 + 1); // Para generalizar setCondicion
            writeRegister(reg1, a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            a = (uint32_t)rand() % (op2 + 1);
            writeMemory(sizeOp1, a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            getRegister(reg2, &b);
            a = (uint32_t)rand() % (b + 1);
            writeMemory(sizeOp1, a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp2, &b, op2);
            a = (uint32_t)rand() % (b + 1);
            writeMemory(sizeOp1, a, op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            readMemory(sizeOp2, &b, op2);
            a = (uint32_t)rand() % (b + 1);
            writeRegister(reg1, a);
        }
        setCondicion(a);
    }
}

/*  FORMATO AUXILIAR

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        
        uint8_t sizeOp2 = op2 >> 24;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){            // De registro a registro

        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){    // Inmediato a registro
        
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria

        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria

        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registro
        }
    }
*/