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
    
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    
    // Obtenemos el tamaño del operando 1
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){                                // Inmediato en el primer operando
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){            // De registro a registro

            uint32_t value;
            getRegister((op2 & 0xFF),&value);
            setRegister((op1 & 0xFF),value);

        } else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            
            uint32_t a = op2 & 0xFFFF;
            if ( (op2 & 0x8000) != 0)
                a |= 0xFFFF0000;
            setRegister((op1 & 0xFF),a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            mbrValue = op2 & 0xFFFF;
            if ( (op2 & 0x8000) != 0)
                mbrValue |= 0xFFFF0000;
            setRegister(2, mbrValue);
            writeMemory(op1); 
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria

            uint32_t value;
            getRegister((op2 & 0xFF), &value);
            setRegister(2, value);
            writeMemory(op1);

        } else {
            readMemory(op2);
            getRegister(2, &mbrValue);
            
            if ( sizeOp1 == 3 ){     // De Memoria a memoria
                writeMemory(op1);
            } else if ( sizeOp1 == 1 ){     // De memoria a registro
                setRegister(op1 & 0xFF,mbrValue);
            }
        }
    }
}

void op_add(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){            // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a + b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);         // Sign-extend inmediato
            setRegister((op1 & 0xFF), a + b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister(2, a + b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a + b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a + b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registr
            readMemory(op2);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op1 & 0xFF), &b);
            setRegister((op1 & 0xFF), a + b);
        }
        setCondicion(a + b);
    }
}

void op_sub(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a - b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister((op1 & 0xFF), a - b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister(2, a - b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a - b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a - b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF),  a - b);
        }
        setCondicion(a - b);
    }
}

void op_mul(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), b * a);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister((op1 & 0xFF), b * a);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister(2, b * a);
            getRegister(2, &mbrValue);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, b * a);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, b * a);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF),  b * a);
        }
        setCondicion(b * a);
    }
}

void op_div(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            if (b != 0)
                setRegister((op1 & 0xFF), a / b);
            else
                setRegister(3,0xFFFFFFFF);
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            if (b != 0)
                setRegister((op1 & 0xFF), a / b);
            else
                setRegister(3,0xFFFFFFFF);
        
            } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);

            if (b != 0) {
                setRegister(2, a / b);
                writeMemory(op1);
            } else
                setRegister(3,0xFFFFFFFF);
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            if (b != 0) {
                setRegister(2, a / b);
                writeMemory(op1);
            } else
                setRegister(3,0xFFFFFFFF);
        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            if (b != 0) {
                setRegister(2, a / b);
                writeMemory(op1);
            } else
                setRegister(3,0xFFFFFFFF);

        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registro
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            if (b != 0)
                setRegister((op1 & 0xFF),  a / b);
            else
                setRegister(3,0xFFFFFFFF);
        }
        if (b != 0){ 
            setCondicion(a / b);
            setRegister(16, a % b);   // Guardamos el resto de la division en AC
        } else 
            setRegister(3,0xFFFFFFFF);
    }
}

void op_cmp(uint32_t op1, uint32_t op2) {
    
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;

        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            printf("a: %08X b: %08X\n", a, b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registro
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
        }
        setCondicion(a - b);
    }
}

void op_shl(uint32_t op1, uint32_t op2) {
    
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a << b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a << b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a << b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a << b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a << b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a << b);
        }
        setCondicion(a << b);
    }
}

void op_shr(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        uint32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a >> b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a >> b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a >> b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a >> b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registro
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a >> b);
        }
        setCondicion(a >> b);
    }
}

void op_sar(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a >> b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a >> b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a >> b);       // lo guardas de nuevo en el registro
            writeMemory(op1);
            
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a >> b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a >> b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a >> b);
        }
        setCondicion(a >> b);
    }
}

void op_and(uint32_t op1, uint32_t op2) {
    
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a & b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a & b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a & b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a & b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a & b);
            writeMemory(op1);
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registro
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a & b);
        }
        setCondicion(a & b);
    }
}

void op_or(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a | b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a | b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a | b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a | b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a | b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a | b);
        }
        setCondicion(a | b);
    }
}

void op_xor(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    printf("op1: %08X  op2: %08X\n",op1,op2);

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), a ^ b);
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = op2 & 0xFFFF;        // Para generalizar setCondicion
            setRegister((op1 & 0xFF), a ^ b);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, a ^ b);
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, a ^ b);
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a ^ b);
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), a ^ b);
        }
        setCondicion(a ^ b);
    }
}

void op_swap(uint32_t op1, uint32_t op2) {
    
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;
    uint8_t sizeOp2 = op2 >> 24;

    if ( sizeOp1 == 2 || sizeOp2 == 2){ // Porque no se puede intercambiar con un valor inmediato
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            setRegister((op1 & 0xFF), b);
            setRegister((op2 & 0xFF), a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, b);
            writeMemory(op1);
            setRegister((op2 & 0xFF),a);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, b);
            writeMemory(op1);
            setRegister(2, a);
            writeMemory(op2);
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, a);
            writeMemory(op1);
            setRegister((op1 & 0xFF),b);
        }
    }
}

void op_ldl(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            
            // Mantiene lo que estaba en la parte alta y cambia la parte baja
            setRegister((op1 & 0xFF), (a & 0xFFFF0000) | (b & 0x0000FFFF));
            
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister((op1 & 0xFF), (a & 0xFFFF0000) | (b & 0x0000FFFF));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister(2, (a & 0xFFFF0000) | (b & 0x0000FFFF));
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, (a & 0xFFFF0000) | (b & 0x0000FFFF));
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, (a & 0xFFFF0000) | (b & 0x0000FFFF));
            writeMemory(op1);
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), (a & 0xFFFF0000) | (b & 0x0000FFFF));
        }
    }
}

void op_ldh(uint32_t op1, uint32_t op2) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;

    if ( sizeOp1 == 2 ){ 
        setRegister(3,0xFFFFFFFF);
        return;
    } else {
        uint8_t sizeOp2 = op2 >> 24;
        int32_t a,b;
        
        
        if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
            getRegister((op1 & 0xFF), &a);
            getRegister((op2 & 0xFF), &b);
            
            // Mantenemos lo que estaba en la parte baja y cambiamos la parte alta
            setRegister((op1 & 0xFF), (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));    
    
        }  else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
            getRegister((op1 & 0xFF), &a);
            b = (int32_t)(int16_t)(op2 & 0xFFFF);
            setRegister((op1 & 0xFF), (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            b = op2 & 0xFFFF;
            setRegister(2, (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));
            writeMemory(op1);
    
        } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            getRegister((op2 & 0xFF), &b);
            setRegister(2, (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));
            writeMemory(op1);

        } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
            readMemory(op1);
            getRegister(2, &mbrValue);
            a = mbrValue;
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister(2, (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));
            writeMemory(op1);
            
        } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
            getRegister((op1 & 0xFF), &a);
            readMemory(op2);
            getRegister(2, &mbrValue);
            b = mbrValue;
            setRegister((op1 & 0xFF), (a & 0x0000FFFF) | ((b & 0x0000FFFF) << 16));
        }
    }
}

void op_rnd(uint32_t op1, uint32_t op2) {
    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;
    if ( sizeOp1 == 2 ){
        printf("ERROR: Operando 1 invalid\n");
        setRegister(3,0xFFFFFFFF);
        return;
    }

    uint8_t sizeOp2 = op2 >> 24;
    int32_t a = 0, b = 0;
    
    

    if ( sizeOp1 == 1 && sizeOp2 == 1 ){     // De registro a registro
        getRegister((op2 & 0xFF), &b);
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister((op1 & 0xFF), (uint32_t)a);

    } else if ( sizeOp1 == 1 && sizeOp2 == 2 ){     // Inmediato a registro
        b = (int32_t)(int16_t)(op2 & 0xFFFF);
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister((op1 & 0xFF), (uint32_t)a);

    } else if ( sizeOp1 == 3 && sizeOp2 == 2 ){     // Inmediato a memoria
        b = (int32_t)(int16_t)(op2 & 0xFFFF);
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister(2, (uint32_t)a);
        writeMemory(op1);

    } else if ( sizeOp1 == 3 && sizeOp2 == 1 ){     // De registro a memoria
        getRegister((op2 & 0xFF), &b);
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister(2, (uint32_t)a);
        writeMemory(op1);

    } else if ( sizeOp1 == 3 && sizeOp2 == 3 ){     // Memoria a memoria
        readMemory(op2);
        getRegister(2, &mbrValue);
        b = (int32_t)mbrValue;
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        if (b == 0)
            a = 0;
        else
            a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister(2, (uint32_t)a);
        writeMemory(op1);

    } else if ( sizeOp1 == 1 && sizeOp2 == 3 ){     // Memoria a registra
        readMemory(op2);
        getRegister(2, &mbrValue);
        b = (int32_t)mbrValue;
        if (b < 0) { setRegister(3, 0xFFFFFFFF); return; }
        a = (int32_t)(rand() % ((uint32_t)b + 1));
        setRegister((op1 & 0xFF), (uint32_t)a);
    }

    setCondicion((uint32_t)a);
}

