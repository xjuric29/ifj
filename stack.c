/**
 * @file stack.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for stack interface
 * @todo
 */

#include "stack.h"


void stackInit(myStack_t *stack)
{
	if(stack == NULL)   // If the stack is not allocated
	{
		stackError(ERR_STACK_NULL);
		return;
	}
	
	stack->top = -1;    // Initialize top of the stack
	stackPush(stack, STACK_ENDCHAR);	// Push "end of stack character" as first one
		
	stackInfo(stack);
}
	
	
void stackPush(myStack_t *stack, char content)
{
	if(stackFull(stack))        // If the stack is full
	{
		stackError(ERR_STACK_FULL);  // Throw an error
		return;
	}
	
	(stack->top)++;     // Increase index of the top of the stack
	stack->arr[stack->top] = content;     // Add value to the stack	
	
}


char stackPop(myStack_t *stack)
{
	if(!stackEmpty(stack))      // If the stack is not empty
	{
		(stack->top)--;     // Decrease index of top of the stack
		return stack->arr[stack->top+1];	// Return content from top of the stack
	}
	else
	{
		stackError(ERR_STACK_EMPTY);  // Throw an error
		return STACK_NULLCHAR;
	}
}


char stackTop(myStack_t *stack)
{
	if(!stackEmpty(stack))      // If the stack is not empty
	{
		return stack->arr[stack->top];	// Return content from top of the stack
	}	
	else
	{
		stackError(ERR_STACK_EMPTY);  // Throw an error
		return STACK_NULLCHAR;
	}
}


void stackError(stackErrorCodes_t code)
{
	char *msg;
	
	switch(code)
	{
			case ERR_STACK_NULL:	msg="Tryied to init not allocated stack";	break;
			case ERR_STACK_FULL:	msg="Tryied to push onto full stack";	break;			
			case ERR_STACK_EMPTY:	msg="Tryied to top/pop empty stack";	break;
			default:	msg="Undefined error";	break;
	}
	
	fprintf(stderr, "stack.c: %s\n", msg);
}


int stackEmpty(myStack_t *stack)
{
	if(stack->top < 0)
		return 1;
	else
		return 0;	
}


int stackFull(myStack_t *stack)
{
	if(stack->top >= STACK_MAX)
		return 1;
	else
		return 0;
}

void stackInfo(myStack_t *stack)
{
	if(stack == NULL)   // If the stack is not allocated
	{
		stackError(ERR_STACK_NULL);
		return;
	}
	
	
	printf("[DBG] Stack(top=%d)='", stack->top);
	for(int i=0; i<=stack->top; i++)
	{
			printf("%c",stack->arr[i]);
	}
	printf("'\n");

	stack->top = 0;    // Initialize top of the stack	
}
