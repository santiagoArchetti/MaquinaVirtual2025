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
void setRegister(int regIndex, uint32_t value) {
    uint8_t reg = (uint8_t) regIndex;
    int index = reg & 0x1F; // bits 0-4 = número de registro
    uint32_t aux;

    if (index < 0 || index >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", index);
        setRegister(3, 0xFFFFFFFF);
        return;
    }

    aux = registers.registerValue[index];

    switch ((reg >> 6) & 0x03) {
        case 0x1:                   // parte baja-baja (byte 0)
            value = (aux & 0xFFFFFF00) | (value & 0xFF);
            break;
        case 0x2:                   // parte alta-baja (byte 1)
            value = (aux & 0xFFFF00FF) | ((value & 0xFF) << 8);
            break;
        case 0x3:                   // Dos bytes menos significativos
            value = (aux & 0xFFFF0000) | (value & 0xFFFF);
            break;
    }

    registers.registerValue[index] = value;
}

// Funcion para cargar el valor de un registro
void getRegister(int regIndex, uint32_t* value) {
    uint8_t reg = (uint8_t) regIndex;
    int index = reg & 0x1F;

    if (index < 0 || index >= REGISTERS_SIZE) {
        printf("Error: Invalid register index: %d\n", index);
        setRegister(3, 0xFFFFFFFF);
        return;
    }

    *value = registers.registerValue[index];

    switch ((reg >> 6) & 0x03) {
        case 0x1:       // parte baja (byte 0)
            *value &= 0xFF;
            break;
        case 0x2:       // parte alta (byte 1)
            *value = (*value >> 8) & 0xFF;
            break;
        case 0x3:       // Dos bytes menos significativos
            *value &= 0xFFFF; 
            break;
    }   
}



int opCodeExists(uint8_t opCode){
    opCode = opCode & 0x1F;
    if ( ( opCode >= 0x10 && opCode <= 0x1F ) || ( opCode >= 0x00 && opCode <= 0x08 ) || ( opCode >= 0x0B && opCode <= 0x0F ) )
        return 1;
    else{
        setRegister(3,0xFFFFFFFF);
        return 0;
    }
}

int binADecimal(uint32_t op) {
    op = op & 0x1F;   // Sacamos el tipo de operando (1F por los 32 registros)
    int sum = 0;
    for (int i = 0 ; i < 6 ; i++){
        if ( (op & 0x1) == 0x1 && sum < 32)
            sum += pow(2, i);
        op = op >> 1;
    }
    return sum;
}

void getOperandName(uint32_t name) {
    uint8_t type = (name >> 24) & 0xFF;

    if (type == 0x01) {
        // 🔹 Tipo registro
        const char* nameRegister = "UNK";
        int regIndex = name & 0x1F;
        uint8_t part = (name >> 6) & 0x3;

        switch (part) {
            case 0x0: // generales, completos, de control
                switch (regIndex) {
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
                }
                break;
            case 0x1: // parte baja (Low)
                switch (regIndex) {
                    case 10: nameRegister = "AL"; break;
                    case 11: nameRegister = "BL"; break;
                    case 12: nameRegister = "CL"; break;
                    case 13: nameRegister = "DL"; break;
                    case 14: nameRegister = "EL"; break;
                    case 15: nameRegister = "FL"; break;
                }
                break;
            case 0x2: // parte alta (High)
                switch (regIndex) {
                    case 10: nameRegister = "AH"; break;
                    case 11: nameRegister = "BH"; break;
                    case 12: nameRegister = "CH"; break;
                    case 13: nameRegister = "DH"; break;
                    case 14: nameRegister = "EH"; break;
                    case 15: nameRegister = "FH"; break;
                }
                break;
            case 0x3: // completo (16 bits)
                switch (regIndex) {
                    case 10: nameRegister = "AX"; break;
                    case 11: nameRegister = "BX"; break;
                    case 12: nameRegister = "CX"; break;
                    case 13: nameRegister = "DX"; break;
                    case 14: nameRegister = "EX"; break;
                    case 15: nameRegister = "FX"; break;
                }
                break;
        }
        printf("%s", nameRegister);
    } else 
        if (type == 0x02) {
            // 🔹 Tipo inmediato (constante)
            printf("%04X", (uint16_t)(name & 0x0000FFFF));
        } else 
            if (type == 0x03) {
                // 🔹 Tipo memoria → formato: [registro + desplazamiento]
                uint16_t regPart = (name >> 16) & 0xFFFF;
                uint16_t offset  = name & 0xFFFF;

                printf("[");
                getOperandName( ((uint32_t)regPart) | (0x01u << 24) ); // forzar tipo registro (0x01)
                printf(" + %04X]", offset);
            } else {
                printf("UNK");
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