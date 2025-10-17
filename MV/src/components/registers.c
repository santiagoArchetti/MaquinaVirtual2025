#include "../../include/registers.h"
#include <stdio.h>
#include <math.h>

Registers registers;  // definicion global

void initRegisters() { // inicializamos en 0 todos los registros
    for (int i = 0; i < REGISTERS_SIZE; i++) {
        registers.registerValue[i] = 0;
    }
}

// Funcion para escribir un registro
void setRegister(int8_t regIndex, uint32_t value) {
    uint32_t aux = 0x0;
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", regIndex);
        setRegister(3,0xFFFFFFFF);
    }
    else {
        aux = registers.registerValue[regIndex & 0x1F];
        switch ( (regIndex >> 6) & 0x3 ) {
            case 1: value = (aux & 0xFFFFFFFFFFFF0000) | (value & 0xFFFF); break;
            case 2: value = (aux & 0xFFFFFFFF0000FFFF) | (value & 0xFFFF0000); break; // o aux = (aux & 0xFFFFFFFF0000FFFF) | (value << 8); break;
            case 3: value = (aux & 0xFFFFFFFF00000000)  | (value & 0xFFFFFFFF); break; 
        }  
        registers.registerValue[regIndex & 0x1F] = value; // o [ binADecimal(regIndex & 0x1F) ]
    }
}

// Funcion para cargar el valor de un registro
void getRegister(int8_t regIndex, uint32_t* value) {
    if (regIndex < 0 || regIndex >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", regIndex);
        setRegister(3,0xFFFFFFFF);
    }
    else{
        *value = registers.registerValue[regIndex & 0x1F];
        switch ( (regIndex >> 6) & 0x3 ) {
            case 1: *value &= 0xFFFF; break;
            case 2: {
                *value &= 0xFFFF0000; break;
                *value = *value >> 8;
            }
            case 3: *value &= 0xFFFFFFFF; break;
        }
    }    
}

int opCodeExists(uint8_t opCode){
    opCode = opCode & 0x1F;
    if ( ( opCode >= 0x10 && opCode <= 0x1F ) || ( opCode >= 0x00 && opCode <= 0x08 ) || opCode == 0x0F)
        return 1;
    else{
        setRegister(3,0xFFFFFFFF);
        return 0;
    }
}

int binADecimal(uint32_t op) {
    op = op & 0x0000003F;   // Sacamos el tipo de operando (3F por los 32 registros)
    int sum = 0;
    for (int i = 0 ; i < 6 ; i++){
        if ( (op & 0x00000001) == 0x00000001 && sum < 32)
            sum += pow(2, i);
        op = op >> 1;
    }
    return sum;
}

void getOperandName(uint32_t name) {
 
    if ( (name >> 24) == 0x01){
        // Tipo registro - imprimir nombre del registro
        char* nameRegister = "UNK";
        int regIndex = name & 0x1F;
        switch ( (name >> 6) & 0x3 ){
            case 0x0:
                switch(regIndex) {
                    case 0: nameRegister = "LAR"; break;
                    case 1: nameRegister = "MAR"; break;
                    case 2: nameRegister = "MBR"; break;
                    case 3: nameRegister = "IP"; break;
                    case 4: nameRegister = "OPC"; break;
                    case 5: nameRegister = "OP1"; break;
                    case 6: nameRegister = "OP2"; break;
                    case 7: nameRegister = "SP"; break;
                    case 8: nameRegister = "BP"; break;
                    case 10: nameRegister = "EAX"; break;
                    case 11: nameRegister = "EBX"; break;
                    case 12: nameRegister = "ECX"; break;
                    case 13: nameRegister = "EDX"; break;
                    case 14: nameRegister = "EEX"; break;
                    case 15: nameRegister = "EFX"; break;
                    case 16: nameRegister = "AC"; break;
                    case 17: nameRegister = "CC"; break;
                    case 26: nameRegister = "CS"; break;
                    case 27: nameRegister = "DS"; break;
                    case 28: nameRegister = "ES"; break;
                    case 29: nameRegister = "SS"; break;
                    case 30: nameRegister = "KS"; break;
                    case 31: nameRegister = "PS"; break;
                }; break;
            case 0x1:
                switch(regIndex) {
                    case 10: nameRegister = "AL"; break;
                    case 11: nameRegister = "BL"; break;
                    case 12: nameRegister = "CL"; break;
                    case 13: nameRegister = "DL"; break;
                    case 14: nameRegister = "EL"; break;
                    case 15: nameRegister = "FL"; break;
                };break;
            case 0x2:
                switch(regIndex) {
                    case 10: nameRegister = "AH"; break;
                    case 11: nameRegister = "BH"; break;
                    case 12: nameRegister = "CH"; break;
                    case 13: nameRegister = "DH"; break;
                    case 14: nameRegister = "EH"; break;
                    case 15: nameRegister = "FH"; break;
                };break;
            case 0x3:
                switch(regIndex) {
                    case 10: nameRegister = "AX"; break;
                    case 11: nameRegister = "BX"; break;
                    case 12: nameRegister = "CX"; break;
                    case 13: nameRegister = "DX"; break;
                    case 14: nameRegister = "EX"; break;
                    case 15: nameRegister = "FX"; break;
                };break;
        }
        printf("%s", nameRegister);
    } else if ( (name >> 24) == 0x02){
        // Tipo inmediato 
        printf("%04X", (uint16_t)(name & 0x0000FFFF));
    } else if ( (name >> 24) == 0x03){
        // Tipo memoria - entre [ ]
        //printf("[%06X]", name & 0x00FFFFFF);
        printf("[");
        getOperandName( ((name >> 16) & 0xFFFF) | 0x0001000000000000 );
        printf(" + %04X]", (uint16_t)(name & 0xFFFFFFFF));
    }
}

void setCondicion(uint32_t value) {

	if (value == 0)
		setRegister(17, 0x40000000);
	else if ((value & 0x80000000u) != 0)
		setRegister(17, 0x80000000);
	else
		setRegister(17, 0x0);

}