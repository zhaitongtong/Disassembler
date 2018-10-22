#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

instruction_t readInstruction(FILE* inputStream, long currAddr, FILE* out);
long getAddrOfNextNonZeroByte (FILE* inputStream, long currAddr, long file_length);


int main(int argc, char **argv) {
  
  FILE *machineCode, *outputFile;
  long currAddr = 0; // current position of the file to read at

  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Usage: %s InputFilename [OutputFilename] [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it 
  // for reading and verify that the open did occur.
  machineCode = fopen(argv[1], "rb");

  if (machineCode == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }


  // Second argument is the file to write, attempt to open it for
  // writing and verify that the open did occur. Use standard output
  // if not provided.
  outputFile = argc <= 2 ? stdout : fopen(argv[2], "w");
  
  if (outputFile == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[2], strerror(errno));
    fclose(machineCode);
    return ERROR_RETURN;
  }

  // If there is a 3rd argument present it is an offset so convert it
  // to a numeric value.
  if (4 == argc) {
    errno = 0;
    currAddr = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      fclose(machineCode);
      fclose(outputFile);
      return ERROR_RETURN;
    }
  }


  fprintf(stderr, "Opened %s, starting offset 0x%lX\n", argv[1], currAddr);
  fprintf(stderr, "Saving output to %s\n", argc <= 2 ? "standard output" : argv[2]);


  // Your code starts here.

  fseek(machineCode, 0, SEEK_END); // seek to end of file
  long file_length = ftell(machineCode); // get total number of bytes in the file
  if(file_length == 0)
	printf("Input file is empty\n");
  else if(file_length <= currAddr){
  	printf("Invalid offset,enter a smaller one\n");
  }
  fseek(machineCode, 0, SEEK_SET); // seek back to beginning of file

  //get the first instruction to read
  instruction_t inst;
  inst = readInstruction(machineCode, currAddr, outputFile);
  
  //if first byte to read is a zero-byte, keep reading ultil finding a non-zero byte and print out the position of that non-zero byte.
  if (inst.type == HALT) {     
     currAddr = getAddrOfNextNonZeroByte(machineCode, currAddr + 1, file_length);
     if(currAddr < file_length)
     	printPosition(outputFile, currAddr);
     fseek(machineCode, currAddr, SEEK_SET);
  }
  
  if (inst.type == INVALID){        
	currAddr += inst.size;//change inst size to number of byte read
  }

  fseek(machineCode, currAddr, SEEK_SET);

  // start disassembing
  instruction_t instruction;
  
  while(currAddr < file_length){
    //read instruction
    instruction = readInstruction(machineCode, currAddr, outputFile);
    //print instruction
    if (instruction.type != INVALID) {
      printInstruction(instruction, currAddr, outputFile);
    }
    currAddr += instruction.size;
    
    if (instruction.type == HALT){
      long addOfNextInst = currAddr ;//the address of the instruction following halt
      // move the file reading postion to the next non-zero byte
      currAddr = getAddrOfNextNonZeroByte(machineCode, currAddr, file_length);
      
      if(currAddr != addOfNextInst && currAddr < file_length){
	  fprintf(outputFile,"\n");
          printPosition(outputFile, currAddr);
      }
      fseek(machineCode, currAddr, SEEK_SET);      
    }
  }
  
  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}




// read a single instruction from inputStream, check if the instruction is valid, and return the instruction
// if the instruction is invalid, print the address, memory value and assemlby of it to output file
instruction_t readInstruction(FILE* inputStream, long currAddr, FILE* out){
  uint8_t instruction_buffer [10];
  uint16_t num_of_bytes_fetched;
  fseek(inputStream, currAddr, SEEK_SET);//set file position of inputStream to currAddr(offset)
  fread(instruction_buffer, 1, 1, inputStream);
  instruction_t instruction;
  
  switch (instruction_buffer[0]) {
    
    case 0x00:	  
          instruction.type = HALT;
          instruction.size = 1;
          instruction.opcode = 0x00;
          break;

    case 0x10:
            instruction.type = NOP;
            instruction.size = 1;
            instruction.opcode = 0x10;
            break;

    case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26:
            instruction.type = CMOVXX;
            switch (instruction_buffer[0])
            {
            case 0x20:
                    instruction.cmov = RRMOVQ;
                    instruction.opcode = 0x20;
                    break;
            case 0x21:
                    instruction.cmov = CMOVLE;
                    instruction.opcode = 0x21;
                    break;
            case 0x22:
                    instruction.cmov = CMOVL;
                    instruction.opcode = 0x22;
                    break;
            case 0x23:
                    instruction.cmov = CMOVE;
                    instruction.opcode = 0x23;
                    break;
            case 0x24:
                    instruction.cmov = CMOVNE;
                    instruction.opcode = 0x24;
                    break;
            case 0x25:
                    instruction.cmov = CMOVGE;
                    instruction.opcode = 0x25;
                    break;
            case 0x26:
                    instruction.cmov = CMOVG;
                    instruction.opcode = 0x26;
                    break;
            }

          instruction.size = 2;
          // read remaining bytes of current instruction and check it validity
          // current file reading position is after opcode of the instruction 
          num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
          if (num_of_bytes_fetched < instruction.size-1){
            instruction.type = INVALID;
            break;
          }
          instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
          instruction.rb = instruction_buffer[1] & 0x0F;
          if (instruction.ra > 0xE || instruction.rb > 0xE) { 
            instruction.type = INVALID;
          }
          break;

    case 0x30:
        instruction.type = IRMOVQ;
        instruction.size = 10;
        instruction.opcode = 0x30;

        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra != 0xF || instruction.rb > 0xE) {
          instruction.type = INVALID;  
        }
        instruction.immVal = get8BytesFromArray(instruction_buffer, 2);
        break;

    case 0x40:
        instruction.type = RMMOVQ;
        instruction.size = 10;
        instruction.opcode = 0x40;

        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }

        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra > 0xE || instruction.rb > 0xE) {
          instruction.type = INVALID;
        }
        instruction.immVal = get8BytesFromArray(instruction_buffer, 2);
        break;

    case 0x50:
        instruction.type = MRMOVQ;
        instruction.size = 10;
        instruction.opcode = 0x50;

        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra > 0xE || instruction.rb > 0xE) {
          instruction.type = INVALID;
          break;
        }

        instruction.immVal = get8BytesFromArray(instruction_buffer, 2);
        break;

    case 0x60: case 0x61: case 0x62: case 0x63:
        instruction.type = OPQ;
        switch (instruction_buffer[0])
        {
        case 0x60:
                instruction.op = ADD;
                instruction.opcode = 0x60;
                break;
        case 0x61:
                instruction.op = SUB;
                instruction.opcode = 0x61;
                break;
        case 0x62:
                instruction.op = AND;
                instruction.opcode = 0x62;
                break;
        case 0x63:
                instruction.op = XOR;
                instruction.opcode = 0x63;
                break;
        }

        instruction.size = 2;
        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra > 0xE || instruction.rb > 0xE) {
                instruction.type = INVALID;
        }
        break;

    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76:
        instruction.type = JXX;
        switch (instruction_buffer[0])
        {
        case 0x70:
                instruction.jump = JMP;
                instruction.opcode = 0x70;
                break;
        case 0x71:
                instruction.jump = JLE;
                instruction.opcode = 0x71;
                break;
        case 0x72:
                instruction.jump = JL;
                instruction.opcode = 0x72;
                break;
        case 0x73:
                instruction.jump = JE;
                instruction.opcode = 0x73;
                break;
        case 0x74:
                instruction.jump = JNE;
                instruction.opcode = 0x74;
                break;
        case 0x75:
                instruction.jump = JGE;
                instruction.opcode = 0x75;
                break;
        case 0x76:
                instruction.jump = JG;
                instruction.opcode = 0x76;
                break;
        }
        instruction.size = 9;
        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.immVal = get8BytesFromArray(instruction_buffer, 1);
        break;

    case 0x80:
        instruction.type = CALL;
        instruction.opcode = 0x80;
        instruction.size = 9;
        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.immVal = get8BytesFromArray(instruction_buffer, 1);
        break;        

    case 0x90:
        instruction.type = RET;
        instruction.opcode = 0x90;
        instruction.size = 1;
        break;

    case 0xa0:
        instruction.type = PUSHQ;
        instruction.opcode = 0xa0;
        instruction.size = 2;
        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra > 0xE || instruction.rb != 0xF) {
          instruction.type = INVALID;
        }
        break;

    case 0xb0:
        instruction.type = POPQ;
        instruction.opcode = 0xb0;
        instruction.size = 2;
        num_of_bytes_fetched = fread(instruction_buffer+1, 1, instruction.size-1, inputStream);
        if (num_of_bytes_fetched < instruction.size-1){
          instruction.type = INVALID;
          break;
        }
        instruction.ra = instruction_buffer[1] >> 4 & 0x0F;
        instruction.rb = instruction_buffer[1] & 0x0F;
        if (instruction.ra > 0xE || instruction.rb != 0xF) {
          instruction.type = INVALID;
        }
        break;

    default:
        instruction.type = INVALID;
        break;
  }

  // handle invalid instruction
  if (instruction.type == INVALID){
    //if the would-be instruction is invalid, treat it as data    
    instruction.size = 8;
    // move file reading position to the beginning of the instruction and read the next 8 bytes if available
    fseek(inputStream, currAddr, SEEK_SET);
    num_of_bytes_fetched = fread(instruction_buffer, 1, instruction.size, inputStream);

    // if actual instruction is shorter than 8 bytes due to eof, update instruction_size
    instruction.size = num_of_bytes_fetched;
     
    // print invalid instruction
    char memVal[8*2+1] = {0};
    if ((num_of_bytes_fetched < 8) ||(num_of_bytes_fetched % 8) != 0 ){
        sprintf(memVal, "%02x", (unsigned char) instruction_buffer[0]);
        fprintf(out, "    %-8s0x%x         # %-22s\n", ".byte", instruction_buffer[0], memVal);
	currAddr += 1;//next byte 
        instruction.size = 1;//only read the first byte

    } else {
      for (int i = 0; i < 8; i++){
        sprintf(memVal+2*i, "%02x", (unsigned char) instruction_buffer[i]);
      }
      fprintf(out, "    %-8s0x%-8lx          # %-22s\n", ".quad", get8BytesFromArray(instruction_buffer, 0), memVal);
      
      currAddr += instruction.size;//update currAddr if invalid inst found
    }
  }

  return instruction;
}


// return the address of the next non-zero byte from current address in inputStream
long getAddrOfNextNonZeroByte (FILE* inputStream, long currAddr, long file_length){
  uint8_t read_val[1] = {0};
  while (currAddr < file_length) {
    fread(read_val, 1, 1, inputStream);
    if (read_val[0] != 0) break;
    currAddr += 1;
  }
  return currAddr;
}

// starting at given position of a byte array src, return the next 8 bytes as 
// a single integer in big or little endian as specified
uint64_t get8BytesFromArray (uint8_t* src, int startPos){
  uint64_t result = 0;
  //output is in BIG_ENDIAN
    for (int i = 0; i < 8; i++){
      result = result | ((uint64_t) src[startPos + i] << (8 * i));
    }
  
  return result;
}
