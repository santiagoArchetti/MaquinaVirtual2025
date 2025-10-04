#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "include/operations.h"
#include "include/memory.h"
#include "include/registers.h"
#include "include/segmentTable.h"
#include "include/directions.h"


void beginExecution(FILE *file, int debug) {
    uint8_t opCode;
    char header[5] = {0};
    uint8_t version;
    uint16_t codeSize;

    if (fread(header, sizeof(uint8_t), 5, file) != 5 ||
        fread(&version, sizeof(uint8_t), 1, file) != 1) {
        printf("Error: No se pudo leer el header del archivo\n");
        return;
    }

    if (strncmp(header, "VMX25", 5) != 0 || version != 0x01) {
        printf("Error: File not valid (Header: %.5s, Version: 0x%02X)\n", header, version);
        return;
    }

    initMemory();
    initRegisters();
    initSegmentTable();
    initOpTable();

    uint8_t sizeHigh, sizeLow;
    fread(&sizeHigh, sizeof(uint8_t), 1, file);
    fread(&sizeLow, sizeof(uint8_t), 1, file);
    codeSize = (sizeHigh << 8) | sizeLow;  // Big-endian
    
    printf("Size of the code: %u bytes\n", codeSize);
    setSegmentDataLength(codeSize);
    setSegmentDataLength(16384 - codeSize);

    for (int i = 0; i < codeSize; i++) {
        fread(&opCode, sizeof(uint8_t), 1, file);
        writeByte(i, opCode);
    }

    setRegister(26, 0x00000000);
    setRegister(27, 0x00010000);
    setRegister(3, 0x00000000);

    uint16_t baseCodeSegment, codeSegmentValueLength;
    uint32_t csValue;
    getRegister(26, &csValue);
    getSegmentRange(csValue, &baseCodeSegment, &codeSegmentValueLength);
    
    if (debug) {
        printf("==========================================\n");
        printf("           DISASSEMBLER VMX25            \n");
        printf("==========================================\n");
        printf("Header: VMX25 | Version: %d | Size: %u bytes\n", version, codeSize);
    } else {
        printf("==========================================\n");
        printf("          STARTING EXECUTION             \n");
        printf("==========================================\n");
    }

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress;

    while (IP < baseCodeSegment + codeSegmentValueLength && IP >= baseCodeSegment && IP != 0xFFFFFFFF) {
        logicalAddress = getLogicalAddress(csValue, IP);
        fisicalAddress = getFisicalAddress(logicalAddress);
        // Guardar IP antes de ejecutar la operación
        uint32_t IPBeforeExecution = IP;
        
        if (isValidAddress(fisicalAddress, 1, csValue)) {
            uint8_t Value;
            readByte(fisicalAddress, &Value); //trae el dato del mbr
            opCode = (uint8_t)(Value & 0xFF);

            uint8_t op1Bytes, op2Bytes;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);
            
            // Debug: mostrar informacion de la instruccion
            if (debug) {
                printf("[%04X] %02X", IP, opCode);
                fflush(stdout);
            }
            
            uint8_t cleanOpCode = opCode & 0x1F;
            setRegister(4, cleanOpCode);
            
            IP = IP + 1;  // Avanzamos la instruccion
            
            if (opCodeExists(opCode)){

              uint32_t operandA = 0, operandB = 0;

              if (op2Bytes > 0) {
                uint8_t bytes2[3] = {0};
                int i = 0;

                uint8_t TOPE_IP = IP + op2Bytes;
                while (IP < TOPE_IP) {
                  logicalAddress = getLogicalAddress(csValue, IP);
                  fisicalAddress = getFisicalAddress(logicalAddress);
                  readByte(fisicalAddress, &Value); //trae el dato del mbr
                  opCode = (uint8_t)(Value & 0xFF);
                  bytes2[i] = opCode;
                  if (debug) {
                      printf(" %02X", opCode);
                  }
                  i++;
                  IP += 1;
                }

                if (op2Bytes == 1) {
                  operandB = bytes2[0];
                } else if (op2Bytes == 2) {
                  operandB = ( (uint16_t) (bytes2[0] << 8) ) | bytes2[1];
                } else if (op2Bytes == 3) {
                  operandB = ( (uint32_t) (bytes2[0] << 16) ) | ( (uint16_t) (bytes2[1] << 8) )| bytes2[2];
                }
                operandB = ( (uint32_t) op2Bytes << 24 ) | operandB;   // Asignacion del codigo de operado
                setRegister(6, operandB);
              }  

              
                
              if (op1Bytes > 0) {
                uint8_t bytes1[3] = {0};
                int i = 0;
                uint8_t TOPE_IP = IP + op1Bytes;

                while (IP < TOPE_IP) {
                  logicalAddress = getLogicalAddress(csValue, IP);
                  fisicalAddress = getFisicalAddress(logicalAddress);
                  readByte(fisicalAddress, &Value); //trae el dato del mbr
                  opCode = (uint8_t)(Value & 0xFF);
                  bytes1[i] = opCode;
                  if (debug) {
                      printf(" %02X", opCode);
                  }
                  i++;
                  IP = IP + 1;
                }
                
                if (op1Bytes == 1) {
                  operandA = bytes1[0];
                } else if (op1Bytes == 2) {
                  operandA = ( (uint16_t) (bytes1[0] << 8) ) | bytes1[1];
                } else if (op1Bytes == 3) {
                  operandA = ( (uint32_t) (bytes1[0] << 16) ) | ( (uint16_t) (bytes1[1] << 8) ) | bytes1[2];
                }
                operandA = ( (uint32_t) op1Bytes << 24 ) | operandA;   // Asignacion del codigo de operado
                setRegister(5, operandA);
              }
              
              // Debug: mostrar mnemonico y ejecutar operacion 
              if (debug) {
                  // Calcular el numero total de bytes mostrados
                  int totalBytesShown = 1 + op1Bytes + op2Bytes; // opcode + operandos
                  
                  // Calcular padding para alinear mnemonicos (maximo 8 bytes = 24 chars)
                  int paddingNeeded = 25 - (totalBytesShown * 3); // 3 chars por byte
                  if (paddingNeeded < 0) paddingNeeded = 0;
                  
                  // Agregar padding
                  for (int p = 0; p < paddingNeeded; p++) {
                      printf(" ");
                  }
                  
                  // Mostrar mnemonico
                  const char* mnemonic = getInstructionMnemonic(cleanOpCode, op1Bytes, op2Bytes);
                  printf("| %-4s", mnemonic);

                   if (op1Bytes > 0 && op2Bytes > 0) {
                       printf(" ");
                       getOperandName(operandA);
                       printf(", ");
                       getOperandName(operandB);
                   } else if (op1Bytes > 0) {
                       printf(" ");
                       getOperandName(operandA);
                   }
                  printf("\n");
                  fflush(stdout);
              }

                if (op1Bytes > 0 && op2Bytes > 0) {
                    opTable2[cleanOpCode](operandA, operandB);
                } else if (op1Bytes > 0 && op2Bytes == 0) {
                    opTable1[cleanOpCode](operandA);
                } else{
                  opTable0[cleanOpCode]();
                }
            } else{
              setRegister(3,0xFFFFFFFF);
              printf("\n=========================================\n");
              printf("             INVALID OPCODE           \n");
            
            }

        } else {  // Fallo de segmento
            printf("ERROR: Fallo de segmento - Direccion fisica 0x%08X invalida\n", fisicalAddress);
            setRegister(3, 0xFFFFFFFF); // Terminar ejecucion
        }
        
        // Verificar si se estableció terminación antes de sobrescribir IP
        uint32_t currentIP;
        getRegister(3, &currentIP);
        
        // Solo actualizar IP si no es terminación Y si no cambió durante la ejecución
        if (currentIP != 0xFFFFFFFF) {
            // Si el IP no cambió durante la ejecución, actualizarlo normalmente
            if (currentIP == IPBeforeExecution) {
                setRegister(3, IP);
            }
            // Si cambió (por ejemplo, en un salto), mantener el nuevo valor
        }
        
        // Actualizar IP para la siguiente iteracion
        getRegister(3, &IP);
    }
    

    getRegister(3, &IP);
    if (debug && IP != 0xFFFFFFFF	) {
        printf("=========================================\n");
        printf("           END OF DISASSEMBLER          \n");
        printf("==========================================\n");
    } else {
        if (IP == 0xFFFFFFFF) {
            printf("=========================================\n");
            printf("           EXECUTION TERMINATED           \n");
            printf("==========================================\n");
        } else {
            printf("==========================================\n");
            printf("        EXECUTION TERMINATED - IP          \n");
            printf("          outside code segment             \n");
            printf("==========================================\n");
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("==========================================\n");
        printf("              VMX25 EMULATOR MACHINE              \n");
        printf("==========================================\n");
        printf("Usage: %s <file.vmx> [-d]\n", argv[0]);
        printf("  -d: Debug mode (disassembler)\n");
        return 1;
    }
    
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    int debug = 0;
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        debug = 1;
    }
    
    beginExecution(file, debug);
    
    // Liberar memoria de la memoria
    fclose(file);
    return 0;
}


/*
gcc -o vmx.exe main.c src/components/memory.c src/components/registers.c src/components/segmentTable.c src/functions/directions.c src/functions/operations.c src/functions/noOperatorOperations.c src/functions/oneOperatorsOperations.c src/functions/twoOperatorsOperations.c
*/