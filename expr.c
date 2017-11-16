/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 * @todo
 */

// Header file
#include "expr.h"

// Module dependency
#include "stack.c"
//#include "scanner.c"

// Libraries
#include <stdio.h>

/**
 * @brief Precedental table determinating next action
 * "<" = shift
 * ">" = reduce
 * "=" = special shift for brackets
 * "#" = error
 * @todo Expand for more values (Now using only 6 from example in lecture)
 *       Should be in header or source file?
 */
char precTable[6][6] =
{  
//	 +    *    (    )    i    $
   {'>', '<', '<', '>', '<', '>'},	// +
   {'>', '>', '<', '>', '<', '>'},	// *
   {'<', '<', '<', '=', '<', '#'},	// (
   {'>', '>', '#', '>', '#', '>'},	// )
   {'>', '>', '#', '>', '#', '>'},	// i
   {'<', '<', '<', '#', '<', '#'}	// $
};
