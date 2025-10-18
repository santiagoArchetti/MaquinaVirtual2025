#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "include/operations.h"
#include "include/memory.h"
#include "include/registers.h"
#include "include/segmentTable.h"
#include "include/directions.h"

void beginExecution(FILE *filei, int debug) {

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress, csValue;
    uint16_t baseCodeSegment, codeSegmentValueLength;
    getRegister(26,&csValue);
    getSegmentRange((csValue >> 16), &baseCodeSegment, &codeSegmentValueLength);
    uint8_t opCode;

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
            
            setRegister(3,IP + op1Bytes + op2Bytes + 1);
            IP += 1;
            
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
                int ii = 0;
                uint8_t TOPE_IP1 = IP + op1Bytes;

                while (IP < TOPE_IP1) {
                  logicalAddress = getLogicalAddress(csValue, IP);
                  fisicalAddress = getFisicalAddress(logicalAddress);
                  readByte(fisicalAddress, &Value); //trae el dato del mbr
                  opCode = (uint8_t)(Value & 0xFF);
                  bytes1[ii] = opCode;
                  if (debug) {
                      printf(" %02X", opCode);
                  }
                  ii++;
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
                if (flag){
                    opTable1[0x00](0xFF);
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

        // Actualizar IP para la siguiente iteracion
        getRegister(3, &IP);
    }

    getRegister(3, &IP);
    if (debug && IP != 0xFFFFFFFF) {
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

void analizeHeader(FILE *file, int debug,int gotParams, , uint32_t offsetPosition, int argc) {
    uint8_t opCode;
    char header[5] = {0};
    uint8_t version;
    uint16_t codeSize;

    if (fread(header, sizeof(uint8_t), 5, file) != 5 || fread(&version, sizeof(uint8_t), 1, file) != 1) {
        printf("Error: No se pudo leer el header del archivo\n");
        return;
    }

    if ( strncmp(header, "VMX25", 5) != 0 || (version != 0x01 && version != 0x02) ) {
        printf("Error: File not valid (Header: %.5s, Version: 0x%02X)\n", header, version);
        return;
    }

    if (version == 0x01) {
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
        
    } else
        if (version == 0x02) {
            uint8_t sizeHigh, sizeLow;
            uint32_t registerValue;
            uint32_t vec[5];

            for (int ii = 0; ii < 6; ii++) { 
                fread(&sizeHigh, sizeof(uint8_t), 1, file);
                fread(&sizeLow, sizeof(uint8_t), 1, file);
                if ( ii < 5)
                    vec[ii + 1 - gotParams] = (uint32_t) (( ((uint32_t) ii) << 16 ) | (0x0));
                else
                    vec[0 + gotParams] = (uint32_t) (( ((uint32_t) ii) << 16 ) | (0x0));
            }
            int emptySeg = 0;
            for (int ii = 0; ii < 5; ii++){
                if (vec[ii] != 0) {
                    setSegmentDataLength(vec[ii]);
                    registerValue = (uint32_t) (( ((uint32_t) ii - emptySeg) << 16 ) | (0x0));
                }else{
                    registerValue = 0xFFFFFFFF;
                    emptySeg++;            
                }
                switch (ii) {
                    case 1:setRegister(30,registerValue);
                            break; 
                    case 2:setRegister(26,registerValue);
                            break;
                    case 3:setRegister(27,registerValue);
                            break;
                    case 4:setRegister(28,registerValue);
                            break;
                    case 5:setRegister(29,registerValue);
                            break;
                }
            }
            uint32_t entryPoint;
            fread(&sizeHigh, sizeof(uint8_t), 1, file);
            fread(&sizeLow, sizeof(uint8_t), 1, file);
            entryPoint = (sizeHigh << 8) | sizeLow;  // Big-endian

            uint32_t direccion_logica;
            getRegister(26,&direccion_logica);
            setRegister(3, (direccion_logica & 0xFFFFFFFF00000000 | entryPoint));
            uint16_t base, tam;
            getSegmentRange((direccion_logica >> 16), &base, &tam);
            for (int i = 0; i < tam; i++) {
                fread(&opCode, sizeof(uint8_t), 1, file);
                writeByte(base + i, opCode);
            }
            uint32_t KS;
            getRegister(30,&KS);
            if (KS != -1){
                getSegmentRange((KS >> 16), &base, &tam);
                for (int i = 0; i < tam; i++) {
                    fread(&opCode, sizeof(uint8_t), 1, file);
                    writeByte(base + i, opCode);
                }
            }
            opTable1[0x0B](offsetPosition);
            opTable1[0x0B](argc);
            opTable1[0x0B](0XFFFFFFFF);
        }
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
    beginExecution(file, debug);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("==========================================\n");
        printf("              VMX25 EMULATOR MACHINE              \n");
        printf("==========================================\n");
        printf("Usage: %s <file.vmx> [options]\n", argv[0]);
        printf("Options:\n");
        printf("  -d              Debug mode\n");
        printf("  -m<size>        Memory size (default: 16384)\n");
        printf("  <file.i>        Input file\n");
        printf("  -P <args...>    Parameters for program\n");
        return 1;
    }
    
    printf("==========================================\n");
    printf("              VMX25 EMULATOR MACHINE              \n");
    printf("==========================================\n");
    
    int i = 1;
    int len;
    FILE *fileA = NULL;
    FILE *fileB = NULL;
    int memorySize = 16384;
    int debug = 0;
    int gotVmx = 0;
    int gotParams = 0;
    
    // Variables para parámetros
    char **lista = NULL;
    uint32_t *offsets = NULL;
    int j = 0;
    uint32_t offsetAcum = 0, offsetPostion =0xFFFFFFFF;

    // Parseo de argumentos
    while (i < argc) {
        len = strlen(argv[i]);
        
        // Buscar archivo .vmx
        if (len > 4 && strcmp(&argv[i][len-4], ".vmx") == 0) {
            fileA = fopen(argv[i], "rb");
            if (!fileA) {
                printf("Error: Cannot open file '%s'\n", argv[i]);
                return 1;
            }
            gotVmx = 1;
        }
        // Buscar archivo .i (input)
        else if (len > 2 && strcmp(&argv[i][len-2], ".i") == 0) {
            fileB = fopen(argv[i], "rb");
            if (!fileB) {
                printf("Error: Cannot open file '%s'\n", argv[i]);
                return 1;
            }
        }
        // Opción de memoria -m<numero>
        else if (len > 2 && argv[i][0] == '-' && argv[i][1] == 'm') {
            char *fin;
            long numero = strtol(&argv[i][2], &fin, 10);
            if (*fin == '\0' && numero > 0) {
                memorySize = (int)numero;
                printf("Memory size set to: %d bytes\n", memorySize);
            } else {
                printf("Error: Invalid memory size in '%s'\n", argv[i]);
                return 1;
            }
        }
        // Opción de debug
        else if (strcmp(argv[i], "-d") == 0) {
            debug = 1;
        }
        // Opción de parámetros
        else if (strcmp(argv[i], "-P") == 0) {
            if (!gotVmx) {
                printf("Error: VMX file must be specified before -P option\n");
                return 1;
            }
            gotParams = 1;
            
            // Reservar memoria para los parámetros
            lista = malloc(50 * sizeof(char*));
            offsets = malloc(50 * sizeof(uint32_t));
            
            if (lista == NULL || offsets == NULL) {
                printf("Error: Cannot allocate memory for parameters\n");
                return 1;
            }
            
            i++; // Avanzar al primer parámetro
            
            // Capturar todos los argumentos restantes como parámetros
            while (i < argc && j < 50) {
                lista[j] = malloc(strlen(argv[i]) + 1);
                if (lista[j] == NULL) {
                    printf("Error: Cannot allocate memory for parameter %d\n", j);
                    // Liberar memoria ya reservada
                    for (int k = 0; k < j; k++) {
                        free(lista[k]);
                    }
                    free(lista);
                    free(offsets);
                    return 1;
                }
                strcpy(lista[j], argv[i]);
                offsets[j] = offsetAcum;
                offsetAcum += strlen(lista[j]) + 1; // +1 para el null terminator
                j++;
                i++;
            }
            break; // Salir del while principal
        }
        i++;
    }

    // Verificar que se haya especificado un archivo VMX
    if (!fileA) {
        printf("Error: No VMX file specified\n");
        return 1;
    }

    // Inicializar componentes con tamaño de memoria dinámico
    setRegister(7,memorySize);
    initMemory(memorySize);
    initRegisters();
    initSegmentTable();
    initOpTable();

       if (gotParams && lista != NULL && offsets != NULL) {
        uint32_t direccion_fisica = 0x0;
               
        // Escribir los strings en memoria
        for (int idx = 0; idx < j; idx++) {
            for (int k = 0; k <= strlen(lista[idx]); k++) { // Incluye null terminator
                writeByte(direccion_fisica, lista[idx][k]);
                direccion_fisica++;
            }
        }
        offsetsPosition = direccion_fisica;         
        setRegister(31,0x00000000);

        // Escribir los offsets en memoria
        for (int idx = 0; idx < j; idx++) {
            for (int k = 0; k < 4; k++) {
                uint8_t byte = (offsets[idx] >> ((3 - k) * 8)) & 0xFF;
                writeByte(direccion_fisica, byte);
                direccion_fisica++;
            }
        }
        
        // Liberar memoria de parámetros
        for (int k = 0; k < j; k++) {
            free(lista[k]);
        }

        setSegmentDataLength(direccion_fisica);
        free(lista);
        free(offsets);
    }else{
        setRegister(31,0xFFFFFFFF);
    }

    if (fileA != NULL) {
        analizeHeader(fileA, debug, gotParams,offsetPostion, j);
    }
    // Liberar recursos
    if (fileA) {
        fclose(fileA);
    }
    if (fileB) {
        fclose(fileB);
    }
    
    // Liberar memoria dinámica
    freeMemory();
    
    return 0;
}


/*
gcc -o vmx.exe main.c src/components/memory.c src/components/registers.c src/components/segmentTable.c src/functions/directions.c src/functions/operations.c src/functions/noOperatorOperations.c src/functions/oneOperatorsOperations.c src/functions/twoOperatorsOperations.c
*/