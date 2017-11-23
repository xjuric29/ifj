#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ilist.h"
#include "symtab.h"
#include "scanner.h"


struct I_output *Instr = NULL;

int add_instruction(int instType, token_t *op1, token_t *op2)
{
	
	
	static int inst_if = 0;
	static int inst_else = 0;
	static int inst_while = 0;
	static int inst_loop = 0;
	
	switch(instType)
	{
		case(LABEL):
			break;
		case(DEFVAR_TF):
			break;
		
		// TODO
	}


	return 0;	
}

int instr_init()
{
	Instr = malloc(sizeof(struct I_output) + INSTSIZE*sizeof(char*));
	if(Instr == NULL)
		return INTERNAL_ERROR;
	Instr->used_lines = 0;
	Instr->alloc_lines = INSTNUMBER;
	
/*	for(unsigned i = 0; i < Instr->alloc_lines; i++)
	{
		Instr->instrList[i] = (char*)malloc(INSTSIZE*sizeof(char));
		if(Instr->instrList[i] == NULL)
			return INTERNAL_ERROR;
		Instr->alloc_lines++;
	}
		
	Instr->instrList[	*/

	
	
	return 0;

}

void print_built_in()
{   

    /**** uvod ****/
    printf(".IFJcode17\njump Scope\n");

    /**** Length ****/
    printf("LABEL Length\npushframe\n");
    printf("defvar LF@%%retval\n");
    printf("strlen LF@%%retval LF@s\n");
    printf("popframe\nreturn\n");

    /**** SubStr ****/
    printf("LABEL SubStr\npushframe\n");
    printf("defvar LF@%%retval\n");
    printf("defvar LF@%%tmp\n");
    printf("move LF@%%tmp string@\n");
    printf("move LF@%%retval string@\n");
	printf("sub LF@i LF@i int@1\n");
    printf("defvar LF@test\n");
    printf("lt LF@test LF@i int@0\n");
    printf("jumpifeq $1subLOOP LF@test bool@true\n");
    printf("lt LF@test LF@n int@0\n");
    printf("jumpifeq $1subELSE LF@test bool@true\n");
	printf("createframe\n");
    printf("defvar TF@s\nMove TF@s LF@s\n");
    printf("call Length\n");
    printf("sub TF@%%retval TF@%%retval LF@i\n");
    printf("gt LF@test LF@n TF@%%retval\n");
	printf("add LF@n LF@n LF@i\n");
    printf("jumpifeq $1subELSE LF@test bool@false\n");
    printf("Move LF@n TF@%%retval\n");
	printf("add LF@n LF@n LF@i\n");
	printf("LABEL $1subELSE\n");
    printf("lt LF@test LF@i LF@n\n");
    printf("jumpifeq $1subLOOP LF@test bool@false\n");
    printf("getchar LF@%%tmp LF@s LF@i\n");
    printf("concat LF@%%retval LF@%%retval LF@%%tmp\n");
    printf("add LF@i LF@i int@1\n");
    printf("jump $1subELSE\n");
    printf("LABEL $1subLOOP\n");
    printf("LABEL $1subIF\n");
    printf("popframe\nreturn\n");

    /**** Asc ****/
    printf("LABEL Asc\npushframe\n");
    printf("defvar LF@%%retval\n");
	printf("Move LF@%%retval int@0\n");
	printf("defvar LF@%%tmp\n");
	printf("defvar LF@%%test\n");
	printf("sub LF@i LF@i int@1\n");
    printf("strlen LF@%%tmp LF@s\n");
	printf("lt LF@%%test LF@i int@0\n");
	printf("jumpifeq $1ascIF LF@%%test bool@true\n");
	printf("lt LF@%%test LF@i LF@%%tmp\n");
	printf("jumpifeq $1ascIF LF@%%test bool@false\n");
	printf("stri2int LF@%%retval LF@s LF@i\n");
	printf("LABEL $1ascIF\n");
    printf("popframe\nreturn\n");

    /**** Chr ****/
    printf("LABEL Chr\npushframe\n");
    printf("defvar LF@%%retval\n");
	printf("int2char LF@%%retval LF@i\n");
    printf("popframe\nreturn\n");
	
}
