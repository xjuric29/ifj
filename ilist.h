
#ifndef ILIST_H
#define ILIST_H

#include "scanner.h"

#define FUNC -2
#define SCOPE -1

#define LABEL 0
#define DEFVAR_TF 1
#define DEFVAR_LF 2
#define CREATEFRAME 3
#define CALL 4
#define RETURN 5
#define PUSHS 6
#define POPS 7
#define CLEARS 8
#define ADDS 9
#define SUBS 10
#define MULS 11
#define DIVS 12
#define LTS 13
#define GTS 14
#define EQS 15
#define READ 16
#define WRITE 17

#define INSTNUMBER 1000
#define INSTSIZE 300


typedef struct I_output
{
	unsigned alloc_lines;
	unsigned used_lines;
	char *instrList[INSTSIZE];

} I_output;




extern struct I_output *Instr; //global list of instructions

int add_instruction(int instType, token_t *op1, token_t *op2); 

int instr_init(); //call from main

void print_built_int(); //call from main

#endif
