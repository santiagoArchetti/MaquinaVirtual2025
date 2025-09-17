#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdint.h>

// Definir diferentes tipos de punteros a función
typedef void (*OpFunc0)(void);                    // Sin operandos
typedef void (*OpFunc1)(uint32_t);                // Un operando
typedef void (*OpFunc2)(uint32_t, uint32_t);      // Dos operandos

// Tablas de operaciones separadas (256 posibles cada una)
extern OpFunc0 opTable0[256];  // Operaciones sin operandos
extern OpFunc1 opTable1[256];  // Operaciones con 1 operando
extern OpFunc2 opTable2[256];  // Operaciones con 2 operandos

void initOpTable(void);
void analizeInstruction(uint8_t instruction, uint8_t* op1Bytes, uint8_t* op2Bytes);
uint8_t analizeOperator(uint8_t operator);

#endif
