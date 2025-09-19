#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include <stdio.h>

void setCondicion(uint32_t value) {
    if (value == 0)
        writeRegister(17, 0x00000001);         // Setteamos el bit 0 (Z = 1)
    else 
        if (value < 0)
            writeRegister(17, 0x00000002);     // Setteamos el bit 1 (N = 1)
        else
            writeRegister(17, 0x00000000);     // Apagamos los bits si es positivo
}

void op_sys(uint32_t op1) {
    printf("SYS ejecutado con código: %u\n", op1);
    // Aquí implementarías las llamadas al sistema
}

/* --------------------- JUMPS ------------------------ */
void op_jmp(uint32_t op1) {
    printf("JMP ejecutado a dirección: %u\n", op1);
    writeRegister(3, op1); // Actualizar IP
}

void op_jz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000001) {  // Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001) && !(cc & 0x00000002)) {  // No Z y no N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000002) {  // N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001)) {  // No Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if ((cc & 0x00000001) || (cc & 0x00000002)) {  // Z o N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000002)) {  // No N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_not(uint32_t op1) {

    uint8_t sizeOp1 = op1 >> 24;
    uint32_t aux;

    if ( sizeOp1 == 1 ){    
        /**
        No puede negar un inmediato? 
        porque donde lo guardaria
        Pero igual deveria modificar el CC no?
        */
        writeRegister(3,0xFFFFFFFF);
    } else if ( sizeOp1 == 2){     // De registro
        getRegister(op1, &aux);
        writeRegister(op1, ~aux);
    
    } else if ( sizeOp1 == 3){     // De memoria
        readMemory(sizeOp1, &aux, op1);
        writeMemory(sizeOp1, ~aux, op1);

    }
    setCondicion(~aux);
}

