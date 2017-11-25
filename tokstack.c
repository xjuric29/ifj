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


int tokStack_Push(tokStack_t *stack, token_t token)
{
	// Check if stack is full
	if(tokStack_Full(stack) == TRUE)
	{
		tokStack_Error("tokStackInit: Stack is not allocated");
		return FAIL;
	}
	
	// Inicialize token to be pushed
	token_t *pushToken = TokenInit();
	if(pushToken == NULL)
	{
		tokStack_Error("tokStackPush: Couldn't init new token");
		return FAIL;
	}
	
	// Copy token values
	pushToken->type = token.type;
	pushToken->value = token.value;

	// Push token to the stack
	stack->tokArr[stack->top] = pushToken;
	
	// Increase top of the stack
	(stack->top)++;     
	
	// Return success
	return SUCCESS;
}


token_t* tokStack_Pop(tokStack_t *stack)
{
	// Get pointer to token on top of the stack
	token_t *topToken = tokStack_Top(stack);	
	
	// Check if it was successful
	if(topToken == NULL)	// If there is none token on top of the stack
		return NULL;	// No need to print error (It is already printed in tokStack_top())
		
	// Decrease top of the stack
	(stack->top)--;  
	
	// Return pointer to the token on top of the stack
	return topToken;
}

token_t* tokStack_Top(tokStack_t *stack)
{
	// Check if stack is empty
	if(tokStack_Empty(stack) == TRUE)      
	{
		tokStack_Error("tokStackPush: Tring to pop/top empty stack");
		return NULL;
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
