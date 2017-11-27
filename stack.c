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
		
	//stackInfo(stack);
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
			case ERR_STACK_NULL:	msg="Tried to init not allocated stack";	break;
			case ERR_STACK_FULL:	msg="Tried to push onto full stack";	break;			
			case ERR_STACK_EMPTY:	msg="Tried to top/pop empty stack";	break;
                        case ERR_STACK_SHIFT:	msg="Tried to shift full stack";	break;
                        case ERR_STACK_TERM:	msg="Terminal not found";	break;
			default:	msg="Undefined error";	break;
	}
	
	fprintf(stderr, "[ERROR] stack.c: %s\n", msg);
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
#ifdef DEBUG
	if(stack == NULL)   // If the stack is not allocated
	{
		stackError(ERR_STACK_NULL);
		return;
	}
	
	
	//printf("[DBG] Stack(top=%d)='", stack->top);
        printf("[DBG] STACK='");
	for(int i=0; i<=stack->top; i++)
	{
			printf("%c",stack->arr[i]);
	}
	printf("'\n");
#endif
}

int stackGetTerminalIndex(myStack_t *stack)
{
	char terminals[TERMINAL_COUNT] = "+-\\*/()i$";
	// @todo Temporary logic solution
	terminals[9] = TERM_equal;
	terminals[10] = TERM_notEqual;
	terminals[11] = TERM_less;
	terminals[12] = TERM_lessEqual;
	terminals[13] = TERM_greater;
	terminals[14] = TERM_greaterEqual;
        terminals[15] = TERM_string;
	
        for(int i = stack->top; i >= 0; i--)    // Start searching from top of the stack
        {
                for(int x = 0; x < TERMINAL_COUNT; x++) // Compare with every possible terminal
                {
                        if(stack->arr[i] == terminals[x])       // If it's a match
                                return i;       // Return terminal index
                }
        }
        
        stackError(ERR_STACK_TERM);
        return STACK_NOTFOUND;
}

void stackRightShift(myStack_t *stack, int index)
{
        if(stackFull(stack))   // Shouldn't ever happen because it's checked in stackPushAtPos but just in case
	{
		stackError(ERR_STACK_SHIFT);
		return;
	}        
        
        char prev = STACK_NULLCHAR;     // First index will be "empty"
        for(; index <= stack->top+1; index++)      // For every item in stack + one more
        {
                char tmp = stack->arr[index];   // Temporary save value
                stack->arr[index] = prev;       // Move value from previous item
                prev = tmp;     // Store value that used to be in this place for next run of the cycle
        }
        
        (stack->top)++; // Increase top of the stack
}

void stackPushAtPos(myStack_t *stack, char content, int pos)
{
        if(pos == (stack->top) + 1)     // There's no need to shift
        {
                stackPush(stack, content);      // Use regular push function
                return;
        }
        
	if(stackFull(stack))        // If the stack is full
	{
		stackError(ERR_STACK_FULL);  // Throw an error
		return;
	}
	
        stackRightShift(stack, pos);    // Make space at specified position (shift everything right by one starting from that poisiton untill top of the stack)
        stack->arr[pos] = content;     // Add value to the stack at specified position	
}

void stackShiftPush(myStack_t *stack)
{
        int index = stackGetTerminalIndex(stack) + 1;      // Index to position right after closest terminal
        
        stackPushAtPos(stack, '<', index);
}

char stackGetTerminal(myStack_t *stack)
{
        int index = stackGetTerminalIndex(stack);       // Get index of the nearest terminal to the end of the stack
        
        if(index == STACK_NOTFOUND)      // If not found
                return STACK_NULLCHAR;
                
        return stack->arr[index];       // Return terminal
}

