#include "../../include/operations.h"
#include <stdio.h>
#include "../../include/twoOperatorsOperations.h"
#include "../../include/oneOperatorsOperations.h"
#include "../../include/noOperatorsOperations.h"

OpFunc0 opTable0[256];  // Operaciones sin operandos
OpFunc1 opTable1[256];  // Operaciones con 1 operando
OpFunc2 opTable2[256];  // Operaciones con 2 operandos

void analizeInstruction(uint8_t instruction, uint8_t *op1Bytes, uint8_t *op2Bytes) {
    uint8_t op1Type, op2Type;
    
    if ((instruction & 0x10) == 0x10) {             // Para saber si es de 2 operandos (0001 0000)

        op1Type = (instruction >> 4) & 0x03;    // bits 5-4 → tipo A
        op2Type = (instruction >> 6) & 0x03;    // bits 7-6 → tipo B
        *op1Bytes = op1Type;
        *op2Bytes = op2Type;
    } else if ((instruction & 0x0F) != 0x0F) {      // Caso: instruccion con 1 operando
 
        op1Type = (instruction >> 6) & 0x03;    // bits 7-6 → tipo A
        *op1Bytes = op1Type;
        *op2Bytes = 0x0;
    } else {                                        // Caso: instruccion sin operandos
        *op2Bytes = 0x0;
        *op1Bytes = 0x0;
    }
}

uint8_t analizeOperator(uint8_t operatorType) {
    switch(operatorType) {
        case 0x00: return 0;  // ninguno - 0 bytes
        case 0x01: return 1;  // registro - 1 byte
        case 0x02: return 2;  // inmediato - 2 bytes
        case 0x03: return 3;  // memoria - 3 bytes
        default: return 0;
    }
}

// Inicializa las tablas con todas las punteros a operaciones
void initOpTable(void) {
    // Inicializar todas las tablas en NULL
    for (int i = 0; i < 256; i++) {
        opTable0[i] = NULL;
        opTable1[i] = NULL;
        opTable2[i] = NULL;
    }
    
    // Operaciones con 2 operandos
    opTable2[0x10] = op_mov;  // MOV
    opTable2[0x11] = op_add;  // ADD
    opTable2[0x12] = op_sub;  // SUB
    opTable2[0x13] = op_mul;  // MUL
    opTable2[0x14] = op_div;  // DIV
    opTable2[0x15] = op_cmp;  // CMP
    opTable2[0x16] = op_shl;  // SHL
    opTable2[0x17] = op_shr;  // SHR
    opTable2[0x18] = op_sar;  // SAR
    opTable2[0x19] = op_and;  // AND
    opTable2[0x1A] = op_or;   // OR
    opTable2[0x1B] = op_xor;  // XOR
    opTable2[0x1C] = op_swap; // SWAP
    opTable2[0x1D] = op_ldl;  // LDL
    opTable2[0x1E] = op_ldh;  // LDH
    opTable2[0x1F] = op_rnd;  // RND

    // Operaciones con 1 operando
    opTable1[0x00] = op_sys;  // SYS
    opTable1[0x01] = op_jmp;  // JMP
    opTable1[0x02] = op_jz;   // JZ
    opTable1[0x03] = op_jp;   // JP
    opTable1[0x04] = op_jn;   // JN
    opTable1[0x05] = op_jnz;  // JNZ
    opTable1[0x06] = op_jnp;  // JNP
    opTable1[0x07] = op_jnn;  // JNN
    opTable1[0x08] = op_not;  // NOT

    // Operaciones sin operandos
    opTable0[0x0F] = op_stop; // STOP
}


/* ejemplo de uso fuera del archivo operations.c
uint8_t opcode = memoria[ip]; // leiste un byte de memoria que corresponde al opcode
uint32_t op1 = ...;           // operandos decodificados
uint32_t op2 = ...;

if (opTable[opcode] != NULL) {
    opTable[opcode](&cpu, op1, op2); // ejecutas la operacion
} else {
    printf("Instruccion invalida: 0x%02X\n", opcode);
    // man
 */

 
/* -------- Otra posible version -----------------
void analizeInstruction(uint8_t instruction, uint8_t *op1Bytes, uint8_t *op2Bytes) {
    uint8_t opCode;
    uint8_t op1Type, op2Type;
    if ((instruction & 0x10) == 0x10) {             // Para saber si es de 2 operandos

        op1Type = (instruction >> 4) & 0x03;    // bits 5-4 → tipo A
        op2Type = (instruction >> 6) & 0x03;    // bits 7-6 → tipo B
        *op2Bytes = analizeOperator(op2Type);
        *op1Bytes = analizeOperator(op1Type);
    } else if ((instruction & 0x0F) != 0x0F) {      // Caso: instruccion con 1 operando
 
        op1Type = (instruction >> 6) & 0x03;    // bits 7-6 → tipo A
        *op1Bytes = analizeOperator(op1Type);
        *op2Bytes = 0;
    } else {                                        // Caso: instruccion sin operandos
        *op2Bytes = 0;
        *op1Bytes = 0;
    }
}
*/

/* ------------- original -----------------------
void analizeInstruction(uint8_t instruction, uint8_t *op1Bytes, uint8_t *op2Bytes) {
    uint8_t opCode;
    uint8_t op1Type, op2Type;
    if ((instruction & 0xC0) != 0x00) {  // bits 7-6 distintos de 00

        op2Type = (instruction >> 4) & 0x03; // bits 5-4 → tipo A
        op1Type = (instruction >> 6) & 0x03; // bits 7-6 → tipo B
        *op2Bytes = analizeOperator(op2Type);
        *op1Bytes = analizeOperator(op1Type);
    }
    // Caso: instruccion con 1 operando
    else if ((instruction & 0xE0) == 0x00 && (instruction & 0x1F) != 0x00) {  
 
        op2Type = (instruction >> 6) & 0x03; // bits 7-6 → tipo A
        *op2Bytes = analizeOperator(op2Type);
        *op1Bytes = 0;
    }
    // Caso: instruccion sin operandos
    else {
        *op2Bytes = 0;
        *op1Bytes = 0;
    }
}
*/
