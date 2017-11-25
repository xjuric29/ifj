/**
 * @file stacktok.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for token stack
 * @todo
 */

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

	// Push token to the stack
	stack->tokArr[stack->top] = tokenType;
	
	// Increase top of the stack
	(stack->top)++;     
	
	// Return success
	return SUCCESS;
}


tokenType_t tokStack_Pop(tokStack_t *stack)
{
	// Get pointer to token on top of the stack
	tokenType_t topType = tokStack_Top(stack);	
		
	// Decrease top of the stack
	(stack->top)--;  
	
	// Return pointer to the token on top of the stack
	return topType;
}

tokenType_t tokStack_Top(tokStack_t *stack)
{
	// Check if stack is empty
	if(tokStack_Empty(stack) == TRUE)      
	{
		tokStack_Error("tokStackPush: Tring to pop/top empty stack");
		return; // @todo
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
