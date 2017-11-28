/**
 * @file stack.h
 * @author Jiri Furda (xfurda00)
 * @brief Header file for stack interface
 * @todo
 */

#ifndef STACK_H
#define STACK_H


//	--- Includes ---
// Libraries
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
#define STACK_NOTFOUND -1       /// Return value representing "not found"
#define TERMINAL_COUNT  16       /// Number of all possible terminals

typedef enum
{
	ERR_STACK_NULL,
	ERR_STACK_FULL,
	ERR_STACK_EMPTY,
        ERR_STACK_SHIFT,
        ERR_STACK_TERM
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

//      --- Advanced functions ---
/**
 * @brief Get index of position with nearest terminal to the top of stack.
 * 
 * This is subfunction used by stackShiftPush().
 * 
 * @param stack Pointer to the stack
 * @return Index in the stack with closet terminal to the top of the stack
 */
int stackGetTerminalIndex(myStack_t *stack);

/**
 * @brief Shift right by one index all the items starting from 'index' untill top of the stack.
 * 
 * Of course this increase value of top of the stack index by one.
 * This is subfunction used by stackPushAtPos().
 * 
 * @param stack Pointer to the stack
 * @param index Starting index from where to start shifting (including this index)
 */
void stackRightShift(myStack_t *stack, int index);

/**
 * @brief Push character on certain position in the stack but do not override anything.
 * 
 * Shift all the items starting from 'index' untill top of the stack and then insert 'content' at the free space;
 * This is subfunction used by stackShiftPush().
 * 
 * @param stack Pointer to the stack
 * @param content Character to be written
 * @param index Where in the stack should be character inserted
 */
void stackPushAtPos(myStack_t *stack, char content, int pos);

/**
 * @brief Push expr shift character ('<') right after closest terminal to the end of the stack.
 * 
 * This function is called by expr_shif() in expr.c.
 * 
 * @param stack Pointer to the stack
 */
void stackShiftPush(myStack_t *stack);

/**
 * @brief Get closest terminal to the end of the stack.
 * 
 * This function is called by expr_algorithm() in expr.c.
 * 
 * @param stack Pointer to the stack
 * @return Terminal closest to the end of the stack
 */
char stackGetTerminal(myStack_t *stack);

// Module dependency
#include "expr.h"	// Need TERM_ constants
// Don't ask me why it has to be here... #IMPROVISE #ADAPT #OVERCOME

#endif
