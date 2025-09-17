#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "include/operations.h"
#include "src/functions/operations.c"
#include "src/functions/directions.c"
#include "src/components/memory.c"


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
        fread(&codeSize, sizeof(uivcc vbnt16_t), 1, file);   
        
          
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

        //registro almaceno la ubicacion en el segento y el offsets
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
          
          setMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
          
          if (isValidAddress(fisicalAddress, IP, csValue)) {  //validamos si la direccion fisica es valida

            readByte(fisicalAddress, &opCode); //leemos la posicion fisica de la memoria, este devuelve el opCode

            uint8_t op1Bytes = 0, op2Bytes = 0;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);
            
            IP = IP + 1; // opCode leido (+1 byte)

            uint8_t cleanOpCode = opCode & 0x1F;  // variable usada por claridad
            
            writeRegister(5, (cleanOpCode) && 0X001F); //registro de la operacion actual

            uint32_t operand1 = 0, operand2 = 0;
            
            // Leer primer operando - exactamente la cantidad de bytes que necesita
            if (op1Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;
              uint8_t TOPE_IP = IP + op1Bytes;

              while (IP < TOPE_IP) {
                setMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                opCode = readByte(fisicalAddress, &opCode); //Lee de la memoria apuntada por IP
                bytes[i] = opCode;
                i++;
                IP = IP + 1;
              }
              
              // Combinar bytes según el tamaño real leído
              if (op1Bytes == 1) {
                operand1 = bytes[0];
                writeRegister(6, (operand1) && 0X100F); //registro de el primer operando
              } else if (op1Bytes == 2) {
                operand1 = (bytes[0] << 8) | bytes[1];
                writeRegister(6, (operand1) && 0X20FF); //registro de el primer operando
              } else if (op1Bytes == 3) {
                operand1 = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(6, (operand1) && 0X3FFF); //registro de el primer operando
              }

            }
            
            // Leer segundo operando - exactamente la cantidad de bytes que necesita
            if (op2Bytes > 0) {
              uint8_t bytes[3] = {0};
              int i = 0;
              uint8_t TOPE_IP = IP + op2Bytes;
              while (IP < TOPE_IP) {
                setMemoryAccess(csValue, IP, &logicalAddress, &fisicalAddress, &opCode);
                opCode = readByte(fisicalAddress, &opCode); //Lee de la memoria apuntada por IP
                bytes[i] = opCode;
                i++;
                IP = IP + 1;
              }
              
              // Combinar bytes según el tamaño real leído
              if (op2Bytes == 1) {
                operand2 = bytes[0];
                writeRegister(7, (operand2) && 0X100F); //registro de el segundo operando
              } else if (op2Bytes == 2) {
                operand2 = (bytes[0] << 8) | bytes[1];
                writeRegister(7, (operand2) && 0X20FF); //registro de el segundo operando
              } else if (op2Bytes == 3) {
                operand2 = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
                writeRegister(7, (operand2) && 0X3FFF); //registro de el segundo operando
              }

            }
            
            // Ejecutar la operación según el número de operandos
            if (op1Bytes > 0 && op2Bytes > 0) {
              // Dos operandos
              if (opTable2[cleanOpCode] != NULL) {
                opTable2[cleanOpCode](operand2, operand1,op1Bytes,op2Bytes);
              } else {
                printf("Instrucción inválida: 0x%02X\n", cleanOpCode);
              }
            } else if (op1Bytes > 0 && op2Bytes == 0) {
              // Un operando
              if (opTable1[cleanOpCode] != NULL) {
                opTable1[cleanOpCode](operand1,op1Bytes);
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
            setRegister(3, 0xFFFFFFFF);
          }
        } 
     }
    }
    else {
        printf("File is not a valid MVX25 file\n");
    }
}




// Función principal con ejemplos de uso
int main() {
    FILE *file = fopen("program.asm", "rb");
    if (!file) {
        printf("Error al abrir el archivo\n");
        return 1;
    }
    else {
        beginExecution(file);
    }
    fclose(file);

    return 0;
    
}