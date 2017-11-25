/**
 * @file expr.h
 * @author Jiri Furda (xfurda00)
 * @brief Header file for precedent analysis of expressions
 * @todo
 */

#ifndef EXPR_H
#define EXPR_H


//	--- Debugging ---
#ifdef DEBUG
#define DEBUG_PRINT     printf
#else
#define DEBUG_PRINT(...)
#endif


//	--- Includes ---
// Libraries
#include <stdio.h>
#include <stdlib.h>     // For postfix debug

// Module dependency
#include "stack.h"
#include "str.h"
#include "scanner.h"
#include "symtab.h"
#include "parser.h"
#include "ilist.h"
#include "tokstack.h"


//	--- Constants ---
// Internal values
#define PREC_TABLE_SIZE 15	/// Defines size of precedent table (legal indexes are 0..SIZE-1)
#define RULES_COUNT     13	/// Number of all the grammar rules used
#define EXPR_ERROR      -1   /// Internal return value for error
#define EXPR_SUCCESS    1  /// Intarnal return value for success
#define EXPR_TRUE       1  /// Intarnal return value for true
#define EXPR_FALSE      0  /// Intarnal return value for false
#define EXPR_RETURN_NOMORETOKENS       420    /// Internal return value respresenting loaded token doen't belog to expression
// External return values (@todo This is already defined somewhere for sure)
#define EXPR_RETURN_SUCC        0
#define EXPR_RETURN_ERROR_SYNTAX        2       // e.g. Missing operator/rule not found
#define EXPR_RETURN_ERROR_SEM   3       // e.g. Varibale not defined
#define EXPR_RETURN_ERROR_INTERNAL      99      // e.g. malloc fail


/**
 * @brief Enum structure for determinating next move of the algorith
 */
typedef enum
{
	ACTION_shift,
	ACTION_reduce,
	ACTION_specialShift,
	ACTION_ilegal	/// Trying to perform an ilegal action
}	precTableAction_t;


/**
 * @brief Enum structure for navigating in precedental table, []...legal combinations
 * @todo Expand for more values (Now using only 6 from example in lecture)
 */
typedef enum
{
	TERM_plus,		/// Plus "+" [int+int = int, int+double = double, double+int = double, double+double = double, str+str = str]
	TERM_minus,	/// Minus "-" [int-int = int, int-double = double, double-int = double, double-double = double]
	TERM_divInt,	/// Integer division "\" [int\int = int]
	TERM_mul,		/// Multiplication "*" [int*int = int, int*double = double, double*int = double, double*double = double]
	TERM_div,		/// Division "/" [int/int = double, int/double = double, double/int = double, double/double = double]
	TERM_lBrac,	/// Left bracket "("
	TERM_rBrac,	/// Right bracket ")"
	TERM_id,		/// Identificator	(@todo Not sure)
	// Special
	TERM_stackEnd,	/// End of stack = "$"
	
	// Logic operators
	TERM_equal,	/// Operator "="
	TERM_notEqual,	/// Operator "<>"
	TERM_less,		/// Operator "<"
	TERM_lessEqual,	/// Operator "<="
	TERM_greater,	/// Operator ">"
	TERM_greaterEqual,	/// Operator ">="
	
	// Special special
	TERM_expr,		/// Expression = "E" (in rule)
	TERM_endingToken	/// First token that doesn't belong to expression @warning This must be the last in this list!
}	precTableIndex_t;


//	--- Functions ---
/**
 * @brief Main function for evaluating expressions (logical or arithmectical) called by parser to transfer token proceeding to this module.
 * 
 * First of all this function create and initialize stack and other needed variables.
 * It's main function is to read all tokens and proceed them to expr_doOperation() untill it found the first one that doesn't belong to the expression.
 * When the non-expression token is found, it's sent back to parser to be proceed and this module is shut down.
 * 
 * Communicating with parser
 * When this function is called by parser, parser sends first token that belong to expression via pointer. 
 * When this module is shutting down, it puts first token that DOESN'T belong to the expression back to the pointer.
 * 
 * @param *parserToken	Pointer to token used for communicating with parser
 * @return (@todo)
 */
int expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *variable);


/**
 * @brief Core function for expression token processing.
 * 
 * This does all the magic stuff that happens in this module. It's first step is to find out precedent table indexes.
 * That is done using converting external token types (defined in scanner.h) to internal ones used in this module (e.g. TERM_plus), that gives us the column index.
 * The row index is based on character on top of the stack, therefore it's gained using expr_getIndexFromChar() function.
 * Now we have all required informations to check the precedent table to find out our next move, this is done by calling expr_readTable() function.
 * Based on the result we call corresponding function, that means one of these: expr_shift / expr_reduce / expr_specialshift / expr_error
 * 
 * @warning TOK_endOfFile doesn't represent it's original purpose but here it symbolizes there are no more tokens to load that would belong to the expression
 * 
 * @param *stack	Pointer to stack for the algorithm
 * @param tokenToken	Type of now proceessed token
 * @return 1 = no error (@todo)
 */
int expr_algorithm(myStack_t *stack, token_t tokenType, int context);


/**
 * @brief Find action for corresponding row (character on top of the stack) and column (loaded token type).
 * 
 * Does nothing unexcepted, if you are not dumb it tells you what is the next move for the algrithm.
 * But if you ask for some fantasy stuff it punches you in the face.
 * 
 * @param row	Index corresponding to character on top of the stack
 * @param col	Index corresponding to loaded token
 * @return next action of the algorithm
 */
precTableAction_t expr_readTable(precTableIndex_t rowIndex, precTableIndex_t colIndex);


/**
 * @brief Convert char to index
 * 
 * Find corresponding index in the precedent table for the character paramater.
 * 
 * @param character		Char you want index for
 * @return Index in precedent table
 */
precTableIndex_t expr_getIndexFromChar(char character);


/**
 * @brief  Convert index to char
 * 
 * Find corresponding character based on the index in the precedent table.
 * 
 * @param index		Index in precedent table you want corresponding char for
 * @return Real character
 */
char expr_getCharFromIndex(precTableIndex_t index);

// @todo Comment these functions
int expr_shift(myStack_t *stack, char character);

/**
 * @brief Algortihm operation reduce
 * 
 * @todo Description
 * 
 * @param *stack	Pointer to stack for the algorithm 
 * @return      EXPR_RETURN_SUCC        when successful
 *              EXPR_RETURN_ERROR_INTERNAL      when couldn't  add char to handle string
 *              EXPR_RETURN_ERROR_SYNTAX        when rule not found
 */
int expr_reduce(myStack_t *stack, token_t token);
int expr_specialShift(myStack_t *stack, char character);
int expr_searchRule(string handle);
int expr_isAlgotihmFinished(myStack_t *stack, int tokenType);  // For successful end there should be only "$E" in the stack
void expr_generateInstruction(char terminal, token_t token);
int expr_generateResult(int context, st_element_t *variable);
void expr_testFinish_retVal(int retVal);

/**
 * @brief Check if token be used as begining of expression
 * @param firstToken    Token structure to be checked
 * @return EXPR_TRUE if can be first, EXPR_FALSE if cannot
 */
int expr_isFirstValid(token_t firstToken);

/**
 * @brief Prints an error to stderr
 * @todo Peacefully end the module
 * @param msg	Message for debugging (without new line at the end)
 */
void expr_error(char *msg);
#endif
