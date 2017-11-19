/**
 * @file stack.h
 * @author Jiri Furda (xfurda00)
 * @brief Header file for stack interface
 * @todo
 */

#ifndef STACK_H
#define STACK_H

//	--- Includes ---
#include <stdio.h>


//	--- Debuging ---
#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(...)
#endif


//	--- Constants ---
#define NO_STACK -1	/// Top index for not inicialised stack
#define STACK_MAX 420	///	@todo Find optial value
#define STACK_ENDCHAR '$'	/// Character representing end of stack
#define STACK_NULLCHAR 0	/// Character representing no value (returned when tried to pop/top empty stack)

typedef enum
{
	ERR_STACK_NULL,
	ERR_STACK_FULL,
	ERR_STACK_EMPTY,
}	stackErrorCodes_t; 


//	--- Strucutres ---
typedef struct {
	char arr[STACK_MAX];
	int top;
} myStack_t;


//	--- Functions ---
void stackInit(myStack_t *stack);
void stackPush(myStack_t *stack, char content);
char stackPop(myStack_t *stack);
char stackTop(myStack_t *stack);
void stackError(stackErrorCodes_t code);
int stackEmpty(myStack_t *stack);
int stackFull(myStack_t *stack);
void stackInfo(myStack_t *stack);

#endif
