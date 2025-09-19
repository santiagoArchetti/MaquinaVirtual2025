#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include "../../include/twoOperatorsOperations.h"
#include <stdio.h>


void op_sys(uint32_t op1) {
    printf("SYS ejecutado con código: %u\n", op1);
    
    if (op1 == 0x01) {
        sys_read();
    } else if (op1 == 0x02) {
        sys_write();
    } else {
        printf("Error: Código SYS inválido: %u\n", op1);
        writeRegister(3, 0xFFFFFFFF); // Terminar ejecución por error
    }
}

void sys_read() {
    uint32_t eax, edx, ecx;
    
    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretación
    getRegister(13, &edx);  // EDX - dirección lógica base
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos
    
    printf("SYS READ - Modo: 0x%02X, Dir: 0x%08X, Cant: %u, Tamaño: %u\n", 
           eax, edx, cantidad, tamano_celda);
    
    for (int i = 0; i < cantidad; i++) {
        uint32_t direccion_actual = edx + (i * tamano_celda);
        uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
        // Mostrar prompt con dirección física
        printf("[%04X]: ", direccion_fisica & 0xFFFF);
        
        if (eax & 0x01) { // Decimal
            int32_t valor;
            scanf("%d", &valor);
            
            // Escribir valor en memoria (little-endian)
            for (int j = 0; j < tamano_celda && j < 4; j++) {
                uint8_t byte = (valor >> (j * 8)) & 0xFF;
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
            printf("Error: Modo de interpretación inválido: 0x%02X\n", eax);
            writeRegister(3, 0xFFFFFFFF);
            return;
        }
    }
}

void sys_write() {
    uint32_t eax, edx, ecx;
    
    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretación
    getRegister(13, &edx);  // EDX - dirección lógica base
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos
    
    printf("SYS WRITE - Modo: 0x%02X, Dir: 0x%08X, Cant: %u, Tamaño: %u\n", 
           eax, edx, cantidad, tamano_celda);
    
    for (int i = 0; i < cantidad; i++) {
        uint32_t direccion_actual = edx + (i * tamano_celda);
        uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
        // Leer valor de memoria (little-endian)
        uint32_t valor = 0;
        for (int j = 0; j < tamano_celda && j < 4; j++) {
            uint8_t byte;
            readByte(direccion_fisica + j, &byte);
            valor |= ((uint32_t)byte << (j * 8));
        }
        
        // Mostrar prompt con dirección física y valor
        printf("[%04X]: ", direccion_fisica & 0xFFFF);
        
        if (eax & 0x01) { // Decimal
            printf("%d\n", (int32_t)valor);
            
        } else if (eax & 0x02) { // Caracteres
            for (int j = 0; j < tamano_celda; j++) {
                uint8_t byte;
                readByte(direccion_fisica + j, &byte);
                if (byte != 0) printf("%c", byte);
            }
            printf("\n");
            
        } else if (eax & 0x08) { // Hexadecimal
            printf("%X\n", valor);
            
        } else if (eax & 0x04) { // Octal
            printf("%o\n", valor);
            
        } else if (eax & 0x10) { // Binario
            for (int bit = 31; bit >= 0; bit--) {
                printf("%d", (valor >> bit) & 1);
            }
            printf("\n");
            
        } else {
            printf("Error: Modo de interpretación inválido: 0x%02X\n", eax);
            writeRegister(3, 0xFFFFFFFF);
            return;
        }
    }
}

/* --------------------- JUMPS ------------------------ */
void op_jmp(uint32_t op1) {
    printf("JMP ejecutado a direccion: %u\n", op1);
    writeRegister(3, op1); // Actualizar IP
}

void op_jz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000001) {  // Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001) && !(cc & 0x00000002)) {  // No Z y no N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x00000002) {  // N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000001)) {  // No Z flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if ((cc & 0x00000001) || (cc & 0x00000002)) {  // Z o N
        writeRegister(3, op1);  // Saltar
    }
}

void op_jnn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x00000002)) {  // No N flag
        writeRegister(3, op1);  // Saltar
    }
}

void op_not(uint32_t op1) {

    uint8_t sizeOp1 = op1 >> 24;
    uint32_t aux;

    if ( sizeOp1 == 1 ){    
        /**
        No puede negar un inmediato? 
        porque donde lo guardaria
        Pero igual deveria modificar el CC no?
        */
        writeRegister(3,0xFFFFFFFF);
    } else if ( sizeOp1 == 2){     // De registro
        getRegister(op1, &aux);
        writeRegister(op1, ~aux);
    
    } else if ( sizeOp1 == 3){     // De memoria
        readMemory(sizeOp1, &aux, op1);
        writeMemory(sizeOp1, ~aux, op1);

    }
    setCondicion(~aux);
}

