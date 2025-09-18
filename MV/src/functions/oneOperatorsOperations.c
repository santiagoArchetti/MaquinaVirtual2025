#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include <stdio.h>

void op_sys(uint32_t op1) {
    printf("SYS ejecutado con código: %u\n", op1);
    // Aquí implementarías las llamadas al sistema
}

void op_jmp(uint32_t op1) {
    printf("JMP ejecutado a dirección: %u\n", op1);
    writeRegister(3, op1); // Actualizar IP
}

void op_jz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x40000000) {  // Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x40000000) && !(cc & 0x80000000)) {  // No Z y no N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x80000000) {  // N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x40000000)) {  // No Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if ((cc & 0x40000000) || (cc & 0x80000000)) {  // Z o N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x80000000)) {  // No N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_not(uint32_t op1) {
    uint32_t value, result;
    getRegister(op1, &value);
    result = ~value;  // NOT bit a bit
    writeRegister(op1, result);
    
    // Actualizar flags CC (R17)
    uint32_t cc = 0;
    if (result == 0) cc |= 0x40000000;  // Z flag
    if (result & 0x80000000) cc |= 0x80000000;  // N flag
    writeRegister(17, cc);
}
