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
 * @brief Precedental table determinating next action.
 * "<" = shift
 * ">" = reduce
 * "=" = special shift for brackets
 * "#" = error
 * rows = character on top of the stack
 * cols = token loaded on input
 * @todo Expand for more values (Now using only 6 from example in lecture)
 * @todo Should be in header, source file or in function?
 */
char precTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] =
{  
//   +    *    (    )    i    $
   {'>', '<', '<', '>', '<', '>'},	// +
   {'>', '>', '<', '>', '<', '>'},	// *
   {'<', '<', '<', '=', '<', '#'},	// (
   {'>', '>', '#', '>', '#', '>'},	// )
   {'>', '>', '#', '>', '#', '>'},	// i
   {'<', '<', '<', '#', '<', '#'}	// $
};

/**
 * @brief List of grammar rules.
 * 
 * There is only the right side, also called "handle".
 * Left side would always be "E -> ..."
 * 
 * @todo Expand for more values (Now using only 6 from example in lecture)
 * @todo Should be in header, source file or in function?
 */
char *rule[RULES_COUNT] =
{
	"E+E",
	"E*E",
	"(E)",
	"i"
};

myStack_t dbg_postfix;     // DEBUG


// ========== DEBUG ==========

#ifdef EXPR_TEST
int main()
{
	token_t endToken;
        
        stackInit(&dbg_postfix);  // DEBUG
	
	token_t id;
	id.type = TOK_identifier;
	
	expr_main(0, id, &endToken);
	
        
	DEBUG_PRINT("[DBG] Returning token to parser (external type = %d)\n", endToken.type);
	return EXPR_SUCCESS;
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
		DEBUG_PRINT("------\n[DBG] Loading token (external type=%d)\n", loadedToken.type);
		
		// CORE OF THE FUNCTION
		int type;	// Internal type of token
		type = expr_algorithm(&stack, loadedToken.type);	// Perform operation based on loaded token
		
		
		// End of expression (TERM_endingToken = Found token that doesn't belong to expression anymore (found in this run))
		if(type == TERM_endingToken)   // (TERM_stackEnd = Already found token that doesn't belong to expression anymore (found before this run))
		{
			// @todo Later if it's going to be more advanced, put it in reutrnProceeding() but for now:
			continueLoading = 0;	// Stop the loading cycle
			*endToken = loadedToken;	// Save token for parser to proceed
			// @todo Do something with the result and try to write this shit again like a human


                        // Finish algorith (There are no more tokens to load but algorithm is not finished)
                        while(expr_isAlgotihmFinished(&stack, loadedToken.type) == EXPR_FALSE)        
                        {
                                expr_algorithm(&stack, TOK_endOfFile);  // Continue with algorithm (Not really TOK_endOfFile, see header file at expr_algorithm())
                        }
                        
                        DEBUG_PRINT("[DBG] Algorithm completed!\n================\n");
                        stackInfo(&stack);
                        stackPush(&dbg_postfix, '\0');
                        DEBUG_PRINT("[DBG] Postfix: %s\n",&(dbg_postfix.arr));
			return EXPR_SUCCESS;	// Return success	
		}
		
		
		// Get next token for next run of the cycle
		#ifndef DEBUG
		getToken(&loadedToken);
		#else
		TEST_getToken(&loadedToken);
		#endif
		
	}
	while(continueLoading);
	
	return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it (If program gets to this line it means there is some strange smell of insect in the air)
}

int expr_algorithm(myStack_t *stack, tokenType_t tokenType)
{
        if(expr_isAlgotihmFinished(stack, tokenType) == EXPR_TRUE)      // Is algorithm finished?
                return EXPR_SUCCESS;    // Return success
        
        
	precTableIndex_t type;	// Internal type of token = Column index
	
	// Getting column index (based on external type of token (tokenType_t from scanner.h))
	switch(tokenType)  
	{
		case TOK_plus:	type = TERM_plus;	break;	// Operator terminal '+'	
		case TOK_mul:	type = TERM_mul;	break;	// Operator terminal '*'
		case TOK_lParenth:	type = TERM_lBrac;	break;	// Left parenthesis terminla = '('
		case TOK_rParenth:	type = TERM_rBrac;	break;	// Right parenthesis terminal = ')'
		case TOK_identifier:	type = TERM_id;	break;	// @todo Is it "i"???
		case TOK_endOfFile:     type = TERM_stackEnd;      break;  // End of stack terminal '$' (Not really TOK_endOfFile, see header file)
		
		default:	// Loaded token doesn't belong to the expression
			return TERM_endingToken;	// End function and report it's not an expression token
			
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
	
	// Getting row index (depending on terminal on top of the stack)
	char top = stackGetTerminal(stack);	// Get character on top of the stack
	precTableIndex_t row;
	row = expr_getIndexFromChar(top);	// Get index in precedent table
	
	// Reading next action from prececdent table
	precTableAction_t action;
	action = expr_readTable(row, type);
	
	DEBUG_PRINT("[DBG] row=%d col=%d action=%d\n", row, type, action);
	
	// Performing the action
	switch(action)
	{
		case ACTION_shift:
			expr_shift(stack, expr_getCharFromIndex(type));
			break;
		case ACTION_reduce:
			expr_reduce(stack);
                        expr_algorithm(stack, tokenType);       // Use recursion (don't ask why, that's just the way it should be)
			break;
		case ACTION_specialShift:
			expr_specialShift(stack, expr_getCharFromIndex(type));
			break;
		case ACTION_ilegal:
			expr_error("expr_algorithm: Tried to perform an ilegal action");
			break;	
	}
	
	return EXPR_SUCCESS;	// @todo Return values, this is here just because gcc won't stop bitching about it
}





// ========== PRECEDENT TABLE FUNCTIONS ==========

precTableAction_t expr_readTable(precTableIndex_t rowIndex, precTableIndex_t colIndex)
{
	// Check if indexs are valid values
	if(colIndex >= PREC_TABLE_SIZE || rowIndex >= PREC_TABLE_SIZE || colIndex < 0 || rowIndex < 0)
	{
		expr_error("expr_readTable: Invalid index");
		return EXPR_ERROR;
	}


	// Find action in precedent table
	switch(precTable[rowIndex][colIndex])
	{
		case '<':	return ACTION_shift;
		case '>':	return ACTION_reduce;
		case '=':	return ACTION_specialShift;
		case '#':	return ACTION_ilegal;
		
		default:	// Invalid character
			expr_error("expr_readTable: Invalid character in the precedent table ()");
			return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it
	}
}


precTableIndex_t expr_getIndexFromChar(char character)
{
	switch(character)	// Return precedent table index depending on character on top of the stack
	{
		case '+':	return TERM_plus;
		case '*':	return TERM_mul;
		case '(':	return TERM_lBrac;
		case ')':	return TERM_rBrac;
		case 'i':	return TERM_id;
		case STACK_ENDCHAR:	return TERM_stackEnd;	// Character '$' (Defined in stack.h)	
			
		default:	// Invalid character
			expr_error("expr_getIndexFromChar: Invalid character (dosn't need to be an error if called from stack.c)");
			return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it
	}
}


char expr_getCharFromIndex(precTableIndex_t index)
{
	switch(index)	// Return precedent table index depending on character on top of the stack
	{
		case TERM_plus:	return '+';
		case TERM_mul:	return '*';
		case TERM_lBrac:	return '(';
		case TERM_rBrac:	return ')';
		case TERM_id:	return 'i';
		case TERM_stackEnd:	return STACK_ENDCHAR;	// Character '$' (Defined in stack.h)	
			
		default:	// Invalid character
			expr_error("expr_getCharFromIndex: Invalid index");
			return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it
	}	
}





// ========== ACTION FUNCTIONS ==========

void expr_shift(myStack_t *stack, char character)
{
	DEBUG_PRINT("[DBG] Operation <\n");
	
	stackShiftPush(stack); // Push '<' after closest terminal to the end of the stack
	stackPush(stack, character);    // Push the terminal at the end of the stack
        
        stackInfo(stack);	// Debug
}

void expr_reduce(myStack_t *stack)
{
	DEBUG_PRINT("[DBG] Operation >\n");
	
	// Initialize variable for handle
	string handle;	// Right side of grammar rule
	strInit(&handle);	// Init string
	
        
        // --- DEBUG --- (Generating postfix)
        char terminal = stackGetTerminal(stack);
        if(terminal != ')')
                stackPush(&dbg_postfix, terminal);
		
        // --- END DEBUG ---
        
        
        // Find out what to reduce
	char top = stackPop(stack);	// Get and remove character from top of the stack
	while(top != '<' && top != STACK_ENDCHAR)	// Keep poping untill end of reduce or end of stack is found
	{
		int success;
		success = strAddChar(&handle, top);	// Add character from top of the stack at the end of the string
		if(success == STR_ERROR)	// If couldn't
		{
			expr_error("expr_reduce: Couldn't add char to end of string");
			return;
		}
		top = stackPop(stack);	// Get character from top of the stack for next run of the cycle
        }
        
        // Search for grammar rule
	int rule;
        rule = expr_searchRule(handle);
        
        strFree(&handle);       // Free memory of the handle
        
        // Check if rule was found
        if(rule == EXPR_ERROR)
        {
                expr_error("expr_reduce: Handle doen't match any rule");
                return; // @todo This shouldn't be void function
        }
        
        // Push left side of the rule to the stack (always E)
        stackPush(stack, 'E');
        
        stackInfo(stack);	// Debug
}

void expr_specialShift(myStack_t *stack, char character)
{
        DEBUG_PRINT("[DBG] Operation =\n");
        
        stackPush(stack, character);    // Push the terminal at the end of the stack
        
        stackInfo(stack);	// Debug
}





// ========== SUB FUNCTIONS ==========

int expr_searchRule(string handle)
{
	// @todo think about return value, I guess it should be E all the time
	
	for(int i=0; i < RULES_COUNT; i++)	// Compare with every existing grammar rule
	{
		if(strCmpConstStr(&handle, rule[i]))	// If found a match
                {
			return i;	// Return rule number
                }
        }	
	
	return EXPR_ERROR;	// If not found return fail
}

int expr_isAlgotihmFinished(myStack_t *stack, int tokenType)
{
        static int algotihmFinished = EXPR_FALSE;       // Used static because I was lost in recursion
        
        if(algotihmFinished == EXPR_TRUE)       // If already finished
                return EXPR_TRUE;
        
        if(stack->top == 1 && stackTop(stack) == 'E' && tokenType == TOK_endOfFile)  // If finishing now
        {
                algotihmFinished = EXPR_TRUE;   // Change static int
                return EXPR_TRUE;
        }
        
        return EXPR_FALSE;      // Otherwise return false
}

// ========== OTHER FUNCTIONS ==========

void expr_error(char *msg)
{
	fprintf(stderr, "[ERROR] %s\n", msg);
	// @todo This function should end whole module and return err value to parser
}
