#ifndef ONE_OPERATOR_OPERATIONS_H
#define ONE_OPERATOR_OPERATIONS_H

#include <stdint.h>

// Operaciones con un solo operando
void op_sys(uint32_t op1,uint8_t op1Bytes);
void op_jmp(uint32_t op1,uint8_t op1Bytes);
void op_jz(uint32_t op1,uint8_t op1Bytes);
void op_jp(uint32_t op1,uint8_t op1Bytes);
void op_jn(uint32_t op1,uint8_t op1Bytes);
void op_jnz(uint32_t op1,uint8_t op1Bytes);
void op_jnp(uint32_t op1,uint8_t op1Bytes);
void op_jnn(uint32_t op1,uint8_t op1Bytes);
void op_not(uint32_t op1,uint8_t op1Bytes);

#endif
