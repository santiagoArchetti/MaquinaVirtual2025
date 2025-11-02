Pot:	PUSH BP
	MOV BP, SP
	SUB SP, 4		; var local
	PUSH EAX
	PUSH ECX

	MOV EAX, [BP+8]	; n
	MOV EBX, [BP+12]	; Base
	MOV [SP], EBX
	CMP EAX, 0
	JN PotN			; PotN
	JZ PotZ			; PotZ

ciclo:	MUL EBX, [SP]
	SUB EAX, 1
	CMP EAX, 0
	JZ fin
	JMP ciclo
	
PotN:	MOV ECX, 1
	DIV ECX, EBX
	MUL EAX, -1
	MOV EBX, ECX
	JMP ciclo

PotZ:	CMP EBX, 0
	JZ fini			; fini
	MOV EBX, 1
	JMP fin

fini:	MOV EBX, -1

fin:	POP ECX
	POP EAX
	ADD SP, 4
	MOV SP, BP
	POP BP
	MOV [DS], EAX
	
	MOV EAX, 1
	LDH ECX, 4
	LDL ECX, 1
	MOV EDX, DS
	
	RET