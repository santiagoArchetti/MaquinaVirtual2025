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
    char header[6] = {0};

    // Leer header "VMX25" (5 bytes)
    if (fread(header, sizeof(uint8_t), 5, file) != 5) {
        printf("Error: No se pudo leer el header del archivo\n");
        return;
    }
    
    // Leer versión (1 byte)
    uint8_t version;
    if (fread(&version, sizeof(uint8_t), 1, file) != 1) {
        printf("Error: No se pudo leer la versión del archivo\n");
        return;
    }

    // Verificar header y versión
    if (strncmp(header, "VMX25", 5) != 0 || version != 0x01) {
        printf("Error: Archivo no es un VMX25 válido (Header: %.5s, Versión: 0x%02X)\n", header, version);
        return;
    }

    printf("=== INICIANDO MÁQUINA VIRTUAL VMX25 ===\n");
    printf("Header: %.5s, Versión: %d\n", header, version);

    // Inicializar componentes
    initMemory();
    initRegisters();
    initSegmentTable();
    initOpTable();  // ¡CRÍTICO! Inicializar tabla de operaciones

    // Leer tamaño del código (2 bytes, little-endian)
    uint16_t codeSize;
    if (fread(&codeSize, sizeof(uint16_t), 1, file) != 1) {
        printf("Error: No se pudo leer el tamaño del código\n");
        return;
    }
    
    printf("Tamaño del código: %u bytes\n", codeSize);
    
    // Validar tamaño del código
    if (codeSize > 16384) {
        printf("Error: Tamaño del código excede la memoria disponible\n");
        return;
    }
    
    // Configurar segmentos según la consigna
    setSegmentDataLength(codeSize);         // Segmento 0: código
    setSegmentDataLength(16384 - codeSize); // Segmento 1: datos

    // Cargar código en memoria (segmento de código)
    printf("Cargando código en memoria...\n");
    for (int i = 0; i < codeSize; i++) {
        if (fread(&opCode, sizeof(uint8_t), 1, file) != 1) {
            printf("Error: Código incompleto en el archivo\n");
            return;
        }
        writeByte(i, opCode);
    }

    // Inicializar registros según la consigna
    writeRegister(26, 0x00000000);  // CS = 0 (segmento 0 = código)
    writeRegister(27, 0x00010000);  // DS = 1 (segmento 1 = datos)  
    writeRegister(3, 0x00000000);   // IP = 0 (primera instrucción)
    
    printf("Registros inicializados: CS=0x%08X, DS=0x%08X, IP=0x%08X\n", 
           0x00000000, 0x00010000, 0x00000000);

    // Obtener información del segmento de código
    uint16_t baseCodeSegment;
    uint16_t codeSegmentValueLength;
    uint32_t csValue;
    getRegister(26, &csValue);
    getSegmentRange(csValue, &baseCodeSegment, &codeSegmentValueLength);
    
    printf("Segmento de código: Base=0x%04X, Tamaño=%u\n", baseCodeSegment, codeSegmentValueLength);
    printf("=== INICIANDO EJECUCION ===\n");

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress;

    // Ciclo principal de ejecución
    while (IP < baseCodeSegment + codeSegmentValueLength && IP >= baseCodeSegment && IP != 0xFFFFFFFF) {
          
        getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
        
        if (isValidAddress(fisicalAddress, 1, csValue)) {  //validamos si la direccion fisica es valida (1 byte)

            readByte(fisicalAddress, &opCode); //leemos la posicion fisica de la memoria, este devuelve el opCode

            uint8_t op1Bytes = 0, op2Bytes = 0;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);
            
            IP = IP + 1; // opCode leido (+1 byte)

            uint8_t cleanOpCode = opCode & 0x1F;  // variable usada por claridad
            
            writeRegister(4, cleanOpCode); //registro de la operacion actual (OPC)

            uint32_t operandA = 0, operandB = 0;
            
            // Leer operandos en orden inverso: primero OP_B (fuente), luego OP_A (destino)
            // Leer segundo operando (OP_B) - exactamente la cantidad de bytes que necesita
            if (op2Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;
              uint8_t TOPE_IP = IP + op2Bytes;
              while (IP < TOPE_IP) {
                getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                uint8_t byteValue;
                readByte(fisicalAddress, &byteValue); //Lee de la memoria apuntada por IP
                bytes[i] = byteValue;
                i++;
                IP = IP + 1;
              }
              
              // Combinar bytes según el tamaño real leído
              if (op2Bytes == 1) {
                operandB = bytes[0];
                writeRegister(6, operandB); //registro de el segundo operando (OP2)
              } else if (op2Bytes == 2) {
                operandB = (bytes[0] << 8) | bytes[1];
                writeRegister(6, operandB); //registro de el segundo operando (OP2)
              } else if (op2Bytes == 3) {
                operandB = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(6, operandB); //registro de el segundo operando (OP2)
              }
            }
            
            // Leer primer operando (OP_A) - exactamente la cantidad de bytes que necesita
            if (op1Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;
              uint8_t TOPE_IP = IP + op1Bytes;

              while (IP < TOPE_IP) {
                getMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                uint8_t byteValue;
                readByte(fisicalAddress, &byteValue); //Lee de la memoria apuntada por IP
                bytes[i] = byteValue;
                i++;
                IP = IP + 1;
              }
              
              // Combinar bytes según el tamaño real leído
              if (op1Bytes == 1) {
                operandA = bytes[0];
                writeRegister(5, operandA); //registro de el primer operando (OP1)
              } else if (op1Bytes == 2) {
                operandA = (bytes[0] << 8) | bytes[1];
                writeRegister(5, operandA); //registro de el primer operando (OP1)
              } else if (op1Bytes == 3) {
                operandA = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(5, operandA); //registro de el primer operando (OP1)
              }
            }
            
            // Ejecutar la operación según el número de operandos
            if (op1Bytes > 0 && op2Bytes > 0) {
              // Dos operandos: operandA (destino), operandB (fuente)
              if (opTable2[cleanOpCode] != NULL) {
                opTable2[cleanOpCode](operandA, operandB);
              } else {
                printf("ERROR: Instrucción inválida con 2 operandos: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            } else if (op1Bytes > 0 && op2Bytes == 0) {
              // Un operando
              if (opTable1[cleanOpCode] != NULL) {
                opTable1[cleanOpCode](operandA);
              } else {
                printf("ERROR: Instrucción inválida con 1 operando: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            } else if (op1Bytes == 0 && op2Bytes == 0) {
              // Sin operandos
              if (opTable0[cleanOpCode] != NULL) {
                opTable0[cleanOpCode]();
              } else {
                printf("ERROR: Instrucción inválida sin operandos: 0x%02X\n", cleanOpCode);
                writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
              }
            }
        } else {
            printf("ERROR: Fallo de segmento - Dirección física 0x%08X inválida\n", fisicalAddress);
            writeRegister(3, 0xFFFFFFFF); // Terminar ejecución
        }
        
        // Actualizar IP para la siguiente iteración
        getRegister(3, &IP);
    }
    
    // Mostrar estado final
    uint32_t finalIP;
    getRegister(3, &finalIP);
    if (finalIP == 0xFFFFFFFF) {
        printf("=== EJECUCIÓN TERMINADA ===\n");
    } else {
        printf("=== EJECUCIÓN COMPLETADA - IP fuera del segmento de código ===\n");
    }
}

// Función para mostrar el disassembler
void showDisassembler(uint16_t codeSize) {
    printf("=== DISASSEMBLER ===\n");
    
    uint32_t IP = 0;
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

// Función principal con ejemplos de uso
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
    
    // Verificar flag -d
    int disassembler = 0;
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        disassembler = 1;
    }
    
    if (disassembler) {
        // Solo mostrar disassembler
        uint8_t opCode;
        char header[6] = {0};
        
        // Leer header "VMX25" (5 bytes)
        if (fread(header, sizeof(uint8_t), 5, file) != 5) {
            printf("Error: No se pudo leer el header del archivo\n");
            fclose(file);
            return 1;
        }
        
        // Leer versión (1 byte)
        uint8_t version;
        if (fread(&version, sizeof(uint8_t), 1, file) != 1) {
            printf("Error: No se pudo leer la versión del archivo\n");
            fclose(file);
            return 1;
        }
        
        // Verificar header y versión
        if (strncmp(header, "VMX25", 5) == 0 && version == 0x01) {
            // Leer tamaño del código
            uint16_t codeSize;
            if (fread(&codeSize, sizeof(uint16_t), 1, file) != 1) {
                printf("Error: No se pudo leer el tamaño del código\n");
                fclose(file);
                return 1;
            }
            
            printf("=== DISASSEMBLER VMX25 ===\n");
            printf("Header: %.5s, Versión: %d, Tamaño: %u bytes\n", header, version, codeSize);
            
            // Inicializar componentes necesarios para el disassembler
            initMemory();
            initRegisters();
            initSegmentTable();
            initOpTable();  // ¡CRÍTICO! También para el disassembler
            
            setSegmentDataLength(codeSize);
            setSegmentDataLength(16384 - codeSize);
            
            // Cargar código en memoria
            for (int i = 0; i < codeSize; i++) {
                if (fread(&opCode, sizeof(uint8_t), 1, file) != 1) {
                    printf("Error: Código incompleto en el archivo\n");
                    fclose(file);
                    return 1;
                }
                writeByte(i, opCode);
            }
            
            showDisassembler(codeSize);
        } else {
            printf("Error: Archivo no es un VMX25 válido (Header: %.5s, Versión: 0x%02X)\n", header, version);
        }
    } else {
        beginExecution(file);
    }
    
    fclose(file);
    return 0;
}
