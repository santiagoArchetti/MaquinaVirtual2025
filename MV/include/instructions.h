#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>


void analizeInstruction(uinT8_t instruction);  
void createTwoOperatorsInstruction(int register, uint16_t* value);
void createOneOperatorInstruction(int register, uint16_t* value);
void createNoOperatorInstruction(int register, uint16_t* value);

#endif
