/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 * @todo Write test for return values
 */
 
// --- TESTING ---
#define EXPR_TEST       // Simulate scanner and generate postfix
//#define DEBUG   // Print stack, operations and table indexes

#ifdef EXPR_TEST
#include "tests/expr/expr-test.c"
int testNum;
extern char input[EXPR_TESTSTR_LENGTH];
extern char expected[EXPR_TESTSTR_LENGTH];
extern int expectedRetVal;
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
//  +    -    \    *    /    (    )    i    $   
  {'>', '>', '<', '<', '<', '<', '>', '<', '>'},	// +
  {'>', '>', '<', '<', '<', '<', '>', '<', '>'},        // -
  {'>', '>', '>', '<', '<', '<', '>', '<', '>'},        // \    // The space after '\' is important
  {'>', '>', '>', '>', '>', '<', '>', '<', '>'},	// *
  {'>', '>', '>', '>', '>', '<', '>', '<', '>'},        // /
  {'<', '<', '<', '<', '<', '<', '=', '<', '#'},	// (
  {'>', '>', '>', '>', '>', '#', '>', '#', '>'},	// )
  {'>', '>', '>', '>', '>', '#', '>', '#', '>'},	// i
  {'<', '<', '<', '<', '<', '<', '#', '<', '#'}         // $
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
        "E-E",
        "E\\E",
	"E*E",
        "E/E",
	"(E)",
	"i"
};



// ========== DEBUG ==========
#ifdef EXPR_TEST
myStack_t dbg_postfix;     // POSTFIX DEBUG
extern token_t firstTestToken;


int main(int argc, char *argv[])
{
        if(argc != 2)
        {
                expr_error("main: Test number not specified");
                return EXPR_FALSE;
        }

        testNum = strtol(argv[1], NULL, 10);
        TEST_generateInputStr(testNum);

	token_t parserToken;
        parserToken = TEST_getFirstToken();
        
        stackInit(&dbg_postfix);  // POSTFIX DEBUG

        string function;
        strInit(&function);
        strAddChar(&function, 'T');
        strAddChar(&function, 'e');
        strAddChar(&function, 's');
        strAddChar(&function, 't');
        
	
        int retVal;
	retVal = expr_main(&parserToken, NULL, &function);
        
        strFree(&function);
        
	expr_testFinish_retVal(retVal);
        return retVal;
}
#endif





// ========== CORE FUNCTIONS ==========

int expr_main(token_t *parserToken, st_globalTable_t *st_global, string *func_name)
{
        // --- Check arguments ---
        /*
        if(parserToken == NULL)
        {
                expr_error("expr_main: NULL pointer to parserToken");
                return EXPR_RETURN_ERROR_INTERNAL;
        }
        if(st_global == NULL)
        {
                expr_error("expr_main: NULL pointer to st_global");
                return EXPR_RETURN_ERROR_INTERNAL;
        }
        if(func_name == NULL)
        {
                expr_error("expr_main: NULL pointer to func_name");
                return EXPR_RETURN_ERROR_INTERNAL;
        }*/
        
        
        // --- Check first token type ---
        if(expr_isFirstValid(*parserToken) == EXPR_FALSE) // Can be token used as beginning of an expression?
                return EXPR_RETURN_ERROR_SYNTAX;        // If not -> Syntax error
        
        
	// --- Initializing stack and default values ---
	myStack_t stack;	// Create stack
	stackInit(&stack);	// Init stack (Push "$")
        
	int continueLoading = 1;	// Determines if this module should read next token
	
	token_t loadedToken;
	loadedToken = *parserToken;	// Value for first run of loading cycle


	// --- Loading tokens ---
	do
	{		
		// --- CORE OF THE FUNCTION ---
		int retVal;	// Internal terminal type
		retVal = expr_algorithm(&stack, loadedToken);	// Use algorith on the loaded token
		
		 
		if(retVal == EXPR_RETURN_NOMORETOKENS)    // TERM_endingToken = Found token that doesn't belong to expression anymore
		{
                        // --- End of expression ---
			continueLoading = 0;	// Stop the loading cycle
			*parserToken = loadedToken;	// Save token for parser to proceed // Don't need this anymore
		}       
                else
		{
                        // --- Check for error ---
                        if(retVal != EXPR_RETURN_SUCC)  // If an error occurred
                                return retVal;  // End module and report error
                                
                                
                        // --- Load next token ---
                        #ifndef EXPR_TEST
                        getToken(&loadedToken);
                        #else
                        TEST_getToken(&loadedToken);
                        #endif
                }
		
	}
	while(continueLoading);
        
        
        // --- Finish the algorith ---
        // (This happens when there are no more tokens to load but algorithm is still not finished)
        while(expr_isAlgotihmFinished(&stack, loadedToken.type) == EXPR_FALSE)  // Should algorithm continue?        
        {
                // --- Token indicating stop of loading ---
                token_t noMoreTokens;
                noMoreTokens.type = TOK_endOfFile; // (Not really TOK_endOfFile, see header file at expr_algorithm())
                
                // --- Continue with the algorithm ---
                int retVal;     // Return value of the algorithm
                retVal = expr_algorithm(&stack, noMoreTokens);  
                
                
                // --- Check for error ---
                if(retVal != EXPR_RETURN_SUCC)
                        return retVal;
        }
        
       
        // ===== POSTFIX DEBUG =====
        expr_finish();

        return EXPR_RETURN_SUCC;        // Return success
}

int expr_algorithm(myStack_t *stack, token_t token)
{
        if(expr_isAlgotihmFinished(stack, token.type) == EXPR_TRUE)      // Is algorithm finished?
                return EXPR_RETURN_SUCC;    // @todo Is this considered as success?
        
        
	precTableIndex_t type;	// Internal type of token = Column index
	
	// Getting column index (based on external type of token (tokenType_t from scanner.h))
	switch(token.type)  
	{
                // Loaded token belong to the expression
		case TOK_plus:	type = TERM_plus;	break;	// Operator terminal '+'
                case TOK_minus:	type = TERM_minus;	break;	// Operator terminal '-'
                case TOK_divInt:	type = TERM_divInt;	break;	// Operator terminal '\'			
		case TOK_mul:	type = TERM_mul;	break;	// Operator terminal '*'
                case TOK_div:	type = TERM_div;	break;	// Operator terminal '/'
		case TOK_lParenth:	type = TERM_lBrac;	break;	// Left bracket terminlal = '('
		case TOK_rParenth:	type = TERM_rBrac;	break;	// Right bracket terminal = ')'
		case TOK_identifier:	type = TERM_id;	break;	// Identifier terminal = 'i'
		case TOK_endOfFile:     type = TERM_stackEnd;      break;  // End of stack terminal '$' (Not really TOK_endOfFile, see header file)
                
                // Loaded token DOESN'T belong to the expression
		default:        return EXPR_RETURN_NOMORETOKENS;	// End function and report it's not an expression token
			
		/* @todo More cases for later
		case TOK_integer:
		case TOK_decimal:
		case TOK_string:
		

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
                // Operation SHIFT '<'
		case ACTION_shift:      return expr_shift(stack, expr_getCharFromIndex(type));
                
                // Operation REDUCE '>'
		case ACTION_reduce:
                {
			int success;    // Return value of reducing (= searching for rule)
                        success = expr_reduce(stack);
                        
                        if(success == EXPR_RETURN_ERROR_SYNTAX) // If rule not found
                                return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
                                
                        // Otherwise continue with algorithm
                        return expr_algorithm(stack, token);       // Use recursion (don't ask why, that's just the way it should be)
		}
                
                // Operation SPECIAL SHIFT '='
                case ACTION_specialShift:       return expr_specialShift(stack, expr_getCharFromIndex(type));
		
                // ILEGAL OPERATON '#'
                case ACTION_ilegal:
			expr_error("expr_algorithm: Tried to perform an ilegal action");
                        return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
	}
	
        // @todo Edit this function so this below doesn't look so stupid
        expr_error("expr_algorithm: Shouldn't ever reach this line");
	return EXPR_RETURN_ERROR_INTERNAL;
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
                case '-':	return TERM_minus;
                case '\\':	return TERM_divInt;
		case '*':	return TERM_mul;
                case '/':	return TERM_div;
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
                case TERM_minus:	return '-';
                case TERM_divInt:	return '\\';
		case TERM_mul:	return '*';
                case TERM_div:	return '/';
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

int expr_shift(myStack_t *stack, char character)
{
	DEBUG_PRINT("[DBG] Operation <\n");
	
	stackShiftPush(stack); // Push '<' after closest terminal to the end of the stack
	stackPush(stack, character);    // Push the terminal at the end of the stack
        
        stackInfo(stack);	// Debug
        
        return EXPR_RETURN_SUCC;        // @todo return values for error when working with stack
}

int expr_reduce(myStack_t *stack)
{
	DEBUG_PRINT("[DBG] Operation >\n");
	
	// Initialize variable for handle
	string handle;	// Right side of grammar rule
	strInit(&handle);	// Init string
        
        
        // Generating instruction
        char terminal = stackGetTerminal(stack);
        expr_generateInstruction(terminal);
        
        
        // --- DEBUG --- (Generating postfix)
        if(terminal != ')')
                stackPush(&dbg_postfix, terminal);
        // --- END DEBUG ---
        
        
        // Find out what to reduce
	char top = stackPop(stack);	// Get and remove character from top of the stack (@todo check if din't try to pop empty stack)
	while(top != '<' && top != STACK_ENDCHAR)	// Keep poping untill end of reduce or end of stack is found
	{
		int success;
		success = strAddChar(&handle, top);	// Add character from top of the stack at the end of the string
		if(success == STR_ERROR)	// If couldn't
		{
                        strFree(&handle);       // Avoid memory leak
			expr_error("expr_reduce: Couldn't add char to end of string");
			return EXPR_RETURN_ERROR_INTERNAL;        // Return internal error
		}
		top = stackPop(stack);	// Get character from top of the stack for next run of the cycle
        }
        
        // Search for grammar rule
	int ruleFound;
        ruleFound = expr_searchRule(handle);
        strFree(&handle);       // Free memory of the handle
        
        // Check if rule was found
        if(ruleFound == EXPR_RETURN_ERROR_SYNTAX)
        {
                expr_error("expr_reduce: Handle doen't match any rule");
                return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
        }
        
        // Update stack
        stackPush(stack, 'E'); // Push left side of the rule to the stack (always E)
        
        // Debug
        stackInfo(stack);
        
        // Return value
        return EXPR_RETURN_SUCC;       // Return success
}

int expr_specialShift(myStack_t *stack, char character)
{
        DEBUG_PRINT("[DBG] Operation =\n");
        
        stackPush(stack, character);    // Push the terminal at the end of the stack
        
        stackInfo(stack);	// Debug
        
        return EXPR_RETURN_SUCC;        // @todo return values for error when working with stack
}





// ========== SUB FUNCTIONS ==========

int expr_searchRule(string handle)
{
	// @todo think about return value, I guess it should be E all the time
	
	for(int i=0; i < RULES_COUNT; i++)	// Compare with every existing grammar rule
	{
		if(strCmpConstStr(&handle, rule[i]))	// If found a match
                {
			return EXPR_RETURN_SUCC;	// Return succes
                }
        }	
	
	return EXPR_RETURN_ERROR_SYNTAX;	// If not found syntax error
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

int expr_isFirstValid(token_t firstToken)
{
        switch(firstToken.type) // Check token type
        {      
                // Legal terminals to stand as first
		case TOK_lParenth:
		case TOK_identifier:
                        return EXPR_TRUE;
                        
                // Other terminals
                default:
                        expr_error("expr_isFirstValid(): First token is not suitable for being first in expression");
                        return EXPR_FALSE;
        }
}

void expr_generateInstruction(char terminal)
{
        // @todo Filter bracktes, because they don't produce an instruction
}

// ========== OTHER FUNCTIONS ==========

void expr_error(char *msg)
{
	fprintf(stderr, "[ERROR] %s\n", msg);
        expr_finish();  // Postfix debug
//        exit(42);
	// @todo This function should end whole module and return err value to parser
}

void expr_finish()
{
#ifdef EXPR_TEST
        // ===== Postfix debug =====
        stackPush(&dbg_postfix, '\0');
        printf("===TEST #%d===\n",testNum);
        printf("%s [Input]\n",input);
        printf("%s [Output]\n",&(dbg_postfix.arr));
        printf("%s [Expected]\n",expected);

#endif
}

void expr_testFinish_retVal(int retVal)
{
#ifdef EXPR_TEST
        // ===== Postfix debug =====
        printf("RetVal=%d [Expected=%d]\n",retVal,expectedRetVal);       
#endif
}
