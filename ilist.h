
#ifndef ILIST_H
#define ILIST_H

#include "scanner.h"
#include "str.h"

#define FUNC -2 // + createframe
#define SCOPE -1 // + createframe pushframe

#define LABEL 0
#define DEFVAR_TF 1
#define DEFVAR_LF 2
#define CREATEFRAME 3
#define CALL 4
#define RETURN 5
#define PUSHS 6
#define POPS 7
#define CLEARS 8 // TODO
#define ADDS 9
#define SUBS 10
#define MULS 11
#define DIVS 12
#define LTS 13
#define GTS 14
#define EQS 15
#define LTEQS 16 // TODO
#define GTEQS 17 // TODO
#define JUMPIFEQS 18
#define JUMPIFENQS 19
#define JUMP 20
#define PUSHFRAME 21
#define READ 22
#define WRITE 23 // TODO
#define MOVE_LF_LF 24
#define MOVE_LF_TF 25
#define MOVE_TF_LF 26
#define WHILE 27
#define LOOP 28
#define IF 29
#define ELSE 30
#define ENDIF 31
#define RETVAL_OUT 32
#define RETVAL_IN 33
#define MOVE 34
#define RETVAL_POP 35
#define NOTS 36
#define INT2FLOAT 37
#define FLOAT2R2EINT 38 //TODO
#define RETVAL_INT2FLOAT_IN 39 //TODO
#define RETVAL_FLOAT2R2EINT_IN 40 //TODO
#define RETVAL_INT2FLOAT_OUT 41 //TODO
#define RETVAL_FLOAT2R2EINT_OUT 42 //TODO
#define INT2FLOATS 43
#define FLOAT2R2EINTS 44
#define CONCAT 45


#define INSTNUMBER 1000
#define INSTSIZE 300

#define INST (Instr->instrList[Instr->used_lines])

typedef struct I_output
{
	unsigned alloc_lines;
	unsigned used_lines;
	char *instrList[];

} I_output;


typedef enum
{
	con_IF,
	con_WHILE,
	con_NONE

} I_context;


extern struct I_output *Instr; //global list of instructions

int add_instruction(int instType, token_t *op1, string *op2, token_t *op3); 

int instr_init(); //call from main

void print_all(); //call from main

void inst_free(); //call at the end of program

#endif
