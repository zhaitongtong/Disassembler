
#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

// You probably want to create a number of printing routines in this file.
// Put the prototypes in printRoutines.h

/*********************************************************************
   Details on print formatting can be found by reading the man page
   for printf. The formatting rules for the disassembler output are
   given below.  Basically it should be the case that if you take the
   output from your disassembler, and then take the resulting file and
   load it into a Y86-64 simulator the object code produced be the
   simulator should match what your program read. If the simulator
   reports an assembler error then you need to fix the output of your
   dissassembler so that it is acceptable to the simulator.

   The printing rules are as follows:
   
     1) The position should be printed if the first instruction or
        directive is not found at the start of the file. The .pos
        directive should be used, with no leading spaces. It should be
        followed by a space and the numeric value of the position to
        be printed. The position should also be printed when two or
        more consecutive halt instructions are found, in which case it
        should be preceded with a blank line. The position's numeric
        value should be printed in hex (lowercase letters) with the
        appropriate leading 0x. Leading zeros are to be suppressed.

     2) For instructions and other directives (.quad, .byte), there
        should be 4 spaces at the start of the line.  The instruction
        field follows, and is 8 characters long. The instruction is to
        be printed left justified in those 8 characters (%-8s). All
instructions are to be printed in lower case.

     3) After the instruction field the first operand is to be
        printed. No extra space is needed, since the 8 characters for
        instructions is enough to leave a space between instruction
        and operands.
 
     4) If there is a second operand then there is to be a comma
        immediately after the first operand (no spaces) and then a
        single space followed by the second operand.

     5) After the instruction and operands, one or more spaces (or tab
        characters) are to be printed, followed by the "#" character
        (indicating a comment) and another space. After this, the
        program is to print, left justified, the hex representation
        (using uppercase letters) of the memory values corresponding
        to the assembler instruction and operands that were printed.

         a) Ideally all "#" characters should be aligned on the same
            column, though this is not required.

     6) The rules for printing operands are as follows: 

         a) Registers: A register is to be printed with the % sign and
            then its name (e.g., %rsp, %rax, %r8). Register names are
            printed in lower case.
 
         b) All numbers are to be printed in hex (with lowercase
            letters) with the appropriate leading 0x. Leading zeros
            are to be suppressed. A value of zero is to be printed as
            "0x0". The numbers are assumed to be unsigned.

         c) A base displacement address is to be printed as D(reg)
            where the printing of D follows the rules in (b), and the
            printing of reg follows the rules in (a). Note there are
            no spaces between the D and "(" or between reg and the "("
            or ")".
        
         d) An address such as that used by a call or jump is to be
            printed as in (b). For these instructions no "$" is
            required.

         e) A constant (immediate value), such as that used by irmovq
            is to be printed as a number in (b) but with a "$"
            immediately preceeding the 0x without any spaces.
           
     7) The mnemonics for the instruction are to conform to those
        described in the textbook and lecture slides. In particular,
        the unconditional move instruction is to be printed as rrmovq,
        while the conditional move is to be printed as cmovXX, where
        XX is the condition (e.g., cmovle).

     8) The arguments for the format string in the example printing
        are strictly for illustrating the spacing. You are free to
        construct the output however you want.
 
********************************************************************************/

//print the position of the instruction to the outputfile
int printPosition(FILE *out, unsigned long location) {
  return fprintf(out, ".pos 0x%lx\n", location);
}

/* This is a function to demonstrate how to do print formatting. It
 * takes the file descriptor the I/O is to be done to. You are not
 * required to use the same type of printf formatting, but you must
 * produce the same resulting format. You are also not required to
 * keep this function in your final result, and may replace it with
 * other equivalent instructions.
 */
int samplePrintInstruction(FILE *out) {

  int res = 0;
  
  char * r1 = "%rax";
  char * r2 = "%rdx";
  char * inst1 = "rrmovq";
  char * inst2 = "jne";
  char * inst3 = "irmovq";
  char * inst4 = "mrmovq";
  unsigned long destAddr = 8193;
  
  res += fprintf(out, "    %-8s%s, %s          # %-22s\n", 
		 inst1, r1, r2, "2002");

  res += fprintf(out, "    %-8s0x%lx              # %-22s\n", 
		 inst2, destAddr, "740120000000000000");

  res += fprintf(out, "    %-8s$0x%lx, %s         # %-22s\n", 
		 inst3, 16L, r2, "30F21000000000000000");

  res += fprintf(out, "    %-8s0x%lx(%s), %s # %-22s\n", 
		 inst4, 65536L, r2, r1, "50020000010000000000"); 
  
  res += fprintf(out, "    %-8s%s, %s          # %-22s\n", 
		 inst1, r2, r1, "2020");
  
  res += fprintf(out, "    %-8s0x%lx  # %-22s\n", 
		 ".quad", 0xFFFFFFFFFFFFFFFFL, "FFFFFFFFFFFFFFFF");

  return res;
}  
  
// return the string representation of the given register that is encoded in integer
const char* getRegister (uint8_t reg){
  switch (reg){
    case 0:
            return "%rax";
    case 1:
            return "%rcx";
    case 2:
            return "%rdx";
    case 3:
            return "%rbx";
    case 4:
            return "%rsp";
    case 5:
            return "%rbp";
    case 6:
            return "%rsi";
    case 7:
            return "%rdi";
    case 8:
            return "%r8";
    case 9:
            return "%r9";
    case 0xA:
            return "%r10";
    case 0xB:
            return "%r11";
    case 0xC:
            return "%r12";
    case 0xD:
            return "%r13";
    case 0xE:
            return "%r14";
    default:
            return "error: invalid register, this line shouldn't be printed\n";
    }
}

// get and store memory value (in integer) of an instruction to buffer
void getMemVal(char* buffer, instruction_t inst){
  switch(inst.type){

    case HALT: case NOP: case RET:
        buffer[0] = inst.opcode;
        break;

    case OPQ: case CMOVXX: case PUSHQ: case POPQ:
        buffer[0] = inst.opcode;
        buffer[1] = inst.ra << 4 | inst.rb;
        break;

    case JXX: case CALL:
        buffer[0] = inst.opcode;
        convertImmValToByteArray(buffer, inst.immVal, 1);
        break;

    case IRMOVQ: case RMMOVQ: case MRMOVQ:
        buffer[0] = inst.opcode;
        buffer[1] = inst.ra << 4 | inst.rb;
        convertImmValToByteArray(buffer, inst.immVal, 2);
        break;

    case INVALID:
        convertImmValToByteArray(buffer, inst.immVal, 0);
        break;
  }
}

// store an 8-byte value (in big endian) into a byte array buffer (in little endian) starting at startPos
void convertImmValToByteArray(char* buffer, uint64_t immVal, int startPos){
  for (uint8_t i = 0; i < 8; i++){
    buffer[startPos + i] = (immVal >> (8 * i) & 0xFF);
  }
}

// print current address, memory value and assembly of an given instruction to output file
void printInstruction (instruction_t inst, long currAddr, FILE* out){
  char* inst_name;

  // store memory value (machine code) of current instruction in memVal using hex representation 
  char buffer[11] = {0};          // longest instruction(10 bytes) + end of string(1 byte) 
  getMemVal(buffer, inst); //each element of buffer contains a byte (two digits) of memory value 					(in integer representation)
  char memVal[22] = {0};// ensure memVal is large enough to contain end of string
				          		
  for (int i = 0; i < inst.size; i++){
    // each buffer element is converted to a two digit hex (uppercase) string and stored in memVal
    
    sprintf(memVal+2*i, "%02X", (unsigned char) buffer[i]);//sprintf:Write formatted data to string
    // "unsigned char": ensure sprintf use unsigned extension when converting 
  }
  
  // print current instruction to output file
  switch(inst.type) {

    case HALT:           
	    inst_name = "halt";
            fprintf(out, "    %-8s%s                    # %-22s\n", inst_name, "", memVal);     	    break;

    case NOP:
	    inst_name = "nop";
	    fprintf(out, "    %-8s%s                    # %-22s\n", inst_name, "", memVal);
            break;

    case RET:            
	    inst_name = "ret";	    
	    fprintf(out, "    %-8s%s                    # %-22s\n", inst_name, "", memVal);
            break;

    case IRMOVQ:            
            inst_name = "irmovq";	    
            fprintf(out, "    %-8s$0x%lx, %s          # %-22s\n", inst_name, inst.immVal, getRegister(inst.rb), memVal);
            break;

    case RMMOVQ:            
	    inst_name = "rmmovq";	    
	    fprintf(out, "    %-8s$0x%lx, %s          # %-22s\n", inst_name, inst.immVal, getRegister(inst.rb), memVal);	    
            break;

    case MRMOVQ:            
	    inst_name = "mrmovq";	    
	    fprintf(out, "    %-8s0x%lx(%s), %s     # %-22s\n", inst_name, inst.immVal, getRegister(inst.rb), getRegister(inst.ra), memVal);	    
            break;

    case CALL:            
	    inst_name = "call";	    
	    fprintf(out, "    %-8s0x%lx               # %-22s\n", inst_name, inst.immVal, memVal);
            break;

    case CMOVXX:
            switch(inst.cmov) {
              case RRMOVQ:
                      inst_name = "rrmovq";
                      break;
              case CMOVLE:
                      inst_name = "cmovle";
                      break;
              case CMOVL:
                      inst_name = "cmovl";
                      break;
              case CMOVE:
                      inst_name = "cmove";
                      break;
              case CMOVNE:
                      inst_name = "cmovne";
                      break;
              case CMOVGE:
                      inst_name = "cmovge";
                      break;
              case CMOVG:
                      inst_name = "cmovg";
                      break;
            }

	    fprintf(out, "    %-8s%s, %s          # %-22s\n", inst_name, getRegister(inst.ra), getRegister(inst.rb), memVal);
            break;

    case OPQ:
            switch(inst.op) {
              case ADD:
                      inst_name = "addq";
                      break;
              case SUB:
                      inst_name = "subq";
                      break;
              case AND:
                      inst_name = "andq";
                      break;
              case XOR:
                      inst_name = "xorq";
                      break;
	      case MUL:
                      inst_name = "mulq";
                      break;
	      case DIV:
                      inst_name = "divq";
                      break;
	      case MOD:
                      inst_name = "modq";
                      break;
            }
            
	    fprintf(out, "    %-8s%s, %s          # %-22s\n", inst_name, getRegister(inst.ra), getRegister(inst.rb), memVal);
            break;

    case JXX:
            switch(inst.jump) {
              case JMP:
                      inst_name = "jmp";
                      break;
              case JLE:
                      inst_name = "jle";
                      break;
              case JL:
                      inst_name = "jl";
                      break;
              case JE:
                      inst_name = "je";
                      break;
              case JNE:
                      inst_name = "jne";
                      break;
              case JGE:
                      inst_name = "jge";
                      break;
              case JG:
                      inst_name = "jg";
                      break;
            }                    
           
	    fprintf(out, "    %-8s0x%lx               # %-22s\n", inst_name, inst.immVal, memVal);
            break;

    case PUSHQ:
            
            inst_name = "pushq";               	
	    fprintf(out, "    %-8s%s              # %-22s\n", inst_name, getRegister(inst.ra), memVal);
            break;

    case POPQ:	    
            inst_name = "popq";
	    fprintf(out, "    %-8s%s              # %-22s\n", inst_name, getRegister(inst.ra), memVal);
            break;

    default:
            // invalid instruction, handled in readInstruction
            break;
  }
}
