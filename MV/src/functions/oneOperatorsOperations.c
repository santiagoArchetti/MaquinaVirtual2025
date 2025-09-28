#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include "../../include/twoOperatorsOperations.h"
#include <stdio.h>


void op_sys(uint32_t op1) {
    
    int operacionCode = op1 & 0x0000001F;
    if (operacionCode == 01) {
        sys_read();
    } else if (operacionCode == 02) {
        sys_write();
    } else {
        printf("Error: SYS code invalid: %u\n", op1);
        writeRegister(3, 0xFFFFFFFF); // Terminar ejecucion por error
    }
}

void sys_read() {
    uint32_t eax, edx, ecx;
    
    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretacion
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    getRegister(13, &edx);  // EDX - direccion logica base
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos

    printf("SYS READ | Dir: 0x%08X | Count: %u | Size: %04X\n", 
           edx, cantidad, tamano_celda);
    
    for (int i = 0; i < cantidad; i++) {
        uint32_t direccion_actual = edx + (i * tamano_celda);
        uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
        // Mostrar prompt con direccion fisica
        printf("[%04X]: ", (edx & 0xFFFF) + (i * 4));
        
        if (eax & 0x01) { // Decimal
            int32_t valor;
            scanf("%d", &valor);
            
            // Escribir valor en memoria (big-endian)
            for (int j = 0; j < tamano_celda && j < 4; j++) {
                uint8_t byte = (valor >> ((tamano_celda - 1 - j) * 8)) & 0xFF;
                writeByte(direccion_fisica + j, byte);
            }
            
        } else if (eax & 0x02) { // Caracteres
            char buffer[256];
            scanf("%s", buffer);
            
            // Escribir caracteres en memoria
            for (int j = 0; j < tamano_celda && buffer[j] != '\0'; j++) {
                writeByte(direccion_fisica + j, (uint8_t)buffer[j]);
            }
            
        } else if (eax & 0x08) { // Hexadecimal
            uint32_t valor;
            scanf("%x", &valor);
            
            // Escribir valor en memoria (little-endian)
            for (int j = 0; j < tamano_celda && j < 4; j++) {
                uint8_t byte = (valor >> (j * 8)) & 0xFF;
                writeByte(direccion_fisica + j, byte);
            }
            
        } else if (eax & 0x04) { // Octal
            uint32_t valor;
            scanf("%o", &valor);
            
            // Escribir valor en memoria (little-endian)
            for (int j = 0; j < tamano_celda && j < 4; j++) {
                uint8_t byte = (valor >> (j * 8)) & 0xFF;
                writeByte(direccion_fisica + j, byte);
            }
            
        } else if (eax & 0x10) { // Binario
            char binario[33];
            scanf("%s", binario);
            
            uint32_t valor = 0;
            for (int k = 0; binario[k] != '\0'; k++) {
                valor = (valor << 1) + (binario[k] - '0');
            }
            
            // Escribir valor en memoria (little-endian)
            for (int j = 0; j < tamano_celda && j < 4; j++) {
                uint8_t byte = (valor >> (j * 8)) & 0xFF;
                writeByte(direccion_fisica + j, byte);
            }
            
        } else {
            printf("Error: Interpretation mode invalid: 0x%02X\n", eax);
            writeRegister(3, 0xFFFFFFFF);
            return;
        }
    }
}

void sys_write() {
    uint32_t eax, edx, ecx, auxEAX;
    int k;

    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretacion
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    getRegister(13, &edx);  // EDX - direccion logica base
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos

    printf("SYS WRITE | Dir: 0x%08X | Count: %u | Size: %04X\n", 
           edx, cantidad, tamano_celda);
 
    for (int i = 0; i < cantidad; i++) {
        uint32_t direccion_actual = edx + (i * tamano_celda);
        uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
        // Leer valor de memoria (big-endian)
        uint32_t valor = 0;
        for (int j = 0; j < tamano_celda && j < 4; j++) {
            uint8_t byte;
            readByte(direccion_fisica + j, &byte);
            valor |= ((uint32_t)byte << ((tamano_celda - 1 - j) * 8));
        }
        
        // Mostrar prompt con direccion fisica y valor
        auxEAX = eax;
        k = 0;
        if (auxEAX > 0x0 && auxEAX <= 0x1F) {
            printf("[%04X]: ", (edx & 0xFFFF) + (i * 4) );      // (i * 4) Desplazamiento "Offset"
            while (k < 5){
                if ((auxEAX & 0x1) == 0x1){
                    switch (k) {
                        case 0: {printf("D:%d", (int32_t)valor);                            // Decimal
                            break;}
                        case 1: {printf("C:");                                              // Caracter
                                for (int j = 0; j < tamano_celda; j++) {
                                    uint8_t byte;
                                    readByte(direccion_fisica + j, &byte);
                                    if (byte != 0) printf("%c", byte);
                                }
                            break;}
                        case 2: {printf("O:%o", valor);                                     // Octal
                            break;}
                        case 3: {printf("H:%08X", valor);                                   // Hexadecimal
                            break;}
                        case 4: {printf("B:");                                              // Binario
                            for (int bit = 31; bit >= 0; bit--) {
                                printf("%d", (valor >> bit) & 1);
                            }
                            break;}
                    }
                    // Si hay más formatos activos, agregar separador
                    if ((auxEAX >> 1) != 0) printf(" | ");
                }
                k++;
                auxEAX = auxEAX >> 1;
            }
            printf("\n");
        }else {
            printf("Error: formato de escritura invalido");
            writeRegister(3,0xFFFFFFFF);
        }
    }
}
/* --------------------- JUMPS ------------------------ */
void op_jmp(uint32_t op1) {
    writeRegister(3, op1 & 0x00FFFFFF); // Actualizar IP
    printf("JMP: Jumping to address: %04x\n", op1 & 0xFFFF);
}

void op_jz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000001) {  // Z flag
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001) && !(cc & 0x00000002)) {  // No Z y no N
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000002) {  // N flag
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001)) {  // No Z flag
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if ((cc & 0x00000001) || (cc & 0x00000002)) {  // Z o N
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000002)) {  // No N flag
        writeRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}
/* ----------------------------------------------------- */
void op_not(uint32_t op1) {

    uint8_t sizeOp1 = op1 >> 24;
    uint32_t aux;
    int reg = binADecimal(op1);

    if ( sizeOp1 == 2 ){
        writeRegister(3,0xFFFFFFFF);
    } else if ( sizeOp1 == 1){     // De registro
        getRegister(reg, &aux);
        writeRegister(reg, ~aux);
    
    } else if ( sizeOp1 == 3){     // De memoria
        readMemory(sizeOp1, &aux, op1);
        writeMemory(sizeOp1, ~aux, op1);

    }
    setCondicion(~aux);
}

