/* This file contains the prototypes and constants needed to use the
   routines defined in printRoutines.c
*/

#ifndef _PRINTROUTINES_H_
#define _PRINTROUTINES_H_

#include <stdio.h>


#include <stdint.h>
#include <inttypes.h>


typedef enum iCd {
	HALT, 
	NOP, 
	CMOVXX,
	IRMOVQ,
	RMMOVQ,
	MRMOVQ,
	OPQ, 
	JXX, 
	CALL, 
	RET,
	PUSHQ, 
	POPQ, 
	INVALID
} iCd_t;

typedef enum cmov{
	RRMOVQ, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG
} cmove_t;

typedef enum op {
	ADD, SUB, AND, XOR, MUL, DIV, MOD
} op_t;

typedef enum jump{
	JMP, JLE, JL, JE, JNE, JGE, JG
} jump_t;

typedef struct instruction {
	iCd_t type;
	uint8_t size;		//uint8_t: unsigned char(1 byte)
	uint8_t opcode;
	uint8_t ra;
	uint8_t rb;
	uint64_t immVal;       // immediate value in the instruction(destAddr)
	char* memVal;		// memory value (in hex) of the instruction (machine code)
	cmove_t cmov;
	op_t	op;
	jump_t	jump;
} instruction_t;

int printPosition(FILE *out, unsigned long location);
int sampleprintInstruction(FILE *);
const char* getRegister (uint8_t reg);
void printInstruction (instruction_t inst, long currAddr, FILE* out);
uint64_t get8BytesFromArray (uint8_t* src, int startPos);
void getMemVal(char* buffer, instruction_t inst);
void convertImmValToByteArray(char* buffer, uint64_t immVal, int startPos);


#endif /* PRINTROUTINES */
