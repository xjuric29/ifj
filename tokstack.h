/**
 * @file stacktok.h
 * @author Jiri Furda (xfurda00)
 * @brief Header file for token stack
 * @todo
 */

#ifndef TOKSTACK_H
#define TOKSTACK_H


//	--- Includes ---
// Libraries
#include <stdio.h>
// Module dependency
#include "scanner.h" // token_t structure
#include "parser.h" // Token init and free


//	--- Constants ---
#define NO_STACK -1	/// Top index for not inicialised stack
#define TOKSTACK_MAX 420	///	@todo Find optial value
#define SUCCESS 0
#define FAIL 1
#define TRUE 1
#define FALSE 0
#define TOK_FAIL	TOK_endOfFile	/// Representing error return with type tokenType_t


//	--- Strucutres ---
typedef struct {
	tokenType_t tokArr[TOKSTACK_MAX];
	int top;
} tokStack_t;


//	--- Functions ---
int tokStack_Init(tokStack_t *stack);
int tokStack_Push(tokStack_t *stack, tokenType_t token);
tokenType_t tokStack_Pop(tokStack_t *stack);
tokenType_t tokStack_Top(tokStack_t *stack);
int tokStack_Empty(tokStack_t *stack);
int tokStack_Full(tokStack_t *stack);
void tokStack_Error(char* msg);

#endif
