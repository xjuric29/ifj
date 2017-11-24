/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 * @todo Write test for return values
 */
 
// --- TESTING ---
//#define DEBUG   // Print stack, operations and table indexes


// Header file
#include "expr.h"




int algotihmFinished = EXPR_FALSE;       // Used static because I was lost in recursion

/**
 * @brief Precedental table determinating next action.
 * "<" = shift
 * ">" = reduce
 * "=" = special shift for brackets
 * "#" = error
 * rows = STACK - character on top of the stack
 * cols = INPUT - token loaded on input
 * @todo Expand for more values (Now using only 6 from example in lecture)
 * @todo Should be in header, source file or in function?
 */
char precTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] =
{  
//  +    -    \    *    /    (    )    i    $    =   <>    <   <=    >   >=
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#'},	// +
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#'},  // -
  {'>', '>', '>', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#'},  // \    // The space after '\' is important
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#'},	// *
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#'},  // /
  {'<', '<', '<', '<', '<', '<', '=', '<', '#', '#', '#', '#', '#', '#', '#'},	// (
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '#', '#', '#', '#', '#', '#'},	// )
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>'},	// i
  {'<', '<', '<', '<', '<', '<', '#', '<', '#', '<', '<', '<', '<', '<', '<'},  // $
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'},  // = 
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'},  // <>
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'},  // <
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'},  // <=
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'},  // >
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#'}   // >=
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
	"i",
	"E=E",
	"E<>E",
	"E<E",
	"E<=E",
	"E>E",
	"E>=E"
};





// ========== CORE FUNCTIONS ==========

int expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *variable)
{
	DEBUG_PRINT("--- Expression module start ---\n");
	DEBUG_PRINT("Context: %d\n", context);
	algotihmFinished = EXPR_FALSE;
	
        // --- Check arguments ---
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
        }
        
        
	// --- Initializing stack and default values ---
	myStack_t stack;	// Create stack
	stackInit(&stack);	// Init stack (Push "$")
        
	int continueLoading = 1;	// Determines if this module should read next token
	
	
	// --- Loading first token ---
	token_t loadedToken;	// Loaded token for this iterration of cycle
	loadedToken = *parserToken;	// It has no purpose, it's just to be more read friendly
	
	if(context != EXPRESION_CONTEXT_ARIGH)	// If context is arithmetic, first token is already loaded by parser
	{
		// Load token from scanner
		getToken(&loadedToken);
	}
	DEBUG_PRINT("[DBG] First token with type %d\n", loadedToken.type);
	
	
	// --- Check first token type ---
        if(expr_isFirstValid(loadedToken) == EXPR_FALSE) // Can be token used as beginning of an expression?
                return EXPR_RETURN_ERROR_SYNTAX;        // If not -> Syntax error
	
	
	// --- Loading tokens ---
	while(continueLoading)
	{
		DEBUG_PRINT("[DBG] Loading token with type %d\n", loadedToken.type);
		
		
                // --- Check if variable exists ---
                if(loadedToken.type == TOK_identifier)  // If token is variable
                        if(st_find_element(st_global, func_name, loadedToken.value.stringVal) == NULL)   // Haven't found it in the table
                        {
				expr_error("expr_main: Tried to work with nonexisting variable");
				DEBUG_PRINT("--- Expression module end (error) ---\n");
                                return EXPR_RETURN_ERROR_SEM;   // Return semantics error
                	}
                	
                		
		// --- CORE OF THE FUNCTION ---
		int retVal;	// Internal terminal type
		retVal = expr_algorithm(&stack, loadedToken, context);	// Use algorith on the loaded token
		
		 
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
                        {
				DEBUG_PRINT("--- Expression module end (error) ---\n");
                                return retVal;  // End module and report error
                        }       
                        
                        //DEBUG_PRINT("[DBG] retVal = %d\n", retVal);
                                
                        // --- Load next token ---
                        getToken(&loadedToken);
                }
		
	}
        
        
        // --- Finish the algorith ---
        // (This happens when there are no more tokens to load but algorithm is still not finished)
        while(expr_isAlgotihmFinished(&stack, loadedToken.type) == EXPR_FALSE)  // Should algorithm continue?        
        {
                // --- Token indicating stop of loading ---
                token_t noMoreTokens;
                noMoreTokens.type = TOK_endOfFile; // (Not really TOK_endOfFile, see header file at expr_algorithm())
                
                // --- Continue with the algorithm ---
                int retVal;     // Return value of the algorithm
                retVal = expr_algorithm(&stack, noMoreTokens, context);  
                
                
                // --- Check for error ---
                if(retVal != EXPR_RETURN_SUCC)
                {
			DEBUG_PRINT("--- Expression module end (error) ---\n");
                        return retVal;
		}
        }
        
       
	// --- Generate result instuction ---
        return expr_generateResult(context, variable);	// Or return error   
}

int expr_algorithm(myStack_t *stack, token_t token, int context)
{
	static token_t savedToken;	// Save token if it had some tokenValue_t (identifier/integer/decimal/string)
	
        // Check if algortihm is finished
        if(expr_isAlgotihmFinished(stack, token.type) == EXPR_TRUE)
                return EXPR_RETURN_SUCC;    // @todo Is this considered as success?
        
	// Initializing varables
	precTableIndex_t type;	// Internal type of token = Column index
	
	
	// Check if token is valid for this expression context
	switch(token.type)  
	{
		// Logic operators can't be in arithmetic expression
		case TOK_equal:
		case TOK_notEqual:
		case TOK_less:
		case TOK_lessEqual:
		case TOK_greater:
		case TOK_greaterEqual:
			if(context == EXPRESION_CONTEXT_ARIGH)	// @todo Maybe also PRINT?
			{
				expr_error("expr_algortihm: Logic oprator can't be in arithmetic expression");
				return EXPR_RETURN_ERROR_SEM;
			}
			break;
		
		// For gcc to shut the fuck up	
		default:
			break;
	}	
	
	
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
		case TOK_endOfFile:     type = TERM_stackEnd;      break;  // End of stack terminal '$' (Not really TOK_endOfFile, see header file)
        
        // Loaded logic token
        case TOK_equal:	type = TERM_equal;	break;			// Operator "="
		case TOK_notEqual:	type = TERM_notEqual;	break;		// Operator "<>"
		case TOK_less:	type = TERM_less;	break;			// Operator "<"
		case TOK_lessEqual:	type = TERM_lessEqual;	break;		// Operator "<="
		case TOK_greater:	type = TERM_greater;	break;		// Operator ">"
		case TOK_greaterEqual:	type = TERM_greaterEqual;	break;	// Operator ">="
                
		// Loaded token that has some value
		case TOK_identifier:
		case TOK_integer:
		case TOK_decimal:
			type = TERM_id;	// Identifier terminal = 'i'
			savedToken = token;	// Save token because it has  
		// All these tokens are going to be represented by 'i' @todo Is this good method?
			break;
		case TOK_string:
			expr_error("expr_algorithm: @todo Processing string")
			DEBUG_PRINT("--- Expression module end (error) ---\n");
			return EXPR_RETURN_ERROR_INTERNAL;
			break;
                
                
                // Loaded token DOESN'T belong to the expression
		default:        return EXPR_RETURN_NOMORETOKENS;	// End function and report it's not an expression token
			
		/* @todo More cases for later
		case TOK_string:
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
                        success = expr_reduce(stack, savedToken);
                        
                        if(success == EXPR_RETURN_ERROR_SYNTAX) // If rule not found
                                return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
                             
                        // Otherwise continue with algorithm
                        return expr_algorithm(stack, token, context);       // Use recursion (don't ask why, that's just the way it should be)
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

		default:	// @todo This is temporary solution for logic operators
			return character;
/*			
		default:	// Invalid character
			expr_error("expr_getIndexFromChar: Invalid character (dosn't need to be an error if called from stack.c)");
			return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it
*/
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

		default:	// @todo This is temporary solution for logic operators
			return index;

/*			
		default:	// Invalid character
			expr_error("expr_getCharFromIndex: Invalid index");
			return EXPR_ERROR;	// @todo Return values, this is here just because gcc won't stop bitching about it
*/
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

int expr_reduce(myStack_t *stack, token_t token)
{
	DEBUG_PRINT("[DBG] Operation >\n");
	
	// Initialize variable for handle
	string handle;	// Right side of grammar rule
	strInit(&handle);	// Init string
        
        
        // Generating instruction
        char terminal = stackGetTerminal(stack);
        expr_generateInstruction(terminal, token);
        
        
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
		case TOK_integer:
		case TOK_decimal:
		//case TOK_string: @todo string
                        return EXPR_TRUE;
                        
                // Other terminals
                default:
                        expr_error("expr_isFirstValid(): First token is not suitable for being first in expression");
                        return EXPR_FALSE;
        }
}

void expr_generateInstruction(char terminal, token_t token) // @todo
{
        switch(terminal)
        {
                // Operators
                case '+':
                        printf("ADDS\n");
                        break;
                case '-':
                        printf("SUBS\n");
                        break ;               
                case '*':
                        printf("MULS\n");
                        break;
                case '/':
                case '\\':
                        printf("DIVS\n");
                        break;
                        
                case 'i':     // We have to find out value
                {
                        switch(token.type)
                        {
                                case TOK_identifier:
                                        printf("PUSHS LF@%s\n", strGetStr(token.value.stringVal));
                                        break;
                                case TOK_integer:
                                        printf("PUSHS %d\n", token.value.integer);
                                        break;
                                case TOK_decimal:
                                        printf("PUSHS %f\n", token.value.decimal);
                                        break;
                                default:
                                        expr_error("expr_generateInstruction: Stack is 'i' but token.type is strange");
                                        // @todo This should end module
                                        return;
                        }
                        break;
                }
                // Logic operators
                case TERM_equal:
					printf("EQS\n");
					break;
                case TERM_notEqual:
                case TERM_less:
					printf("LTS\n");
					break;
                case TERM_lessEqual:
                case TERM_greater:
					printf("GTS\n");
					break;
                case TERM_greaterEqual:
					//printf("GTS\n");
					break;
        }
}

// ========== OTHER FUNCTIONS ==========

void expr_error(char *msg)
{
	fprintf(stderr, "[ERROR] %s\n", msg);
//        expr_finish();  // Postfix debug
//        exit(42);
	// @todo This function should end whole module and return err value to parser
}

int expr_generateResult(int context, st_element_t *variable)
{
	switch(context)
	{
		case EXPRESION_CONTEXT_ARIGH:
			if(variable == NULL)
			{
				expr_error("expr_generateResult: Result variable doesn't exist... Strange :O");
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return(EXPR_RETURN_ERROR_INTERNAL);
			}
			// @todo check types
			printf("POPS %s\n",strGetStr(&(variable->key)));
			break;
		case EXPRESION_CONTEXT_LOGIC:
			printf("JUMPS\n");
			break;
		case EXPRESION_CONTEXT_PRINT:
			// @todo
			expr_error("expr_generateResult: Not done for EXPRESION_CONTEXT_PRINT");
			DEBUG_PRINT("--- Expression module end (error) ---\n");
			return(EXPR_RETURN_ERROR_INTERNAL);
			break;
		case EXPRESION_CONTEXT_RETURN:
			// @todo check types
			printf("RETVAL_POP\n");	// No operator, inst modul has some internal variable
			break;
	}
	
	DEBUG_PRINT("--- Expression module end (success)---\n");
	return EXPR_RETURN_SUCC;
}
