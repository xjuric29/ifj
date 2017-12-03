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

char *convert_string(char *str)
{
	int len = 0;
	size_t max_len = 2*strlen(str);
	char c[5];
	char *result = malloc(max_len);
	result[0] = '\0';
	if(result == NULL)
		return NULL;
	for(unsigned i = 0; str[i] != '\0'; i++)
	{
		if(i + 5 > max_len)
		{
			max_len *= 2;
			result = realloc(result, max_len);
			if(result == NULL)
				return NULL;
		}
		
		if((str[i] >= 0 && str[i] <= 32) || str[i] == 35 || str[i] == 92 || str[i] == 34)
		{
			sprintf(c, "%d", str[i]);
			result[len] = 92;
			len++;
			result[len] = '0';
			len++;
			strcpy(&result[len], c);
			len += 2;
		}

		else
		{
			result[len] = str[i];
			len++;
		}

	}
	result[len] = '\0';
	//printf("****************recieved: %s ****************\n", str);
	//printf("****************returned: %s ****************\n", result);
	return result;
}

void addInstComment(char *comment)
{
	if(strlen(comment) >= INSTSIZE)
		return;
	strcpy(INST, "#************** %s **************\n");
	strcat(INST, comment);
	Instr->used_lines++;
	return;
}



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
	
	
	static int inst_if = 0;
	static int inst_else = 0;
	static int inst_while = 0;
	static int inst_loop = 0;
	static int test = 0;	
	static I_context context = con_NONE;

	char c[100];	

	if(Instr->alloc_lines <= (Instr->used_lines + 15))
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
			if(op1 != NULL)
			{
				strcat(INST, op1->value.stringVal->str);
				strcat(INST, "\n");
			}

			else
			{
				strcat(INST, op2->str);
				strcat(INST, "\n");
			}
			break;
	
		case(DEFVAR_LF):
			if(op1 != NULL)
			{
				strcpy(INST, "DEFVAR LF@");
				strcat(INST, op1->value.stringVal->str);
				strcat(INST, "\n");
			}

			else
			{
				strcat(INST, op2->str);
				strcat(INST, "\n");
			}
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
			if(op1 != NULL)
			{
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

					case TOK_string:
						strcat(INST, "string@");
						char *tmp = convert_string(op1->value.stringVal->str);
						strcat(INST, tmp);
						free(tmp);
						break;
				
					default:
						return INTERNAL_ERROR;
				
				}
			}

			else
			{
				strcat(INST, "LF@");
				strcat(INST, op2->str);
				strcat(INST, "\n");				
			}

			break;


		case(POPS):
			strcpy(INST, "POPS LF@");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;

		case(RETVAL_POP):
			strcpy(INST, "POPS LF@%retval\n"); 
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

		case(NOTS):
			strcpy(INST, "NOTS\n");
			break;
	
		case(INT2FLOATS):
			strcpy(INST, "INT2FLOATS\n");
			break;

		case(FLOAT2R2EINTS):
			strcpy(INST, "FLOAT2R2EINTS\n");
			break;	
	
		case(READ):
			strcpy(INST, "READ LF@");
			strcat(INST, op1->value.stringVal->str);
			switch(op1->type)
			{
				case KW_integer:
					strcat(INST, " int");
					break;
				
				case KW_double:
					strcat(INST, " float");
					break;

				case KW_string:
					strcat(INST, " string");
					break;

				default:
					return INTERNAL_ERROR;
			}
			strcat(INST, "\n");
			
			break;

		case(FUNC):
			strcpy(INST, "LABEL ");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, "\n");
			Instr->used_lines++;
			strcpy(INST, "PUSHfRAME\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@%retval\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$int\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$dec\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$str\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$test\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$test bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$str2\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$str string@\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$str2 string@\n");
			break;

		case(SCOPE):
			strcpy(INST, "\nLABEL Scope\n");
			Instr->used_lines++;
			strcpy(INST, "CREATEfRAME\n");
			Instr->used_lines++;
			strcpy(INST, "PUSHfRAME\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$int\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$dec\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$str\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$test\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$test bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "DEFVAR LF@$str2\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$str string@\n");
			Instr->used_lines++;
			strcpy(INST, "MOVE LF@$str2 string@\n");
			break;
		
		case(MOVE_LF_LF):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, " LF@");
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:	
					strcat(INST, " int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					strcat(INST, " float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

				case TOK_string:
					strcat(INST, " string@");
					char *tmp = convert_string(op1->value.stringVal->str);
					strcat(INST, tmp);
					free(tmp);
					break;
				
				default:
					return INTERNAL_ERROR;
			}


			strcat(INST, "\n");
			break;
		
		case(MOVE_TF_LF):
			strcpy(INST, "MOVE TF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, " LF@");			
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					strcat(INST, " int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					strcat(INST, " float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

				case TOK_string:
					strcat(INST, " string@");
					char *tmp = convert_string(op1->value.stringVal->str);
					strcat(INST, tmp);
					free(tmp);
					break;

				default:
					return INTERNAL_ERROR;
			}

			strcat(INST, "\n");
			break;

		case(MOVE_LF_TF):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{	
				case TOK_identifier:
					strcat(INST, " TF@");
					strcat(INST, op1->value.stringVal->str);
					break;
				
				case TOK_integer:
					strcat(INST, " int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					break;

				case TOK_decimal:
					strcat(INST, " float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					break;

				case TOK_string:
					strcat(INST, " string@");
					char *tmp = convert_string(op1->value.stringVal->str);
					strcat(INST, tmp);
					free(tmp);

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
				case KW_integer:
					strcat(INST, " int@0");
				//	sprintf(c, "%d", op1->value.integer);
				//	strcat(INST, c);
					break;

				case KW_double:
					strcat(INST, " float@0.0");
				//	sprintf(c, "%g", op1->value.decimal);
				//	strcat(INST, c);
					break;

				case KW_string:
					strcat(INST, " string@");
					//sprintf(c, "%d", op1->value.stringVal);
					//strcat(INST, c);
					break;

				default:
					return INTERNAL_ERROR;

			}
			strcat(INST, "\n");
			break;

	

		case(RETVAL_IN):
			strcpy(INST, "MOVE LF@%retval ");
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, "LF@");
					strcat(INST, op1->value.stringVal->str);
					break;
				
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

				case TOK_string:
					strcat(INST, " string@");
					char *tmp = convert_string(op1->value.stringVal->str);
					strcat(INST, tmp);
					free(tmp);
					break;
				
				case KW_double:
					strcat(INST, "float@0.0\n");
					break;

				case KW_integer:
					strcat(INST, "int@0\n");
					break;

				case KW_string:
					strcat(INST, "string@\n");
					break;
				
				default:
					return INTERNAL_ERROR;
			}
			strcat(INST, "\n");
			break;

		case(RETVAL_OUT):
			strcpy(INST, "MOVE LF@");
			strcat(INST, op1->value.stringVal->str);
			strcat(INST, " TF@%retval\n");
			break;

		case(WHILE):
			context = con_WHILE;
			inst_while = op1->value.integer;
			strcpy(INST, "LABEL ");
			sprintf(c, "%d", inst_while);
			strcat(INST, "$$");
			strcat(INST, c);
			strcat(INST, "$$WHILE\n");
			break;

		case(LOOP):
			context = con_NONE;
			inst_while = op1->value.integer;
			
			strcpy(INST, "JUMP ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_while);
			strcat(INST, c);
			strcat(INST, "$$WHILE\n");
			Instr->used_lines++;
			
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", inst_while);
			strcat(INST, c);
			strcat(INST, "$$LOOP\n");
			inst_loop += 1;
			break;

		case(IF):
			context = con_IF;
			inst_if = op1->value.integer;
			return SUCCESS;
		
		case(ELSE):
			context = con_NONE;
			
			strcpy(INST, "JUMP ");
			strcat(INST, "$$");
			sprintf(c, "%d", op1->value.integer);
			strcat(INST, c);
			strcat(INST, "$$ENDIF\n");			
			Instr->used_lines++;	
	
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", op1->value.integer);
			strcat(INST, c);
			strcat(INST, "$$ELSE\n");
			break;

		case(ENDIF):
			strcpy(INST, "LABEL ");
			strcat(INST, "$$");
			sprintf(c, "%d", op1->value.integer);
			strcat(INST, c);
			strcat(INST, "$$ENDIF\n");
			break;

		case(JUMPIFEQS):
			strcpy(INST, "PUSHS bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "JUMPIFEQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;
				
				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;
				
				default:
					return INTERNAL_ERROR;
				
			}
			break;

		case(LTEQS):
			strcpy(INST, "JUMPIFENQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			Instr->used_lines++;
			strcpy(INST, "LTS\n");
			Instr->used_lines++;
			strcpy(INST, "PUSHS bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "JUMPIFEQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			break;

		case(GTEQS):
			strcpy(INST, "JUMPIFENQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			Instr->used_lines++;
			strcpy(INST, "GTS\n");
			Instr->used_lines++;
			strcpy(INST, "PUSHS bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "JUMPIFEQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			break;
	

		case(JUMPIFENQS):
			strcpy(INST, "PUSHS bool@false\n");
			Instr->used_lines++;
			strcpy(INST, "JUMPIFENQS ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;

				case(con_WHILE):	
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			break;
				
		case(RETVAL_INT2FLOAT_OUT):
			strcpy(INST, "INT2FLOAT LF@");
			strcat(INST, op2->str);
			strcat(INST, " TF@%retval");
			break;

		case(RETVAL_FLOAT2R2EINT_OUT):
			strcpy(INST, "FLOAT2R2EINT LF@");
			strcat(INST, op2->str);
			strcat(INST, " TF@%retval");
			break;

		case(RETVAL_INT2FLOAT_IN):
			strcpy(INST, "INT2FLOAT LF@%retval");
			strcat(INST, " LF@");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;

		case(RETVAL_FLOAT2R2EINT_IN):
			strcpy(INST, "FLOAT2R2EINT LF@%retval");
			strcat(INST, " LF@");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;

		case(INT2FLOAT):
			strcpy(INST, "INT2FLOAT TF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, " LF@");
					strcat(INST, op1->value.stringVal->str);
					break;

				case TOK_integer:
					strcat(INST, " int@");
					sprintf(c, "%d", op1->value.integer);
					strcat(INST, c);
					strcat(INST, "\n");
					break;

				default:
					return INTERNAL_ERROR;

			}
			strcat(INST, "\n");
			break;
		
		case(FLOAT2R2EINT):
			strcpy(INST, "FLOAT2R2EINT TF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, " LF@");
					strcat(INST, op1->value.stringVal->str);
					break;

				case TOK_decimal:
					strcat(INST, " float@");
					sprintf(c, "%g", op1->value.decimal);
					strcat(INST, c);
					strcat(INST, "\n");
					break;

				default:
					return INTERNAL_ERROR;

			}
			strcat(INST, "\n");
			break;

		case(CONCAT):
			strcpy(INST, "CONCAT LF@");
			strcat(INST, op2->str);
			strcat(INST, " LF@");
			strcat(INST, op2->str);
			switch(op1->type)
			{
				case TOK_identifier:
					strcat(INST, " LF@");
					strcat(INST, op1->value.stringVal->str);
					break;
	
				case TOK_string:
					strcat(INST, " string@");
					char *tmp = convert_string(op1->value.stringVal->str);
					strcat(INST, tmp);
					free(tmp);
					break;

				default:
					return INTERNAL_ERROR;
			}
			strcat(INST, "\n");

			break;

		case(WRITE):
			strcpy(INST, "WRITE LF@");
			strcat(INST, op2->str);
			strcat(INST, "\n");
			break;
		
		case(LT):
			strcpy(INST, "LT LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			break;

		case(GT):
			strcpy(INST, "GT LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			break;

		case(EQ):
			strcpy(INST, "EQ LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			break;

		case(NOTEQ):
			strcpy(INST, "EQ LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			Instr->used_lines++;
			strcpy(INST, "NOTS\n");
			break;

		case(LTEQ):
			strcpy(INST, "JUMPIFNEQ ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;
				
				case(con_WHILE):
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			Instr->used_lines++;
			strcpy(INST, "LT LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			break;
		
		case(GTEQ):
			strcpy(INST, "JUMPIFNEQ ");
			strcat(INST, "$$");
			switch(context)
			{
				case(con_IF):
					sprintf(c, "%d", inst_if);
					strcat(INST, c);
					strcat(INST, "$$ELSE\n");
					break;
				
				case(con_WHILE):
					sprintf(c, "%d", inst_while);
					strcat(INST, c);
					strcat(INST, "$$LOOP\n");
					break;

				default:
					return INTERNAL_ERROR;
			}
			Instr->used_lines++;
			strcpy(INST, "GT LF@$test LF@$str LF@$str2\nPUSHS LF@$test\n");
			break;

		case(FLOAT2INTS):
			strcpy(INST, "FLOAT2INTS\n");
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
    printf("LABEL length\npushframe\n");
    printf("defvar LF@%%retval\n");
    printf("strlen LF@%%retval LF@s\n");
    printf("popframe\nreturn\n");

    /**** SubStr ****/
    printf("LABEL substr\npushframe\n");
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
    printf("LABEL asc\npushframe\n");
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
    printf("LABEL chr\npushframe\n");
    printf("defvar LF@%%retval\n");
	printf("int2char LF@%%retval LF@i\n");
    printf("popframe\nreturn\n");
	
}
