#ifndef TWO_OPERATORS_OPERATIONS_H
#define TWO_OPERATORS_OPERATIONS_H

#include <stdint.h>

void op_mov(uint32_t op1, uint32_t op2);
void op_add(uint32_t op1, uint32_t op2);
void op_sub(uint32_t op1, uint32_t op2);
void op_mul(uint32_t op1, uint32_t op2);
void op_div(uint32_t op1, uint32_t op2);
void op_cmp(uint32_t op1, uint32_t op2);
void op_shl(uint32_t op1, uint32_t op2);
void op_shr(uint32_t op1, uint32_t op2);
void op_sar(uint32_t op1, uint32_t op2);
void op_and(uint32_t op1, uint32_t op2);
void op_or(uint32_t op1, uint32_t op2);
void op_xor(uint32_t op1, uint32_t op2);
void op_swap(uint32_t op1, uint32_t op2);
void op_ldl(uint32_t op1, uint32_t op2);
void op_ldh(uint32_t op1, uint32_t op2);
void op_rnd(uint32_t op1, uint32_t op2);

// Funciones auxiliares
void setCondicion(uint32_t value);
void readMemory(uint8_t sizeOp, uint32_t *valueAux, uint32_t op);
void writeMemory(uint8_t sizeOp, uint32_t aux, uint32_t op);

#endif
