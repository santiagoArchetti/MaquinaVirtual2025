#include "../../include/oneOperatorsOperations.h"
#include "../../include/registers.h"
#include "../../include/memory.h"
#include "../../include/segmentTable.h"
#include "../../include/directions.h"
#include "../../include/twoOperatorsOperations.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

void setImage(FILE *arch){
    
    // Creacion de cabecera
    char header[5] = "VMI25";
    uint8_t version = 0x1;
    fwrite(&header, sizeof(uint8_t), 5, arch);
    fwrite(&version, sizeof(uint8_t), 1, arch);
    fwrite(&(memory.size), sizeof(uint16_t), 1, arch);

    // Datos de memoria, registros y segmentos
    int p;
    uint32_t aux;
    uint8_t mem;
    // Setteo de registros
    for (p = 0; p < 32; p++){
        getRegister(p,&aux);
        fwrite(&aux, sizeof(uint32_t), 1, arch);
    }
    // Setteo de tabla
    for (p = 0; p < 8; p++){
        fwrite(&segmentTable.segment[p], sizeof(uint32_t), 1, arch);
    }
    // Setteo de memoria
    for (p = 0; p < memory.size; p++) {
        readByte(p,&mem);
        fwrite(&mem, sizeof(uint8_t), 1, arch);
    }
}

void op_sys(uint32_t op1, FILE *arch) {
    
    int operacionCode = op1 & 0x0000001F;
    switch (operacionCode) {
        case 0x01: sys_read(); break;
        case 0x02: sys_write(); break;
        case 0x03: sys_string_read(); break;
        case 0x04: sys_string_write(); break;
        case 0x07: sys_clear_screen(); break;
        case 0x0F: sys_breakpoint(arch); break;
        default: {
            printf("Error: SYS code invalid: %u\n", op1);
            setRegister(3, 0xFFFFFFFF); // Terminar ejecucion por error
        } break;    // el default lleva break?
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
        printf("[%04X]: ", (direccion_fisica & 0xFFFF));
        if (isValidAddress(direccion_fisica,1, (uint16_t)((edx >> 16) & 0xFFFF) )) {    
            if (eax & 0x01) {                               // Decimal
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
                
                // Escribir valor en memoria (big-endian)
                for (int j = 0; j < tamano_celda && j < 4; j++) {
                    uint8_t byte = (valor >> ((tamano_celda - 1 - j) * 8)) & 0xFF;
                    writeByte(direccion_fisica + j, byte);
                }
                
            } else if (eax & 0x04) { // Octal
                uint32_t valor;
                scanf("%o", &valor);
                
                // Escribir valor en memoria (big-endian)
                for (int j = 0; j < tamano_celda && j < 4; j++) {
                    uint8_t byte = (valor >> ((tamano_celda - 1 - j) * 8)) & 0xFF;
                    writeByte(direccion_fisica + j, byte);
                }
                
            } else if (eax & 0x10) { // Binario
                char binario[33];
                scanf("%s", binario);
                
                uint32_t valor = 0;
                for (int k = 0; binario[k] != '\0'; k++) {
                    valor = (valor << 1) + (binario[k] - '0');
                }
                
                // Escribir valor en memoria (big-endian)
                for (int j = 0; j < tamano_celda && j < 4; j++) {
                    uint8_t byte = (valor >> ((tamano_celda - 1 - j) * 8)) & 0xFF;
                    writeByte(direccion_fisica + j, byte);
                }
                
            } else {
                printf("Error: Interpretation mode invalid: 0x%02X\n", eax);
                setRegister(3, 0xFFFFFFFF);
                return;
            }
        } else {
            printf("Error: Se cayo del segmento: 0x%08X\n", direccion_fisica);
            setRegister(3,0xFFFFFFFF);
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

    printf("SYS WRITE | Dir: 0x%08X | Count: %u | Size: %04X\n", edx, cantidad, tamano_celda);
 
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
            printf("[%04X]: ", (direccion_fisica & 0xFFFF));      // (i * 4) Desplazamiento "Offset"
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
                        case 3: {printf("H:%X", valor);                                   // Hexadecimal
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
            setRegister(3,0xFFFFFFFF);
        }
    }
}

void sys_string_read(){
    
    uint32_t edx,ecx;
    getRegister(12,&ecx);
    getRegister(13,&edx);
    uint32_t direccion_actual = edx;    // probablemente no necesario
    uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
        
    if (ecx > 0) {
        if (isValidAddress(direccion_fisica, ecx + 1, (uint16_t)(edx >> 16) )) {  // Vemos si hay espacio sufciente para escribir
            char car;
            
            for (int i = 0; i <= ecx ; i++){
                scanf("%c",&car);
                writeByte(direccion_fisica + i, car);
                // Por si hay que hacer manejo de la memoria
                memoryAccess((edx >> 16), (edx & 0xFFFF), &direccion_actual, &direccion_fisica, 1);
                setRegister(2, car);
            }
            writeByte(direccion_fisica + ecx + 1,'\0');   // Le agregamos el caracter nulo
        } else {
            printf("Error: Espacio en memoria insuficiente.");
            setRegister(3,0xFFFFFFFF);
        }
    } else
        if (ecx == -1){
            char car = ' ';
            int i = 0;
            while ( isValidAddress(direccion_fisica + i, 1, (uint16_t)(edx >> 16) ) && (car != '\0') ) {
                scanf("%c",&car);
                writeByte(direccion_fisica + i, car);
                memoryAccess((edx >> 16), (edx & 0xFFFF), &direccion_actual, &direccion_fisica, 1);
                setRegister(2, car);
                i++;
            }
            if (isValidAddress(direccion_fisica + i, 1, (uint16_t)(edx >> 16) ))
              writeByte(direccion_fisica + i, '\0');
            else {
                printf("Error: Espacio en memoria insuficiente.");
                 setRegister(3,0xFFFFFFFF);
            }      
        } else {
            printf("Error: Invalid Operation ECX incorrect Value for this operation.");
            setRegister(3,0xFFFFFFFF);
        }
}

void sys_string_write(){

    uint32_t edx;
    getRegister(13,&edx);
    uint32_t direccion_actual = edx;    // probablemente no necesario
    uint32_t direccion_fisica = getFisicalAddress(direccion_actual);
    
    char car;
    if (isValidAddress(direccion_fisica, 1, (uint16_t)(edx >> 16) )){
        int i = 0;
        while (car != '\0') {
            readByte((direccion_fisica + i), &car);
            printf("%c",car);
            i++;
            memoryAccess((edx >> 16), (edx & 0xFFFF), &direccion_actual, &direccion_fisica, 1);
            setRegister(2, car);
        }
    } else {
        printf("ERROR: direccion fisica invalida");
        setRegister(3,0xFFFFFFFF);
    }
    
}

void sys_clear_screen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void sys_breakpoint(FILE *arch){
    char stop;
    scanf("%c", &stop);
    switch (stop) {
        case 'q': setRegister(3,0xFFFFFFFF); break;
        case 'g': flag = 0; break;
        case '\0': flag = 1; break;
        default: {
            printf("ERROR: el caracter (%c) ingresado es invalido",stop);
            setRegister(3,0xFFFFFFFF);
        }
    }
    if (arch == NULL)
        arch = fopen("imagen.vmi", "wb");
    setImage(arch);
}

/* --------------------- JUMPS ------------------------ */
void op_jmp(uint32_t op1) {
    setRegister(3, op1 & 0x00FFFFFF); // Actualizar IP
    printf("JMP: Jumping to address: %04x\n", op1 & 0xFFFF);
}

void op_jz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x40000000) {  // Z flag
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x40000000) && !(cc & 0x80000000)) {  // No Z y no N
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (cc & 0x80000000) {  // N flag
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnz(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x40000000)) {  // No Z flag
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnp(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if ((cc & 0x40000000) || (cc & 0x80000000)) {  // Z o N
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}

void op_jnn(uint32_t op1) {
    uint32_t cc;
    getRegister(17, &cc);
    if (!(cc & 0x80000000)) {  // No N flag
        setRegister(3, op1 & 0x00FFFFFF);  // Saltar
    }
}
/* ----------------------------------------------------- */
void op_not(uint32_t op1) {

    uint32_t mbrValue;  // Variable MBR para operaciones de memoria
    uint8_t sizeOp1 = op1 >> 24;
    uint32_t aux = 0;  // Inicializar aux
    int reg = binADecimal(op1);

    if ( sizeOp1 == 2 ){
        setRegister(3,0xFFFFFFFF);
    } else if ( sizeOp1 == 1){     // De registro
        getRegister(reg, &aux);
        setRegister(reg, ~aux);
    
    } else if ( sizeOp1 == 3){     // De memoria
        readMemory(op1);
        getRegister(2, &aux);
        setRegister(2, ~aux);
        writeMemory(op1);

    }
    if (sizeOp1 != 2) {  // Solo llamar setCondicion si no hay error
        setCondicion(~aux);
    }
}

void op_push(uint32_t op1) {

    uint32_t SP;
    uint32_t SS;
    uint8_t sizeOp1 = op1 >> 24;

    getRegister(7,&SP);
    getRegister(29,&SS);

    
    if ((SP & 0xFF - 4) < (SS & 0XFF)){ // si el valor es menor, es stack overflow
        printf("ERROR: STACK OVERFLOW\n");
        setRegister(3,0xFFFFFFFF);
    }else{
        uint32_t value;
        if(sizeOp1 == 1 ){ // registro
            int reg = op1 & 0xFF;
            uint32_t aux;
            getRegister(reg, &aux);

            uint8_t part = (op1 >> 6) & 0x03;
            if (part == 1 || part == 2)         // AL o AH
                aux = (int8_t)aux;
            else
                if (part == 3)                  // AX
                    aux = (int16_t)aux;

            value = aux; 
            setRegister(2,value);
             
        } else
            if (sizeOp1 == 2) { // inmediato
                value = (int16_t)(op1 & 0xFFFF); // sign extension correcta
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

    uint32_t direccion_fisica = getFisicalAddress(SP - 4);

    if (direccion_fisica > memory.size){
        printf("ERROR: STACK UNDERFLOW");
        setRegister(3,0xFFFFFFFF);
    }
    else{
        uint32_t value;

        readStack(SP); // guarda en mbr tope de la pila

        if (sizeOp1 == 1){ // registro
            uint32_t reg1 = binADecimal(op1);
            getRegister(2,&value);

            int reg = op1 & 0x1F;
            uint8_t part = (op1 >> 6) & 0x03;

            // Ajustar tamaño antes de escribir
            if (part == 1 || part == 2) value &= 0xFF;   // AL y AH
            if (part == 3) value &= 0xFFFF; // AX

            setRegister(reg, value);
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
    op_push(0x01000003);    // pusheo IP (mando IP solo porque pide un operando, pero no es necesario, el mbr ya esta modificado)
    op_jmp(op1);            // verificar que funcione correctamente con la subrutina
}
