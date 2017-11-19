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
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(...)
#endif


//	--- Includes ---
// Libraries
#include <stdio.h>

// Module dependency
#include "stack.c"
#ifndef EXPR_TEST
#include "scanner.c"
#endif



//	--- Constants ---
#define PREC_TABLE_SIZE 6	/// Defines size of precedent table (legal indexes are 0..SIZE-1)

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
	P_plus,		/// Plus "+" [int+int = int, int+double = double, double+int = double, double+double = double, str+str = str]
	P_mul,		///	Multiplication "*" [int*int = int, int*double = double, double*int = double, double*double = double]
	P_lBrac,	///	Left bracket "("
	P_rBrac,	/// Right bracket ")"
	P_id,		/// Identificator	(@todo Not sure)
	// Special
	P_stackEnd,		/// End of stack = "$"
	P_expr,		/// Expression = "E" (in rule)
	P_endingToken,	/// First token that doesn't belong to expression @warning This must be the last in this list!
	
	// For later use:
	P_minus,	///	Minus "-" [int-int = int, int-double = double, double-int = double, double-double = double]
	P_div,		/// Division "/" [int/int = double, int/double = double, double/int = double, double/double = double]
	P_intDiv,	/// Integer division "\" [int\int = int]
	// Logic operators
	P_equal,		/// Operator "="
    P_notEqual,		/// Operator "<>"
    P_less,			/// Operator "<"
    P_lessEqual,	/// Operator "<="
    P_greater,		/// Operator ">"
    P_greaterEqual	/// Operator ">="
}	precTableIndex_t;


//	--- Functions ---
/**
 * @brief Main function for evaluating expressions (logical or arithmectical) called by parser and then transfer token proceeding to this function
 * @param context	Determines if it's comparasion or assignment (@todo)
 * @param firstToken	First token of the expression
 * @param endToken	First token that doesn't belong to expression anymore (return token to parser)
 * @return 0 = no error, (@todo other errors)
 */
int expr_main(int context, token_t firstToken, token_t *endToken);

// @todo Comment these functions
int expr_doOperation(myStack_t *stack, tokenType_t token);
precTableAction_t expr_readTable(precTableIndex_t colIndex, precTableIndex_t rowIndex);
precTableIndex_t expr_getRowIndex(myStack_t *stack);
void expr_shift();
void expr_reduce();
void expr_specialshift();
void expr_error(char *msg);

#endif
