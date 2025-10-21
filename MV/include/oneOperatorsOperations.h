#ifndef ONE_OPERATOR_OPERATIONS_H
#define ONE_OPERATOR_OPERATIONS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Operaciones con un solo operando
void op_sys(uint32_t op1, FILE *arch);
void op_jmp(uint32_t op1);
void op_jz(uint32_t op1);
void op_jp(uint32_t op1);
void op_jn(uint32_t op1);
void op_jnz(uint32_t op1);
void op_jnp(uint32_t op1);
void op_jnn(uint32_t op1);
void op_not(uint32_t op1);
void op_push(uint32_t op1);
void op_pop(uint32_t op1);
void op_call(uint32_t op1);

// Funciones auxiliares para llamadas al sistema
void setImage(FILE *arch);
void sys_read(void);
void sys_write(void);
void sys_string_read();
void sys_string_write();
void sys_clear_screen();
void sys_breakpoint(FILE *arch);

#endif
