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

    for (int i = 0; i < 5; i++) {
      fread(&header[i], sizeof(uint8_t), 1, file);
    }
    uint8_t version;
    fread(&version, sizeof(uint8_t), 1, file);

    if (strncmp(header, "VMX25", 5) == 0 && version == 0x01) {

        // Inicializar componentes
        initMemory();
        initRegisters();
        initSegmentTable();

        uint16_t codeSize;
        fread(&codeSize, sizeof(uint16_t), 1, file);   
        
          
        // Configurar segmentos
        setSegmentDataLength(codeSize); // Segmento código
        setSegmentDataLength(16384-codeSize); // Segmento datos

        int i = 0;
        while (fread(&opCode, sizeof(uint8_t), 1, file) == 1) {  //leemos el codigo y lo escribimos en memoria
          writeByte(i, opCode);
          i++;
        }

        // Inicializar registros según especificaciones
        writeRegister(26, 0x00000000);  // CS = segmento código
        writeRegister(27, 0x00010000);  // DS = segmento datos  
        writeRegister(3, 0x00000000);   // IP = primera instrucción (apunta al CS)

        //registro almaceno la ubicacion en el segmento y el offsets
        uint16_t baseCodeSegment;
        uint16_t codeSegmentValueLength;
        uint32_t csValue;
        getRegister(26, &csValue);

        getSegmentRange(csValue, &baseCodeSegment, &codeSegmentValueLength);

        uint32_t IP;
        getRegister(3, &IP); // Leer IP del registro 3

        uint32_t logicalAddress;  //con estas direcciones se accede a la memoria
        uint32_t fisicalAddress;

        while (IP < baseCodeSegment + codeSegmentValueLength && IP >= baseCodeSegment) {
          
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
                printf("Instrucción inválida: 0x%02X\n", cleanOpCode);
              }
            } else if (op1Bytes > 0 && op2Bytes == 0) {
              // Un operando
              if (opTable1[cleanOpCode] != NULL) {
                opTable1[cleanOpCode](operandA);
              } else {
                printf("Instrucción inválida: 0x%02X\n", cleanOpCode);
              }
            } else if (op1Bytes == 0 && op2Bytes == 0) {
              // Sin operandos
              if (opTable0[cleanOpCode] != NULL) {
                opTable0[cleanOpCode]();
              } else {
                printf("Instrucción inválida: 0x%02X\n", cleanOpCode);
              }
            }
          }
          else {
            writeRegister(3, 0xFFFFFFFF);
          }
        } 
    }
    else {
        printf("File is not a valid MVX25 file\n");
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
        
        for (int i = 0; i < 5; i++) {
            fread(&header[i], sizeof(uint8_t), 1, file);
        }
        uint8_t version;
        fread(&version, sizeof(uint8_t), 1, file);
        
        if (strncmp(header, "VMX25", 5) == 0 && version == 0x01) {
            uint16_t codeSize;
            fread(&codeSize, sizeof(uint16_t), 1, file);
            
            // Inicializar memoria y cargar código
            initMemory();
            initRegisters();
            initSegmentTable();
            
            setSegmentDataLength(codeSize);
            setSegmentDataLength(16384-codeSize);
            
            int i = 0;
            while (fread(&opCode, sizeof(uint8_t), 1, file) == 1) {
                writeByte(i, opCode);
                i++;
            }
            
            showDisassembler(codeSize);
        } else {
            printf("File is not a valid MVX25 file\n");
        }
    } else {
        beginExecution(file);
    }
    
    fclose(file);
    return 0;
}