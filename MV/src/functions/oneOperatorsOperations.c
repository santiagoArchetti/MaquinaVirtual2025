#include "oneOperatorsOperations.h"
#include "registries.h"
#include "memory.h"
#include "segmentTable.h"
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
    printf("JZ ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si cero
}

void op_jp(uint32_t op1) {
    printf("JP ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si positivo
}

void op_jn(uint32_t op1) {
    printf("JN ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si negativo
}

void op_jnz(uint32_t op1) {
    printf("JNZ ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si no cero
}

void op_jnp(uint32_t op1) {
    printf("JNP ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si no positivo
}

void op_jnn(uint32_t op1) {
    printf("JNN ejecutado a dirección: %u\n", op1);
    // Aquí implementarías el salto condicional si no negativo
}

void op_not(uint32_t op1) {
    printf("NOT ejecutado con operando: %u\n", op1);
    // Aquí implementarías la operación NOT
}
