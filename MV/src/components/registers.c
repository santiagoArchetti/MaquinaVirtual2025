#include "../../include/registers.h"
#include <stdio.h>
#include <math.h>

Registers registers;  // definicion global

void initRegisters() { // inicializamos en 0 todos los registros
    for (int i = 0; i < REGISTERS_SIZE; i++) {
        registers.registerValue[i] = 0;
    }
}

// Funcion para escribir un registro
void writeRegister(int regIndex, uint32_t value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", regIndex);
        return;
    }
    registers.registerValue[regIndex] = value;
}

// Funcion para cargar el valor de un registro
void getRegister(int regIndex, uint32_t* value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", regIndex);
        return;
    }
    *value = registers.registerValue[regIndex];
}

int opCodeExists(uint8_t opCode){
    opCode = opCode & 0x1F;
    if ( ( opCode >= 0x10 && opCode <= 0x1F ) || ( opCode >= 0x00 && opCode <= 0x08 ) || opCode == 0x0F)
        return 1;
    else{
        writeRegister(3,0xFFFFFFFF);
        return 0;
    }
}

int binADecimal(uint32_t op) {
    op = op & 0x0000003F;   // Sacamos el tipo de operando (3F por los 32 registros)
    int sum = 0;
    for (int i = 0 ; i < 6 ; i++){
        if ( (op & 0x00000001) == 0x00000001 && sum < 32)
            sum += pow(2, i);
        op = op >> 1;
    }
    return sum;
}

void getOperandName(uint32_t name) {
 
      if ( (name>>24) == 0x01){
      // Tipo registro - imprimir nombre del registro
      char* nameRegister = "UNK";
      int regIndex = name & 0x0000001F;
      switch(regIndex) {
          case 0: nameRegister = "LAR"; break;
          case 1: nameRegister = "MAR"; break;
          case 2: nameRegister = "MBR"; break;
          case 3: nameRegister = "IP"; break;
          case 4: nameRegister = "OPC"; break;
          case 5: nameRegister = "OP1"; break;
          case 6: nameRegister = "OP2"; break;
          case 7: nameRegister = "UNK"; break;
          case 8: nameRegister = "UNK"; break;
          case 9: nameRegister = "UNK"; break;
          case 10: nameRegister = "EAX"; break;
          case 11: nameRegister = "EBX"; break;
          case 12: nameRegister = "ECX"; break;
          case 13: nameRegister = "EDX"; break;
          case 14: nameRegister = "EFX"; break;
          case 15: nameRegister = "AC"; break;
          case 16: nameRegister = "CC"; break;
          case 17: nameRegister = "UNK"; break;
          case 18: nameRegister = "UNK"; break;
          case 19: nameRegister = "UNK"; break;
          case 20: nameRegister = "UNK"; break;
          case 21: nameRegister = "UNK"; break;
          case 22: nameRegister = "UNK"; break;
          case 23: nameRegister = "UNK"; break;
          case 24: nameRegister = "UNK"; break;
          case 25: nameRegister = "UNK"; break;
          case 26: nameRegister = "CS"; break;
          case 27: nameRegister = "DS"; break;
          case 28: nameRegister = "UNK"; break;
          case 29: nameRegister = "UNK"; break;
          case 30: nameRegister = "UNK"; break;
          case 31: nameRegister = "UNK"; break;
      }
      printf("%s", nameRegister);
    } else if ( (name>>24) == 0x02){
      // Tipo inmediato 
      printf("%04X", (uint16_t)(name & 0x0000FFFF));
    } else {
      // Tipo memoria - extraer registro y offset
      uint8_t regIndex = (name >> 16) & 0xFF;  // Segundo byte más significativo
      uint16_t offset = name & 0xFFFF;         // Dos bytes menos significativos
      
      // Obtener nombre del registro
      char* regName = "UNK";
      switch(regIndex) {
          case 0: regName = "LAR"; break;
          case 1: regName = "MAR"; break;
          case 2: regName = "MBR"; break;
          case 3: regName = "IP"; break;
          case 4: regName = "OPC"; break;
          case 5: regName = "OP1"; break;
          case 6: regName = "OP2"; break;
          case 10: regName = "EAX"; break;
          case 11: regName = "EBX"; break;
          case 12: regName = "ECX"; break;
          case 13: regName = "EDX"; break;
          case 14: regName = "EFX"; break;
          case 15: regName = "AC"; break;
          case 16: regName = "CC"; break;
          case 26: regName = "CS"; break;
          case 27: regName = "DS"; break;
          default: regName = "UNK"; break;
      }
      
      printf("[%s+%04X]", regName, offset);
    }
}