#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ilist.h"
#include "symtab.h"
#include "scanner.h"


void print_built_in();

//int main()
//{
//	instr_init();
//	token_t token;
//	string label;
//	strInit(&label);
//	token.value.stringVal = &label;
//	char *t1 = "SCOPE";
//	for(int i = 0; t1[i] != '\0'; i++)
//		strAddChar(&label, t1[i]);
//	add_instruction(LABEL, &token, NULL, NULL);
//	
//	strClear(&label);
//	t1 = "i";
//	for(int i = 0; t1[i] != '\0'; i++)
//		strAddChar(&label, t1[i]);
//	add_instruction(DEFVAR_TF, &token, NULL, NULL);
//	
//	add_instruction(SCOPE, NULL, NULL, NULL);
//	add_instruction(FUNC, &token, NULL, NULL);
//	
//	print_all();
//	//printf("%s\n", Instr->instrList[7]);
//	inst_free();
//	
//	strFree(&label);
//
//	return 0;
//}


struct I_output *Instr = NULL;

void inst_free()
{	
	for (unsigned i = 0; i < Instr->alloc_lines; i++)
		free(Instr->instrList[i]);

	free(Instr);


}

void print_all()
{
	print_built_in();
	
	for (unsigned i = 0; i < Instr->used_lines; i++)
		printf("%s\n", Instr->instrList[i]);
	
}

int add_instruction(int instType, token_t *op1, string *op2, token_t *op3)
{
	
	
	static int inst_endif = 0;
	static int inst_else = 0;
	static int inst_while = 0;
	static int inst_loop = 0;
	static int test = 0;	
	static I_context context = con_NONE;

	char c[100];	

	if(Instr->alloc_lines == (Instr->used_lines + 10))
	{
		Instr = realloc(Instr, sizeof(struct I_output) + Instr->alloc_lines*2*sizeof(char*));
		if(Instr == NULL)
			return INTERNAL_ERROR;
		Instr->alloc_lines*=2;
	}
	
	switch(instType)
	{
		case(LABEL):		
			strcpy(INST, "LABEL ");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			break;
		
		case(DEFVAR_TF):
			strcpy(INST, "DEFVAR TF@");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			break;
	
		case(DEFVAR_LF):
			strcpy(INST, "DEFVAR LF@");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			break;
		
		case(CREATEFRAME):
			strcpy(INST, "CREATEFRAME");
			strcat(INST, "\n");
			break;
			
		case(PUSHFRAME):
			strcpy(INST, "PUSHFRAME");
			strcat(INST, "\n");
			break;
		
		case(CALL):
			strcpy(INST, "CALL ");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;

		case(RETURN):
			strcpy(INST, "POPFRAME\n");
			Instr->used_lines++;
			strcpy(INST, "RETURN \n");
			break;

		case(PUSHS):
			strcpy(INST, "PUSHS ");
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, "LF@");
					strcat(INST, op1->value.stringVal->str);
					strcat(INST, "\n");
					break;
				
				case TOK_integer:
					strcat(INST, "int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					strcat(INST, "\n");
					break;

				case TOK_decimal:
					strcat(INST, "float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					strcat(INST, "\n");
					break;
				
				default:
					return INTERNAL_ERROR;
				
			}
			break;

		case(POPS):
			strcpy(INST, "POPS LF@");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;

		case(ADDS):
			strcpy(INST, "ADDS\n");
			break;

		case(SUBS):
			strcpy(INST, "SUBS\n");
			break;

		case(MULS):
			strcpy(INST, "MULS\n");
			break;

		case(DIVS):
			strcpy(INST, "DIVS\n");
			break;
	
		case(LTS):
			strcpy(INST, "LTS\n");
			break;

		case(GTS):
			strcpy(INST, "GTS\n");
			break;

		case(EQS):
			strcpy(INST, "EQS\n");
			break;
	
		case(READ):
			strcpy(INST, "READ LF@");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			break;

		case(FUNC):
			strcpy(INST, "LABEL ");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			Instr->used_lines++;
			add_instruction(PUSHFRAME, NULL, NULL, NULL);
			strcpy(INST, "DEFVAR LF@%%retval\n");
			Instr->used_lines--;
			break;

		case(SCOPE):
			strcpy(INST, "LABEL SCOPE");
			Instr->used_lines++;
			add_instruction(CREATEFRAME, NULL, NULL, NULL);
			add_instruction(PUSHFRAME, NULL, NULL, NULL);
			Instr->used_lines--;
			break;
		
		case(MOVE_LF_LF):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op2->str);
			strcat(INST, " LF@");
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;
				
				default:
					return INTERNAL_ERROR;
			}


			strcat(INST, "\n");
			break;
		
		case(MOVE_TF_LF):
			strcpy(INST, "MOVE TF@");
			strcat(INST, op2->str);
			strcat(INST, " LF@");			
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

				default:
					return INTERNAL_ERROR;
			}

			strcat(INST, "\n");
			break;

		case(MOVE_LF_TF):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op2->str);
			strcat(INST, " TF@");
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

				default:
					return INTERNAL_ERROR;
			}

			strcat(INST, "\n");
			break;

		case(MOVE):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{
				case TOK_integer:
					strcat(INST, "int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					strcat(INST, "float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

			}
			strcat(INST, "\n");
			break;
	

		case(RETVAL_IN):
			strcpy(INST, "MOVE LF@%retval LF@");
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;
				
				default:
					return INTERNAL_ERROR;
			}
			strcat(INST, "\n");
			break;

		case(RETVAL_OUT):
			strcpy(INST, "MOVE LF@");
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;
				
				default:
					return INTERNAL_ERROR;
			}
			strcat(INST, " TF@%retval\n");
			break;



		case(WHILE):
			context = con_WHILE;
			inst_while += 1;
			strcpy(INST, "LABEL ");
			sprintf(c, "%d", inst_while);
			strcat(INST, "$$");
			strcat(INST, c);
			strcat(INST, "$$WHILE\n");
			break;

		case(LOOP):
			context = con_NONE;
			
			strcpy(INST, "JUMP ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_while);
			strcat(INST, c);
			strcat(INST, "$$WHILE\n");
			Instr->used_lines++;
			
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_while - inst_loop);
			strcat(INST, c);
			strcat(INST, "$$LOOP\n");
			inst_loop += 1;
			break;

		case(IF):
			context = con_IF;
			inst_else += 1;
			return SUCCESS;
		
		case(ELSE):
			context = con_NONE;
			
			strcpy(INST, "JUMP ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_else - inst_endif);
			strcat(INST, c);
			strcat(INST, "$$ENDIF\n");			
			Instr->used_lines++;	
	
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_else - inst_endif);
			strcat(INST, c);
			strcat(INST, "$$ELSE\n");
			break;

		case(ENDIF):
			inst_endif++;
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_endif);
			strcat(INST, c);
			strcat(INST, "$$ENDIF\n");
			break;

		case(JUMPIFEQS):
			strcpy(INST, "JUMPIFEQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_else);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;
				
				case(con_WHILE):	
					sprintf(c, "%d", inst_loop);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;
				
				default:
					return INTERNAL_ERROR;
				
			}
			break;

		case(JUMPIFENQS):
			strcpy(INST, "JUMPIFENQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_else);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_loop);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			break;
				

		default:
			return INTERNAL_ERROR;



	
		// TODO
	}

	Instr->used_lines++;


	return SUCCESS;	
}

int instr_init()
{
	Instr = malloc(sizeof(struct I_output) + INSTNUMBER*sizeof(char*));
	if(Instr == NULL)
		return INTERNAL_ERROR;
	Instr->used_lines = 0;
	
	for(unsigned i = 0; i < INSTNUMBER; i++)
	{
		Instr->instrList[i] = (char*)malloc(INSTSIZE*sizeof(char));
		if(Instr->instrList[i] == NULL)
			return INTERNAL_ERROR;
		Instr->alloc_lines++;
	}
		
	
	Instr->alloc_lines = INSTNUMBER;
	
	
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