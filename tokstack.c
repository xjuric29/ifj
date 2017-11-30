/**
 * @file stacktok.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for token stack
 * @todo
 */

//#define TOKSTACKDEBUG
#include "tokstack.h"



int tokStack_Init(tokStack_t *stack)
{
	if(stack == NULL)   // If the stack is not allocated
	{
		tokStack_Error("tokStackInit: Stack is not allocated");
		return FAIL;
	}
	
	stack->top = NO_STACK;    // Initialize top of the stack
}


int tokStack_Push(tokStack_t *stack, tokenType_t tokenType)
{
	// Check if stack is full
	if(tokStack_Full(stack) == TRUE)
	{
		tokStack_Error("tokStackInit: Stack is not allocated");
		return FAIL;
	}

	// Increase top of the stack
	(stack->top)++;  

	// Push token to the stack
	stack->tokArr[stack->top] = tokenType;   
	
	// Debug
	#ifdef TOKSTACKDEBUG
	tokStack_Info(stack);
	#endif
	
	// Return success
	return SUCCESS;
}


tokenType_t tokStack_Pop(tokStack_t *stack)
{
	// Get pointer to token on top of the stack
	tokenType_t topType = tokStack_Top(stack);	
	if(topType == TOK_FAIL)
		return TOK_FAIL;
		
	// Decrease top of the stack
	(stack->top)--;  
	
	// Debug
	#ifdef TOKSTACKDEBUG
	tokStack_Info(stack);
	#endif
	
	// Return pointer to the token on top of the stack
	return topType;
}

tokenType_t tokStack_Top(tokStack_t *stack)
{
	// Check if stack is empty
	if(tokStack_Empty(stack) == TRUE)      
	{
		tokStack_Error("tokStackPush: Tring to pop/top empty stack");
		return TOK_FAIL; // @todo checking whne called this function!!!!!!
	}	
		
	// Return pointer to token on top of the stack
	return stack->tokArr[stack->top];
}

int tokStack_Empty(tokStack_t *stack)
{
	if(stack->top < 0)
		return TRUE;
	else
		return FALSE;	
}


int tokStack_Full(tokStack_t *stack)
{
	if(stack->top >= TOKSTACK_MAX)
		return TRUE;
	else
		return FALSE;
}

void tokStack_Error(char* msg)
{
		fprintf(stderr, "[ERROR] %s\n", msg);
}

#ifdef TOKSTACKDEBUG
void tokStack_Info(tokStack_t *stack)
{
	printf("[DBG] tokStack=");
	for(int i = 0; i <= stack->top; i++)
	{
		switch(stack->tokArr[i])
		{
			case TOK_integer:	printf("int ");	break;
			case TOK_decimal:	printf("dec ");	break;
			case TOK_string:	printf("str ");	break;
			case TOK_BOOLEAN:	printf("bool ");	break;
			default:	printf("WTF ");	break;
		}
	}
	printf("\n");
}
#endif
