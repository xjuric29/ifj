/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 * @todo
 */
 
// --- TESTING ---
#define EXPR_TEST
#ifdef EXPR_TEST
#include "tests/expr/expr-test.c"
#endif


// Header file
#include "expr.h"






/**
 * @brief Precedental table determinating next action
 * "<" = shift
 * ">" = reduce
 * "=" = special shift for brackets
 * "#" = error
 * @todo Expand for more values (Now using only 6 from example in lecture)
 *       Should be in header or source file?
 */
char precTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] =
{  
//	 +    *    (    )    i    $
   {'>', '<', '<', '>', '<', '>'},	// +
   {'>', '>', '<', '>', '<', '>'},	// *
   {'<', '<', '<', '=', '<', '#'},	// (
   {'>', '>', '#', '>', '#', '>'},	// )
   {'>', '>', '#', '>', '#', '>'},	// i
   {'<', '<', '<', '#', '<', '#'}	// $
};





// ========== DEBUG ==========

#ifdef EXPR_TEST
int main()
{
	token_t endToken;
	
	token_t id;
	id.type = TOK_identifier;
	
	expr_main(0, id, &endToken);
	
	DEBUG_PRINT("[DBG] Ending token type = %d\n", endToken.type);
	return 0;
}
#endif





int expr_main(int context, token_t firstToken, token_t *endToken)
{
	// --- Initializing stack and default values ---
	myStack_t stack;	// Create stack
	stackInit(&stack);	// Init stack (Push "$")
	
	int continueLoading = 1;	// Determines if this module should read next token
	
	token_t loadedToken;
	loadedToken = firstToken;	// Value for first run of loading cycle

	// --- Loading tokens ---
	do
	{
		DEBUG_PRINT("[DBG] Loading token (external type=%d)\n", loadedToken.type);
		
		// CORE OF THE FUNCTION
		int type;	// Internal type of token
		type = expr_doOperation(&stack, loadedToken.type);	// Perform operation based on loaded token
		
		
		// End of expression (found token that doesn't belong to expression anymore
		if(type == P_endingToken)
		{
			// @todo Later if it's going to be more advanced, put it in reutrnProceeding() but for now:
			continueLoading = 0;	// Stop the loading cycle
			*endToken = loadedToken;	// Save token for parser to proceed
			// @todo Do something with the result and try to write this shit again like a human
			return 0;	// Return success	
		}
		
		
		// Get next token for next run of the cycle
		#ifndef DEBUG
		getToken(&loadedToken);
		#else
		TEST_getToken(&loadedToken);
		#endif
		
	}
	while(continueLoading);
	
	return -1;	// @todo Return values, this is here just because gcc won't stop bitching about it (If program gets to this line it means there is some strange smell of insect in the air)
}

int expr_doOperation(myStack_t *stack, tokenType_t token)
{
	precTableIndex_t type;	// Internal type of token = Column index
	
	// Getting column index (based on external type of token (tokenType_t from scanner.h))
	switch(token)  
	{
		case TOK_plus:	type = P_plus;	break;	// Operator "+"	
		case TOK_mul:	type = P_mul;	break;	// Operator "*"
		case TOK_lParenth:	type = P_lBrac;	break;	// Left parenthesis = "("
		case TOK_rParenth:	type = P_rBrac;	break;	// Right parenthesis ")"
		case TOK_identifier:	type = P_id;	break;	// @todo Is it "i"???
		// @todo case END_OF_STACK???
		
		default:	// Loaded token doesn't belong to the expression
			return P_endingToken;	// End function and report it's not an expression token
			
		/* @todo More cases for later
		case TOK_identifier:
		case TOK_integer:
		case TOK_decimal:
		case TOK_string:
		case TOK_lParenth:		// Left parenthesis = "("
		case TOK_rParenth:		// Right parenthesis ")"
		
		case TOK_minus:			// Operator "-"
		case TOK_mul:			// Operator "*"
		case TOK_div:			// Operator "/"
		case TOK_divInt			// Operator "\"
		case TOK_equal:			// Operator "="
		case TOK_notEqual:		// Operator "<>"
		case TOK_less:			// Operator "<"
		case TOK_lessEqual:		// Operator "<="
		case TOK_greater:		// Operator ">"
		case TOK_greaterEqual:	// Operator ">="
		*/
	}
	
	// Getting row index (depending on character on top of the stack)
	precTableIndex_t row;
	row = expr_getRowIndex(stack);
	
	// Reading next action from prececdent table
	precTableAction_t action;
	action = expr_readTable(type, row);
	
	// Performing the action
	switch(action)
	{
		case ACTION_shift:
			expr_shift();	// @todo
			break;
		case ACTION_reduce:
			expr_reduce();	// @todo
			break;
		case ACTION_specialShift:
			expr_specialshift();	// @todo
			break;
		case ACTION_ilegal:
			expr_error("expr_doOperation: Tryied to perform an ilegal action\n");
			break;	
	}
	
	return 0;	// @todo Return values, this is here just because gcc won't stop bitching about it
}





// ========== PRECEDENT TABLE FUNCTIONS ==========

precTableAction_t expr_readTable(precTableIndex_t colIndex, precTableIndex_t rowIndex)
{
	// Check if indexs are valid values
	if(colIndex >= PREC_TABLE_SIZE || rowIndex >= PREC_TABLE_SIZE || colIndex < 0 || rowIndex < 0)
	{
		expr_error("expr_readTable: Invalid index\n");
		return -1;
	}


	// Find action in precedent table
	switch(precTable[colIndex][rowIndex])
	{
		case '<':	return ACTION_shift;
		case '>':	return ACTION_reduce;
		case '=':	return ACTION_specialShift;
		case '#':	return ACTION_ilegal;
		
		default:	// Invalid character
			expr_error("expr_readTable: Invalid character in the precedent table ()\n");
			return -1;	// @todo Return values, this is here just because gcc won't stop bitching about it
	}
}

precTableIndex_t expr_getRowIndex(myStack_t *stack)
{
	char top = stackTop(stack);	// Get character on top of the stack
	
	switch(top)	// Return precedent table index depending on character on top of the stack
	{
		case '+':	return P_plus;
		case '*':	return P_mul;
		case '(':	return P_lBrac;
		case ')':	return P_rBrac;
		case 'i':	return P_id;
		case STACK_ENDCHAR:	return P_stackEnd;	// Character '$' (Defined in stack.h)	
			
		default:	// Invalid character
			expr_error("expr_getRowIndex: Invalid character on top of the stack\n");
			return -1;	// @todo Return values, this is here just because gcc won't stop bitching about it
	}
}





// ========== ACTION FUNCTIONS ==========

void expr_shift()	// @todo
{
	DEBUG_PRINT("[DBG] expr_shift()\n");
}

void expr_reduce()	// @todo
{
	DEBUG_PRINT("[DBG] expr_reduce()\n");
}

void expr_specialshift()	// @todo
{
}





// ========== OTHER FUNCTIONS ==========

void expr_error(char *msg)
{
	fprintf(stderr, "%s", msg);
	// @todo This function should end whole module and return err value to parser
}
