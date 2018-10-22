#ifndef _READ_H_
#define _READ_H_

#include <stdio.h>


struct Instruction readInstruction(FILE *, long);
char *getRegister(char);

#endif
