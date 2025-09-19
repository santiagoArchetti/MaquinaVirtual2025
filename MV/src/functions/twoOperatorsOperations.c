#include "../../include/twoOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/operations.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include <stdio.h>
#include <stdlib.h>

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

/*------------------- Funciones principales -----------------*/

void op_mov(uint32_t op1, uint32_t op2) {
    
    // Obtenemos el tamaño del operando 1
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){                            // Inmediato en el primer operando
        writeRegister(3,0xFFFFFFFF);
    } else {

        uint8_t sizeOp2 = op2 >> 24;        
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro

            uint32_t value;
            getRegister(op2,&value);
            writeRegister(op1,value);

        } else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            
            writeRegister(op1,op2);

        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            
            uint32_t logicalAddress;
            uint32_t fisicalAddress;

            setMemoryAccess(26, op1, &logicalAddress, &fisicalAddress, op2); //ver

        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria

            uint32_t value;
            uint32_t aux;

            getRegister(op2, &value);
            invertir(&aux, value);
            writeMemory(sizeOp1, aux, op1);

        } else {
            
            uint32_t aux = 0x00000000;
            
            readMemory(sizeOp2, &aux, op2);
            
            if ( sizeOp1 == 3 ){     // De Memoria a memoria

                writeMemory(sizeOp1, aux, op1);

            } else if ( sizeOp1 == 2 ){     // De memoria a registro
                writeRegister(op1,aux);
            }
        }
    }
}

void op_add(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a + b);
    
        } else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a + b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a + b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a + b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a + b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registr
            readMemory(sizeOp2, &a, op2);
            getRegister(op1, &b);
            writeRegister(op1, a + b);
        }
        setCondicion(a + b);
    }
}

void op_sub(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, b - a);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, b - a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b - a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1,  b - a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b - a, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1,  b - a);
        }
        setCondicion(b - a);
    }
}

void op_mul(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, b * a);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, b * a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b * a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1,  b * a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b * a, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1,  b * a);
        }
        setCondicion(b * a);
    }
}

void op_div(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, b / a);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, b / a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, b / a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1,  b / a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,  b / a, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1,  b / a);
        }
        setCondicion(b / a);
        writeRegister(16, b % a);   // Guardamos el resto de la division en AC
    }
}

void op_cmp(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
        }
        setCondicion(b - a);
    }
}

void op_shl(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a << b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a << b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a << b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a << b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a << b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a << b);
        }
        setCondicion(b << a);
    }
}

void op_shr(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a >> b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a >> b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a >> b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a >> b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a >> b);
        }
        setCondicion(a >> b);
    }
}
// Iguales -- Ver
void op_sar(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a >> b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a >> b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a >> b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a >> b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a >> b);
        }
        setCondicion(a >> b);
    }
}

void op_and(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a & b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a & b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a & b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a & b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a & b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a & b);
        }
        setCondicion(a & b);
    }
}

void op_or(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a | b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a | b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a | b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a | b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a | b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a | b);
        }
        setCondicion(a | b);
    }
}

void op_xor(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, a ^ b);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, a ^ b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, a ^ b, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, a ^ b, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, a ^ b, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, a ^ b);
        }
        setCondicion(a ^ b);
    }
}

void op_swap(uint32_t op1, uint32_t op2) {
    
    uint8_t sizeOp1 = op1 >> 24;
    uint8_t sizeOp2 = op2 >> 24;

    if ( sizeOp1 == 1 || sizeOp2 == 1){ // Porque no se puede intercambiar con un valor inmediato
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            writeRegister(op1, b);
            writeRegister(op2, a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, b, op1);
            writeRegister(op2, a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1, b, op1);
            writeMemory(sizeOp2, a, op2);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, b);
            writeMemory(sizeOp2, a, op2);
        }
        // SWAP no modifica CC - solo intercambia valores
    }
}

void op_ldl(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            
            // Mantiene lo que estaba en la parte alta y cambia la parte baja
            writeRegister(op1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
            
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, (a & 0xFFFF0000) | (b & 0x0000FFFF), op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, (a & 0xFFFF0000) | (b & 0x0000FFFF), op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,(a & 0xFFFF0000) | (b & 0x0000FFFF), op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, (a & 0xFFFF0000) | (b & 0x0000FFFF));
        }
        setCondicion((a & 0xFFFF0000) | (b & 0x0000FFFF));
    }
}

void op_ldh(uint32_t op1, uint32_t op2) {

    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op1, &a);
            getRegister(op2, &b);
            
            // Mantenemos lo que estaba en la parte baja y cambiamos la parte alta
            writeRegister(op1, (a & 0x0000FFFF) | (b & 0xFFFF0000));    
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registro
            getRegister(op1, &a);
            b = op2;        // Para generalizar setCondicion
            writeRegister(op1, (a & 0x0000FFFF) | (b & 0xFFFF0000));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            readMemory(sizeOp1, &a, op1);
            b = op2;
            writeMemory(sizeOp1, (a & 0x0000FFFF) | (b & 0xFFFF0000), op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            readMemory(sizeOp1, &a, op1);
            getRegister(op2, &b);
            writeMemory(sizeOp1, (a & 0x0000FFFF) | (b & 0xFFFF0000), op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp1, &a, op1);
            readMemory(sizeOp2, &b, op2);
            writeMemory(sizeOp1,(a & 0x0000FFFF) | (b & 0xFFFF0000), op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister(op1, &a);
            readMemory(sizeOp2, &b, op2);
            writeRegister(op1, (a & 0x0000FFFF) | (b & 0xFFFF0000));
        }
        setCondicion((a & 0x0000FFFF) | (b & 0xFFFF0000));
    }
}

void op_rnd(uint32_t op1, uint32_t op2) {
    // RND: generar número aleatorio (simplificado)
        
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 1 ){ 
        writeRegister(3,0xFFFFFFFF);
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        if ( sizeOp1 == 2 && sizeOp2 == 2 ){     // De registro a registro
            getRegister(op2, &b);
            a = (uint32_t)rand() % (b + 1);
            writeRegister(op1, a);
    
        }  else if ( sizeOp1 == 2 && sizeOp2 == 1 ){     // Inmediato a registr
            a = (uint32_t)rand() % (op2 + 1); // Para generalizar setCondicion
            writeRegister(op1, a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // Inmediato a memoria
            a = (uint32_t)rand() % (op2 + 1);
            writeMemory(sizeOp1, a, op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // De registro a memoria
            getRegister(op2, &b);
            a = (uint32_t)rand() % (b + 1);
            writeMemory(sizeOp1, a, op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(sizeOp2, &b, op2);
            a = (uint32_t)rand() % (b + 1);
            writeMemory(sizeOp1, a, op1);
            
        } else if ( sizeOp1 == 2 && sizeOp2 == 3 ){     // Memoria a registra
            readMemory(sizeOp2, &b, op2);
            a = (uint32_t)rand() % (b + 1);
            writeRegister(op1, a);
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