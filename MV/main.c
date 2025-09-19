#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "include/operations.h"
#include "include/memory.h"
#include "include/registers.h"
#include "include/segmentTable.h"
#include "include/directions.h"

void beginExecution(FILE *file) {
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
        printf("Error: Archivo no valido (Header: %.5s, Versión: 0x%02X)\n", header, version);
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
    
    printf("Tamaño del codigo: %u bytes\n", codeSize);
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
    
    printf("=== INICIANDO EJECUCION ===\n");

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress;
    while (IP < baseCodeSegment + codeSegmentValueLength && IP >= baseCodeSegment && IP != 0xFFFFFFFF) {
          
        getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
        
        if (isValidAddress(fisicalAddress, 1, csValue)) {
            readByte(fisicalAddress, &opCode);

            uint8_t op1Bytes = 0, op2Bytes = 0;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);
            
            IP = IP + 1;
            uint8_t cleanOpCode = opCode & 0x1F;
            writeRegister(4, cleanOpCode);

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
                i++;
                IP = IP + 1;
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
        
        // Actualizar IP para la siguiente iteración
        getRegister(3, &IP);
    }
    

    getRegister(3, &IP);
    if (IP == 0xFFFFFFFF) {
        printf("=== EJECUCIÓN TERMINADA ===\n");
    } else {
        printf("=== EJECUCIÓN COMPLETADA - IP fuera del segmento de código ===\n");
    }
}

// Función para mostrar el disassembler
void showDisassembler(uint16_t codeSize) {
    printf("=== DISASSEMBLER ===\n");
    
    uint32_t IP;
    writeRegister(26, 0x00000000);
    writeRegister(27, 0x00010000);
    writeRegister(3, 0x00000000);
    getRegister(3, &IP);

    while (IP < codeSize) {
        uint8_t instruction;
        readByte(IP, &instruction);
        
        printf("[%04X] %02X", IP, instruction);
        
        uint8_t op1Bytes = 0, op2Bytes = 0;
        analizeInstruction(instruction, &op1Bytes, &op2Bytes);
        
        uint32_t totalBytes = 1 + op1Bytes + op2Bytes;
        
        // Leer operandos para mostrar
        IP++;
        for (int i = 0; i < op1Bytes + op2Bytes; i++) {
            uint8_t byte;
            readByte(IP + i, &byte);
            printf(" %02X", byte);
        }
        
        // Mostrar mnemónico
        uint8_t opCode = instruction & 0x1F;
        const char* mnemonic = "UNK";
        
        if (instruction & 0x80) {  // 2 operandos
            switch(opCode) {
                case 0x10: mnemonic = "MOV"; break;
                case 0x11: mnemonic = "ADD"; break;
                case 0x12: mnemonic = "SUB"; break;
                case 0x13: mnemonic = "MUL"; break;
                case 0x14: mnemonic = "DIV"; break;
                case 0x15: mnemonic = "CMP"; break;
                case 0x16: mnemonic = "SHL"; break;
                case 0x17: mnemonic = "SHR"; break;
                case 0x18: mnemonic = "SAR"; break;
                case 0x19: mnemonic = "AND"; break;
                case 0x1A: mnemonic = "OR"; break;
                case 0x1B: mnemonic = "XOR"; break;
                case 0x1C: mnemonic = "SWAP"; break;
                case 0x1D: mnemonic = "LDL"; break;
                case 0x1E: mnemonic = "LDH"; break;
                case 0x1F: mnemonic = "RND"; break;
            }
        } else if ((instruction & 0xC0) == 0x00) {  // 1 operando
            switch(opCode) {
                case 0x00: mnemonic = "SYS"; break;
                case 0x01: mnemonic = "JMP"; break;
                case 0x02: mnemonic = "JZ"; break;
                case 0x03: mnemonic = "JP"; break;
                case 0x04: mnemonic = "JN"; break;
                case 0x05: mnemonic = "JNZ"; break;
                case 0x06: mnemonic = "JNP"; break;
                case 0x07: mnemonic = "JNN"; break;
                case 0x08: mnemonic = "NOT"; break;
            }
        } else {  // sin operandos
            if (opCode == 0x0F) mnemonic = "STOP";
        }
        
        printf(" | %s", mnemonic);
        
        // Mostrar operandos (simplificado)
        if (op1Bytes > 0 || op2Bytes > 0) {
            printf(" OP_A, OP_B");
        }
        
        printf("\n");
        IP += totalBytes;
    }
    printf("===================\n");
}

void showDisassemblerFromFile(FILE *file) {
    
    uint8_t opCode;
    char header[6] = {0};
    uint8_t version;
    uint16_t codeSize;
    
    if (fread(header, sizeof(uint8_t), 5, file) != 5 ||
        fread(&version, sizeof(uint8_t), 1, file) != 1) {
        printf("Error: No se pudo leer el header del archivo\n");
        return;
    }

    if (strncmp(header, "VMX25", 5) != 0 || version != 0x01) {
        printf("Error: Archivo no valido (Header: %.5s, Version: 0x%02X)\n", header, version);
        return;
    }

    uint8_t sizeHigh2, sizeLow2;
    fread(&sizeHigh2, sizeof(uint8_t), 1, file);
    fread(&sizeLow2, sizeof(uint8_t), 1, file);
    codeSize = (sizeHigh2 << 8) | sizeLow2;  // Big-endian
    
    printf("=== DISASSEMBLER VMX25 ===\n");
    printf("Header: %.5s, Version: %d, Size: %u bytes\n", header, version, codeSize);
    
    initMemory();
    initRegisters();
    initSegmentTable();
    initOpTable();
    setSegmentDataLength(codeSize);
    setSegmentDataLength(16384 - codeSize);
    
    for (int i = 0; i < codeSize; i++) {
        if (fread(&opCode, sizeof(uint8_t), 1, file) != 1) {
            printf("Error: Codigo incompleto\n");
            return;
        }
        writeByte(i, opCode);
    }
    
    showDisassembler(codeSize);
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
    
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        showDisassemblerFromFile(file);
    } else {
        beginExecution(file);
    }
    
    fclose(file);
    return 0;
}
