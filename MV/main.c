#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "include/operations.h"
#include "include/memory.h"
#include "include/registers.h"
#include "include/segmentTable.h"
#include "include/directions.h"

// Función para obtener el mnemónico de una instrucción
const char* getInstructionMnemonic(uint8_t opCode, uint8_t op1Bytes, uint8_t op2Bytes) {
    if (op1Bytes > 0 && op2Bytes > 0) {
        // Operaciones con 2 operandos
        switch(opCode) {
            case 0x10: return "MOV OP_A, OP_B";
            case 0x11: return "ADD OP_A, OP_B";
            case 0x12: return "SUB OP_A, OP_B";
            case 0x13: return "MUL OP_A, OP_B";
            case 0x14: return "DIV OP_A, OP_B";
            case 0x15: return "CMP OP_A, OP_B";
            case 0x16: return "SHL OP_A, OP_B";
            case 0x17: return "SHR OP_A, OP_B";
            case 0x18: return "SAR OP_A, OP_B";
            case 0x19: return "AND OP_A, OP_B";
            case 0x1A: return "OR OP_A, OP_B";
            case 0x1B: return "XOR OP_A, OP_B";
            case 0x1C: return "SWAP OP_A, OP_B";
            case 0x1D: return "LDL OP_A, OP_B";
            case 0x1E: return "LDH OP_A, OP_B";
            case 0x1F: return "RND OP_A, OP_B";
            default: return "UNK OP_A, OP_B";
        }
    } else if (op1Bytes > 0 && op2Bytes == 0) {
        // Operaciones con 1 operando
        switch(opCode) {
            case 0x00: return "SYS OP_A";
            case 0x01: return "JMP OP_A";
            case 0x02: return "JZ OP_A";
            case 0x03: return "JP OP_A";
            case 0x04: return "JN OP_A";
            case 0x05: return "JNZ OP_A";
            case 0x06: return "JNP OP_A";
            case 0x07: return "JNN OP_A";
            case 0x08: return "NOT OP_A";
            default: return "UNK OP_A";
        }
    } else {
        // Operaciones sin operandos
        switch(opCode) {
            case 0x0F: return "STOP";
            default: return "UNK";
        }
    }
}

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

    writeRegister(26, 0x00000000);
    writeRegister(27, 0x00010000);
    writeRegister(3, 0x00000000);

    uint16_t baseCodeSegment, codeSegmentValueLength;
    uint32_t csValue;
    getRegister(26, &csValue);
    getSegmentRange(csValue, &baseCodeSegment, &codeSegmentValueLength);
    
    if (debug) {
        printf("=== DISASSEMBLER VMX25 ===\n");
        printf("Header: VMX25, Version: %d, Size: %u bytes\n", version, codeSize);
    } else {
        printf("=== STARTING EXECUTION ===\n");
    }

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress;

    while (IP < baseCodeSegment + codeSegmentValueLength && IP >= baseCodeSegment && IP != 0xFFFFFFFF) {
          
        getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
        
        if (isValidAddress(fisicalAddress, 1, csValue)) {
            readByte(fisicalAddress, &opCode);

            uint8_t op1Bytes = 0, op2Bytes = 0;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);
            
            // Debug: mostrar información de la instrucción
            if (debug) {
                printf("[%04X] %02X", IP, opCode);
                fflush(stdout);
            }
            
            uint8_t cleanOpCode = opCode & 0x1F;
            writeRegister(4, cleanOpCode);
            
            IP = IP + 1;

            uint32_t operandA = 0, operandB = 0;

            if (op2Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;

              uint8_t TOPE_IP = IP + op2Bytes;
              while (IP < TOPE_IP) {
                getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                uint8_t byteValue;
                readByte(fisicalAddress, &byteValue);
                bytes[i] = byteValue;
                if (debug) {
                    printf(" %02X", byteValue);
                }             
                i++;
                IP += 1;
              }
              
              if (op2Bytes == 1) {
                operandB = bytes[0];
                writeRegister(6, operandB);
              } else if (op2Bytes == 2) {
                operandB = (bytes[0] << 8) | bytes[1];
                writeRegister(6, operandB);
              } else if (op2Bytes == 3) {
                operandB = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(6, operandB);
              }
            }  

            
              
            if (op1Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;
              uint8_t TOPE_IP = IP + op1Bytes;

              while (IP < TOPE_IP) {
                getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                uint8_t byteValue;
                readByte(fisicalAddress, &byteValue);
                bytes[i] = byteValue;
                if (debug) {
                    printf(" %02X", byteValue);
                }
                i++;
                IP = IP + 1;
              }
              
              if (op1Bytes == 1) {
                operandA = bytes[0];
                writeRegister(5, operandA);
              } else if (op1Bytes == 2) {
                operandA = (bytes[0] << 8) | bytes[1];
                writeRegister(5, operandA);
              } else if (op1Bytes == 3) {
                operandA = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(5, operandA);
              }
            }
            
            // Debug: mostrar mnemónico y ejecutar operación
            if (debug) {
                // Mostrar mnemónico
                const char* mnemonic = getInstructionMnemonic(cleanOpCode, op1Bytes, op2Bytes);
                printf(" | %s", mnemonic);
                if (op1Bytes > 0 && op2Bytes > 0) {
                    printf(" OP_A=0x%08X, OP_B=0x%08X", operandA, operandB);
                } else if (op1Bytes > 0) {
                    printf(" OP_A=0x%08X", operandA);
                }
                printf("\n");
                fflush(stdout);
            }
            
            if (op1Bytes > 0 && op2Bytes > 0) {
              if (opTable2[cleanOpCode] != NULL) {  // Dos operandos: operandA (destino), operandB (fuente)
                opTable2[cleanOpCode](operandA, operandB);
              } else {
                printf("ERROR: Instrucción inválida con 2 operandos: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            } else if (op1Bytes > 0 && op2Bytes == 0) {
              if (opTable1[cleanOpCode] != NULL) {  // Un operando
                opTable1[cleanOpCode](operandA);
              } else {
                printf("ERROR: Instrucción inválida con 1 operando: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            } else if (op1Bytes == 0 && op2Bytes == 0) {
              if (opTable0[cleanOpCode] != NULL) {  // Sin operandos
                opTable0[cleanOpCode]();
              } else {
                printf("ERROR: Instrucción inválida sin operandos: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            }
        } else {  // Fallo de segmento
            printf("ERROR: Fallo de segmento - Dirección física 0x%08X inválida\n", fisicalAddress);
            writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
        }
        writeRegister(3, IP);

        // Actualizar IP para la siguiente iteración
        getRegister(3, &IP);
        if (debug) {
            printf("IP: %08X\n", IP);
        }
    }
    

    getRegister(3, &IP);
    if (debug) {
        printf("===================\n");
    } else {
        if (IP == 0xFFFFFFFF) {
            printf("=== EJECUCIÓN TERMINADA ===\n");
        } else {
            printf("=== EJECUCIÓN COMPLETADA - IP fuera del segmento de código ===\n");
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo.vmx> [-d]\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        printf("Error al abrir el archivo %s\n", argv[1]);
        return 1;
    }
    
    int debug = 0;
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        debug = 1;
    }
    
    beginExecution(file, debug);
    
    fclose(file);
    return 0;
}
