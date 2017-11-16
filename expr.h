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

//	--- Constants ---
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
	P_i,		/// Identificator	(@todo Not sure)
	P_end,		/// End of stack = "$"
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

#endif
