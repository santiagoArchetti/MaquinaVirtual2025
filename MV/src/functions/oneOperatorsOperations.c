#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include "../../include/twoOperatorsOperations.h"
#include <stdio.h>

void op_sys(uint32_t op1) {
    
    int operacionCode = op1 & 0x0000001F;
    switch (operacionCode) {
        case 01: sys_read(); break;
        case 02: sys_write(); break;
        case 03: sys_string_read(); break;
        case 04: sys_string_write(); break;
        case 07: sys_clear_screen(); break;
        case 1F: sys_breakpoint(); break;
        default: {
            printf("Error: SYS code invalid: %u\n", op1);
            writeRegister(3, 0xFFFFFFFF); // Terminar ejecucion por error
        } break;    // el default lleva break?
    }
    /*
    if (operacionCode == 01) {
        sys_read();
    } else if (operacionCode == 02) {
        sys_write();
    } else {
        printf("Error: SYS code invalid: %u\n", op1);
        writeRegister(3, 0xFFFFFFFF); // Terminar ejecucion por error
    }*/
}

void sys_read() {
    uint32_t eax, edx, ecx;
    
    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretacion
    getRegister(13, &edx);  // EDX - direccion logica base
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos
    
    printf("SYS READ | Dir: 0x%08X | Count: %u | Size: %u\n", 
           eax, edx, cantidad, tamano_celda);
    
    for (int i = 0; i < cantidad; i++) {
        uint32_t direccion_actual = edx + (i * tamano_celda);
        uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
        // Mostrar prompt con direccion fisica
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
            printf("Error: Interpretation mode invalid: 0x%02X\n", eax);
            writeRegister(3, 0xFFFFFFFF);
            return;
        }
    }
}

void sys_write() {
    uint32_t eax, edx, ecx;
    
    // Leer registros involucrados
    getRegister(10, &eax);  // EAX - modo de interpretacion
    getRegister(13, &edx);  // EDX - direccion logica base
    getRegister(12, &ecx);  // ECX - cantidad y tamaño
    
    uint16_t cantidad = ecx & 0xFFFF;        // 16 bits bajos
    uint16_t tamano_celda = (ecx >> 16) & 0xFFFF; // 16 bits altos
    
    printf("SYS WRITE | Dir: 0x%08X | Count: %u | Size: %u\n", 
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
        
        // Mostrar prompt con direccion fisica y valor
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
            printf("Error: Interpretation mode invalid: 0x%02X\n", eax);
            writeRegister(3, 0xFFFFFFFF);
            return;
        }
    }
}

void sys_string_read(){

}

void sys_string_write(){

}

void sys_clear_screen(){
    system("cls");
}

void sys_breakpoint(){
    char stop;
    scanf("%c", &stop);
    switch (stop) {
        case 'q': setRegister(3,0xFFFFFFFF); break;
        case 'g': flag = 0;
        case '': flag = 1;
        default: {
            printf("ERROR: el caracter (%c) ingresado es invalido",stop);
            setRegister(3,0xFFFFFFFF);
        }
    }
}

/* --------------------- JUMPS ------------------------ */
void op_jmp(uint32_t op1) {
    printf("JMP: Jumping to address: %u\n", op1);
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

void op_push(uint32_t op1){
    /*
    decrementar SP en 4 (jijiji)
    si el valor de SP < valor SS --> stack overflow (aborta ejecucion)
    obtener el valor del operando
    transformar valor obtenido a 4 bytes
    almacenrar en big endian.
    */

    uint32_t SP;
    uint32_t SS;
    uint8_t sizeOp1 = op1 >> 24;

    getRegister(7,&SP);
    getRegister(29,&SS);

    if ((SP - 4) < SS){ // si el valor es menor, es stack overflow
        printf("ERROR: STACK OVERFLOW\n");
        setRegister(3,0xFFFFFFFF);
    }else{
        uint32_t value;
        if(sizeOp1 == 1 ){ // registro
            int reg1 = binADecimal(op1);
            getRegister(reg1, &value); // obtengo valor que hay en el registro de op1
            setRegister(2,value);
        } else
            if (sizeOp1 == 2) { // inmediato
                value = op1 & 0xFFFF;
                if ( ((uint16_t)value & 0xFFFF) < 0)
                    value |= 0xFFFF0000;
                setRegister(2,value);   // Setteo MBR
            }
            else
                if (sizeOp1 == 3){ // memoria
                    readMemory(op1);
                }

        writeStack(SP); // guarda nuevo dato en tope de la pila
    }
}

void op_pop(uint32_t op1){

    uint32_t SP;
    uint32_t SS;
    uint8_t sizeOp1 = op1 >> 24;

    getRegister(7,&SP);
    getRegister(29,&SS);

    if ((SP + 4) > MEMORY_SIZE){
        printf("ERROR: STACK UNDERFLOW");
        setRegister(3,0xFFFFFFFF);
    }
    else{
        uint32_t value;

        readStack(SP); // guarda en mbr tope de la pila

        if (sizeOp1 == 1){ // registro
            uint32_t reg1 = binADecimal(op1);
            getRegister(2,&value);
            setRegister(reg1,value);
        }
        else
            if (sizeOp1 == 3) { // memoria
                writeMemory(op1);
            }
            else { // no se admiten operandos inmediatos en el POP
                printf("Operando invalido");
                setRegister(3,0xFFFFFFFF);  // lol
            }
    }
}

void op_call (uint32_t op1){
    uint32_t IP;
    getRegister(3,&IP);     // obtengo IP
    setRegister(2,IP);      // guardo valor del IP en mbr
    op_push(IP);            // pusheo IP (mando IP solo porque pide un operando, pero no es necesario, el mbr ya esta modificado)
    op_jmp(op1);            // verificar que funcione correctamente con la subrutina
}