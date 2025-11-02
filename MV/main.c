#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "include/operations.h"
#include "include/memory.h"
#include "include/registers.h"
#include "include/segmentTable.h"
#include "include/directions.h"
#include "include/oneOperatorsOperations.h"

void beginExecution(FILE *filei, int debug) {

    uint32_t IP;
    getRegister(3, &IP);
    uint32_t logicalAddress, fisicalAddress, csValue;
    uint16_t baseCodeSegment, codeSegmentValueLength;
    getRegister(26,&csValue);
    getSegmentRange((csValue >> 16), &baseCodeSegment, &codeSegmentValueLength);
    uint8_t opCode;

    while ((baseCodeSegment + (IP & 0xFFFF) < baseCodeSegment + codeSegmentValueLength) && (baseCodeSegment + (IP & 0xFFFF) >= baseCodeSegment)) {
        fisicalAddress = getFisicalAddress(IP);
        // Guardar IP antes de ejecutar la operación
        uint32_t IPBeforeExecution = IP;
        
        if (isValidAddress(fisicalAddress, 1, (uint16_t) (csValue >> 16))) {
            uint8_t Value;
            readByte(fisicalAddress, &Value); //trae el dato del mbr
            opCode = (uint8_t)(Value & 0xFF);

            uint8_t op1Bytes, op2Bytes;
            analizeInstruction(opCode, &op1Bytes, &op2Bytes);

            // Debug: mostrar informacion de la instruccion
            if (debug) {
                printf("[%04X] %02X", fisicalAddress, opCode);
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

                    uint32_t TOPE_IP = IP + op2Bytes;
                    while (IP < TOPE_IP) {
                    fisicalAddress = getFisicalAddress(IP);
                    readByte(fisicalAddress, &Value); //trae el dato del mbr
                    bytes2[i] = Value;
                    if (debug) {
                        printf(" %02X", Value);
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
                    uint32_t TOPE_IP1 = IP + op1Bytes;
                    while (IP < TOPE_IP1) {
                        fisicalAddress = getFisicalAddress(IP);
                        readByte(fisicalAddress, &Value); //trae el dato del mbr
                        bytes1[ii] = Value;
                        if (debug) {
                            printf(" %02X", Value);
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
                    int paddingNeeded = 23 - (totalBytesShown * 3); // 3 chars por byte
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
                    if (cleanOpCode == 0x00)
                        op_sys(operandA, filei);
                    else
                        opTable1[cleanOpCode](operandA);
                } else{
                  opTable0[cleanOpCode]();
                }

                if (flag){
                    //opTable1[0x00](0xFF);
                    sys_breakpoint(filei);
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
        printf("==========================================\n");
        printf("            END OF DISASSEMBLER           \n");
        printf("==========================================\n");
    } else {
        if (IP == 0xFFFFFFFF) {
            printf("==========================================\n");
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

void analizeHeader(FILE *fileA,FILE *fileB, int debug,int gotParams, uint32_t offsetPosition, int argc, uint16_t paramSegmentSize) {
    uint8_t opCode;
    char header[5] = {0};
    uint8_t version;
    uint16_t codeSize;

    if ((fread(header, sizeof(uint8_t), 5, fileA) != 5 || fread(&version, sizeof(uint8_t), 1, fileA) != 1) &&
        (fread(header, sizeof(uint8_t), 5, fileB) != 5 || fread(&version, sizeof(uint8_t), 1, fileB) != 1)) {
        printf("Error: No se pudo leer el header del archivo\n");
        return;
    }

    if ( (strncmp(header, "VMX25", 5) != 0 || (version != 0x01 && version != 0x02)) &&
         (strncmp(header, "VMI25", 5) != 0 || (version != 0x01)) ) {
        printf("Error: File not valid (Header: %.5s, Version: 0x%02X)\n", header, version);
        return;
    }

    if (fileA != NULL){
        if (version == 0x01) {
            uint8_t sizeHigh, sizeLow;
            fread(&sizeHigh, sizeof(uint8_t), 1, fileA);
            fread(&sizeLow, sizeof(uint8_t), 1, fileA);
            codeSize = (sizeHigh << 8) | sizeLow;  // Big-endian
            
            setSegmentDataLength(codeSize);
            setSegmentDataLength(16384 - codeSize);

            for (int i = 0; i < codeSize; i++) {
                fread(&opCode, sizeof(uint8_t), 1, fileA);
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
                
                // El header lee en este orden: Code, Data, Extra, Stack, Const
                uint16_t sizes[6]; // [Code, Data, Extra, Stack, Const, Param]
                uint16_t physicalOrder[6]; // Orden fisico: Param(?), Const, Code, Data, Extra, Stack
                
                // Leer tamaños del header
                for (int ii = 0; ii < 5; ii++) {
                    fread(&sizeHigh, sizeof(uint8_t), 1, fileA);
                    fread(&sizeLow, sizeof(uint8_t), 1, fileA);
                    sizes[ii] = ((uint16_t)sizeHigh << 8) | sizeLow; // Code=0, Data=1, Extra=2, Stack=3, Const=4
                    // printf("Segment %d size: %04X\n", ii, sizes[ii]);
                }
                
                // Usar el tamaño del Param Segment calculado previamente
                physicalOrder[0] = paramSegmentSize;
                
                // Orden físico: Param(0), Const(4), Code(0), Data(1), Extra(2), Stack(3)
                physicalOrder[1] = sizes[4]; // Const
                physicalOrder[2] = sizes[0]; // Code
                physicalOrder[3] = sizes[1]; // Data
                physicalOrder[4] = sizes[2]; // Extra
                physicalOrder[5] = sizes[3]; // Stack
                
                // Crear segmentos en orden físico (solo los que tienen tamaño > 0)
                uint32_t segmentTableIndex = 0;
                
                // Primero: marcar todos los registros como inválidos
                setRegister(30, 0xFFFFFFFF); // KS  
                setRegister(26, 0xFFFFFFFF); // CS
                setRegister(27, 0xFFFFFFFF); // DS
                setRegister(28, 0xFFFFFFFF); // ES
                setRegister(29, 0xFFFFFFFF); // SS
                
                // Segundo: crear segmentos en orden físico y asignar registros
                for (int i = 0; i < 6; i++) {
                    if (physicalOrder[i] > 0) {
                        // Crear segmento en tabla de descriptores primero
                        setSegmentDataLength(physicalOrder[i]);
                        
                        // Mapear índice lógico (i) a registro correcto
                        uint32_t regNum = 0xFFFFFFFF;
                        
                        switch (i) {
                            case 1: regNum = 30; break; // KS - Const Segment
                            case 2: regNum = 26; break; // CS - Code Segment
                            case 3: regNum = 27; break; // DS - Data Segment
                            case 4: regNum = 28; break; // ES - Extra Segment
                            case 5: regNum = 29; break; // SS - Stack Segment
                        }
                        
                        if (regNum != 0xFFFFFFFF && i != 0) {
                            // Asignar registro con índice en la tabla de descriptores
                            uint32_t regValue = (segmentTableIndex << 16) | 0x0000;
                            setRegister(regNum, regValue);
                        }
                        segmentTableIndex++;
                    }
                }
/*
                // Mostrar tabla de segmentos y registros 
                printf("\n=== TABLA DE SEGMENTOS ===\n");
                printf("Indice | Segmento | Base  | size\n");
                printf("-------|----------|-------|--------\n");
                for (int i = 0; i < segmentTable.position; i++) {
                    uint16_t base, length;
                    getSegmentRange(i, &base, &length);
                    printf("  %2d   |     %2d   | %04X  | %04X\n", i, i, base, length);
                }
*/                
                // Mostrar registros de segmento
                /*
                uint32_t CS, DS, ES, SS1, KS1, PS1;
                getRegister(26, &CS);
                getRegister(27, &DS);
                getRegister(28, &ES);
                getRegister(29, &SS1);
                getRegister(30, &KS1);
                getRegister(31, &PS1);
                
                printf("\n=== REGISTROS DE SEGMENTO ===\n");
                printf("CS=%08X (indice tabla: %d)\n", CS, CS == 0xFFFFFFFF ? -1 : (CS >> 16));
                printf("DS=%08X (indice tabla: %d)\n", DS, DS == 0xFFFFFFFF ? -1 : (DS >> 16));
                printf("ES=%08X (indice tabla: %d)\n", ES, ES == 0xFFFFFFFF ? -1 : (ES >> 16));
                printf("SS=%08X (indice tabla: %d)\n", SS1, SS1 == 0xFFFFFFFF ? -1 : (SS1 >> 16));
                printf("KS=%08X (indice tabla: %d)\n", KS1, KS1 == 0xFFFFFFFF ? -1 : (KS1 >> 16));
                printf("PS=%08X\n", PS1);
                */
                uint32_t entryPoint;
                fread(&sizeHigh, sizeof(uint8_t), 1, fileA);
                fread(&sizeLow, sizeof(uint8_t), 1, fileA);
                entryPoint = ((uint16_t)sizeHigh << 8) | sizeLow;  // Big-endian
                
                uint32_t direccion_logica;
                uint16_t base, tam;
                getRegister(26, &direccion_logica);
                getSegmentRange((direccion_logica >> 16), &base, &tam);
                setRegister( 3, (direccion_logica & 0xFFFF0000) | (entryPoint));

                // Cargar Code Segment
                /// printf("base: %04X, size: %04X\n", base, tam);
                for (int i = 0; i < tam; i++) {
                    fread(&opCode, sizeof(uint8_t), 1, fileA);
                    writeByte(base + i, opCode);
                }

                // Cargar Const Segment (si existe)
                uint32_t KS;
                getRegister(30,&KS);
                if (KS != 0xFFFFFFFF){
                    getSegmentRange((KS >> 16), &base, &tam);
                    for (int i = 0; i < tam; i++) {
                        fread(&opCode, sizeof(uint8_t), 1, fileA);
                        writeByte(base + i, opCode);
                    }
                }
                
                if (debug != 0 && KS != 0xFFFFFFFF){
                    int t = 0, j = 0;
                    uint32_t actual, base1 = (uint32_t) base;
                    char car = ' ';
                    setRegister(10, 0x2);
                    setRegister(12, 0xFFFFFFFF);
                    while (base1 < tam) {
                        printf("[%04X] ", base1);
                        t = 0;
                        while ((t < 6) && (car != '\0')){
                            readByte((base1 + t), &car);
                            printf("%02X",car);
                            printf(" ");
                            t++;
                        }
                        
                        if ((car != '\0'))
                            printf("...");
                        else {
                            readByte((base1 + t), &car); // leo el nulo
                            printf("%02X",car);
                        }
                        printf(" | \"");
                        t = 0;
                        readByte(base1, &car);
                        while ((car != '\0') && (car != '\n') && (car != '\r')) {
                            if (car > 0x1F || car < 0x7F)
                                printf("%c",car);
                            else
                                printf(".");    
                            t++;
                            memoryAccess((KS >> 16), (KS & 0xFFFF) + j + t, &actual, &base1, 3);
                            setRegister(2, car);
                            readByte(base1, &car);
                        }
                        j += (t + 1);
                        base1++;
                        printf("\\n\"\n");
                    }
                }

                // Setear SP al tope del stack (base + tamaño)
                uint32_t SS;
                getRegister(29,&SS);
                getSegmentRange((SS >> 16), &base, &tam);
                setRegister(7, (SS & 0xFFFF0000) | (tam));

                // Push de argc y argv si hay parámetros
                if (gotParams && offsetPosition != 0xFFFFFFFF) {
                    // Push argv (puntero al array de argumentos en Param Segment)
                    opTable1[0x0B](0x02000000 | (offsetPosition & 0x00FFFFFF)); // PUSH 0x0000 (offset en Param Segment)
                    // Push argc
                    opTable1[0x0B](0x02000000 | argc);
                } else {
                    // Sin parámetros: argc=0, argv=0xFFFFFFFF
                    opTable1[0x0B](0x0200FFFF); // PUSH 0xFFFF (puntero inválido)
                    opTable1[0x0B](0x02000000); // PUSH 0 (argc = 0)
                }
                opTable1[0x0B](0x0200FFFF); // PUSH -1 (ret)
            }
    } else
        if (fileB != NULL){ // Solo hay imagen
            uint8_t datoLeido;
            codeSize = 0;
            int r;
            for (r = 0; r < 2; r++) {
                fread(&datoLeido, sizeof(uint8_t), 1, fileB);
                codeSize = (codeSize << 8) | datoLeido;
            }
            initMemory(codeSize);
            int p;
            uint32_t aux;
            // Setteo de registros
            for (p = 0; p < 32; p++){
                for (r = 0; r < 4; r++) {
                    fread(&datoLeido, sizeof(uint8_t), 1, fileB);
                    aux = (aux << 8) | datoLeido;
                }
                setRegister(p,aux);
            }
            aux = 0x0;
            initSegmentTable();
            // Setteo de tabla
            for (p = 0; p < 8; p++){
                for (r = 0; r < 4; r++) {
                    fread(&datoLeido, sizeof(uint8_t), 1, fileB);
                    aux = (aux << 8) | datoLeido;
                }
                if (aux != 0xFFFFFFFF)
                    setSegmentTable(aux);
            }
            // Setteo de memoria
            for (p = 0; p < memory.size; p++) {
                fread(&opCode, sizeof(uint8_t), 1, fileB);
                writeByte(p, opCode);
            }
            uint32_t KS;
            uint16_t base,tam;
            getRegister(30,&KS);
            getSegmentRange((KS >> 16) & 0xFFFF, &base, &tam);
            if (debug != 0 && KS != 0xFFFFFFFF){
                int t = 0, j = 0;
                uint32_t actual, base1 = (uint32_t) base;
                char car = ' ';
                setRegister(10, 0x2);
                setRegister(12, 0xFFFFFFFF);
                while (base1 < tam) {
                    printf("[%04X] ", base1);
                    t = 0;
                    while ((t < 6) && (car != '\0')){
                        readByte((base1 + t), &car);
                        printf("%02X",car);
                        printf(" ");
                        t++;
                    }
                    
                    if ((car != '\0'))
                        printf("...");
                    else {
                        readByte((base1 + t), &car); // leo el nulo
                        printf("%02X",car);
                    }
                    printf(" | \"");
                    t = 0;
                    readByte(base1, &car);
                    while ((car != '\0') && (car != '\n') && (car != '\r')) {
                        if (car > 0x1F || car < 0x7F)
                            printf("%c",car);
                        else
                            printf(".");    
                        t++;
                        memoryAccess((KS >> 16), (KS & 0xFFFF) + j + t, &actual, &base1, 3);
                        setRegister(2, car);
                        readByte(base1, &car);
                    }
                    j += (t + 1);
                    base1++;
                    printf("\\n\"\n");
                }
            }
        } else { 
            return;
        }
    
        printf("==========================================\n");
        printf("          STARTING EXECUTION             \n");
        printf("==========================================\n");
    

    beginExecution(fileB, debug);
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
    
    int i = 1, len;
    FILE *fileA = NULL;
    FILE *fileB = NULL;
    int memorySize = 1024, debug = 0, gotVmx = 0, gotParams = 0;

    // Variables para parámetros
    char **lista = NULL;
    uint32_t *offsets = NULL;
    int j = 0;
    uint32_t offsetAcum = 0;
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
        else if (len > 4 && strcmp(&argv[i][len-4], ".vmi") == 0) {
            fileB = fopen(argv[i], "r+b");
        }
        // Opción de memoria m=M
        else if (len > 2 && argv[i][0] == 'm' && argv[i][1] == '=') {
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
        else
            if (argv[i][0] == '-') {
                if (argv[i][1] == 'd') {
                    debug = 1;
                }
                // Opción de parámetros
                else if (argv[i][1] == 'p') {
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
            }
        i++;
    }

    // Verificar que se haya especificado un archivo VMX
    if (!fileA && !fileB) {
        printf("Error: No VMX file specified\n");
        return 1;
    }

    // Inicializar componentes con tamaño de memoria dinámico
    initMemory(memorySize);
    initRegisters();
    initSegmentTable();
    initOpTable();

    uint32_t direccion_fisica = 0x0, argcPos = 0xFFFFFFFF, PS = 0x0;
    uint16_t paramSegmentSize = 0;

    if (gotParams && lista != NULL && offsets != NULL) {

        // Escribir los strings en memoria
        for (int idx = 0; idx < j; idx++) {
            for (int k = 0; k <= strlen(lista[idx]); k++) { // Incluye null terminator
                writeByte(direccion_fisica, lista[idx][k]);
                direccion_fisica++;
            }
        }
        argcPos = direccion_fisica;
        PS |= direccion_fisica;
        setRegister(31, PS);

        // Escribir los offsets en memoria
        for (int idx = 0; idx < j; idx++) {
            for (int k = 0; k < 4; k++) {
                uint8_t byte = (offsets[idx] >> ((3 - k) * 8)) & 0xFF;
                writeByte(direccion_fisica, byte);
                direccion_fisica++;
            }
        }

        // Guardar tamaño del Param Segment
        paramSegmentSize = direccion_fisica;

        // Liberar memoria de parámetros
        for (int k = 0; k < j; k++) {
            free(lista[k]);
        }

        free(lista);
        free(offsets);
    }

    analizeHeader(fileA, fileB, debug, gotParams, argcPos, j, paramSegmentSize);
     
    // Liberar recursos
    if (fileA)
        fclose(fileA);
    if (fileB)
        fclose(fileB);
    
    // Liberar memoria dinámica
    freeMemory();
    
    return 0;
}

/*
gcc -o vmx.exe main.c src/components/memory.c src/components/registers.c src/components/segmentTable.c src/functions/directions.c src/functions/operations.c src/functions/noOperatorOperations.c src/functions/oneOperatorsOperations.c src/functions/twoOperatorsOperations.c
*/