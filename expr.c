/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 */
 
// --- TESTING ---
#define DEBUG   // Print stack, operations and table indexes


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
 */
char precTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] =
{  
//  +    -    \    *    /    (    )    i    $    =   <>    <   <=    >   >=   str
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // +
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '#'},  // -
  {'>', '>', '>', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '#'},  // \    // The space after '\' is important
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '#'},  // *
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '#'},  // /
  {'<', '<', '<', '<', '<', '<', '=', '<', '#', '#', '#', '#', '#', '#', '#', '#'},  // (
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '#', '#', '#', '#', '#', '#', '#'},  // )
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#'},  // i
  {'<', '<', '<', '<', '<', '<', '#', '<', '#', '<', '<', '<', '<', '<', '<', '<'},  // $
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // = 
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <>
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <=
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // >
  {'#', '#', '#', '#', '#', '#', '#', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // >=
  {'>', '#', '#', '#', '#', '#', '#', '#', '>', '>', '>', '>', '>', '>', '>', '#'},  // str
};


// ========== CORE FUNCTIONS ==========

int expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *variable)
{
	DEBUG_PRINT("--- Expression module start ---\n");
	DEBUG_PRINT("Context: %d\n", context);
	
	int firstToken = EXPR_TRUE;     // Indicates if the processed token is the first in the expression
	
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
        
        
    // --- Setup variables ---    
 	int continueLoading = 1;	// Determines if this module should read next token
	algotihmFinished = EXPR_FALSE;       
        
        
	// --- Initializing stacks ---
	myStack_t stack;	// Create terminal stack
	stackInit(&stack);	// Init stack (Push "$")
	tokStack_t tokStack;

	int success;
	success = tokStack_Init(&tokStack);
	if(success == FAIL)
	{
		expr_error("expr_main: Couldn't init token stack");
		DEBUG_PRINT("--- Expression module end (error) ---\n");	
		return EXPR_RETURN_ERROR_INTERNAL;		
	}
    
    
    // --- Reset temporary varaible for strings ---
	string *varString;
	strInit(varString);
	char *varChar = "$str";
	strCopyConst(varString, varChar);
	
	// Creating token with empty string
	token_t *resetToken = TokenInit(); // Initialize token
	resetToken->type = TOK_string;
	
	// Resetting variable in 3AC
	add_instruction(MOVE_LF_LF, resetToken, varString, NULL);	// MOVE LF@$str ""
					
	// Update tokStack (strings are NOT stored in stack but tokStack is still used for data type check)
	tokStack_Push(&tokStack, TOK_string);
	
	// Free memory
	strFree(varString);
	TokenFree(resetToken);
    
	
	// --- Loading first token ---
	token_t loadedToken;	// Loaded token for this iterration of cycle
	loadedToken = *parserToken;	// It has no purpose, it's just to be more read friendly
	
	if(context != EXPRESSION_CONTEXT_ASSIGN)	// If context is arithmetic, first token is already loaded by parser
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
         
        int skipMaskingAsID = EXPR_FALSE;	// Reset variable to default
		
		// --- Search for varibale in symbol table ---
		if(loadedToken.type == TOK_identifier)  // If token is variable
		{
			st_element_t *element = st_find_element(st_global, func_name, loadedToken.value.stringVal);
			
			// --- Check if variable exists ---
			if(element == NULL)   // Haven't found it in the table
			{
				expr_error("expr_main: Tried to work with nonexisting variable");
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return EXPR_RETURN_ERROR_SEM;   // Return semantics error
			}
			
			// --- Find out variable type --- 
			tokenType_t tokenType = expr_elTypeConvert(element->el_type);
			if(tokenType == TOK_FAIL)
			{
				// Error message already printed in expr_elTypeConvert()
				DEBUG_PRINT("--- Expression module end (error) ---\n");		
				return EXPR_RETURN_ERROR_INTERNAL;						
			}
			else
			{
				// Push variable type to the stack
				// (If it's value and not variable, then this is done in expr_algorithm())
				tokStack_Push(&tokStack, tokenType);
				
				if(tokenType == TOK_string)
					skipMaskingAsID = EXPR_TRUE;	// Can't save string as TERM_identifier because it has it's own teriminal TERM_string
			}
		}    
                
                  	
                	
                		
		// --- CORE OF THE FUNCTION ---
		int retVal;	// Internal terminal type
		retVal = expr_algorithm(&stack, &tokStack, loadedToken, context, skipMaskingAsID);	// Use algorithm on the loaded token
		if(retVal == EXPR_RETURN_STARTOVER)
		{
			// Found semicolon in print -> Start process over
			expr_generateResult(&tokStack, context, st_global, func_name, variable);
			return expr_main(context, parserToken, st_global, func_name, variable);
		}
		 
		if(retVal == EXPR_RETURN_NOMORETOKENS)    // EXPR_RETURN_NOMORETOKENS = Found token that doesn't belong to expression anymore
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
        DEBUG_PRINT("[DBG] Loading done\n");
        
        
	// --- Finish the algorith ---
	// (This happens when there are no more tokens to load but algorithm is still not finished)
	int retVal = expr_finishAlgorithm(&stack, &tokStack, loadedToken, context);
	if(retVal != EXPR_RETURN_SUCC)
	{
		// Found semicolon in print -> Start process over
		if(retVal == EXPR_RETURN_STARTOVER)
		{
			expr_generateResult(&tokStack, context, st_global, func_name, variable);
			return expr_main(context, parserToken, st_global, func_name, variable);
		}
		else
			return retVal;
	}

	// --- Generate result instuction ---
	return expr_generateResult(&tokStack, context, st_global, func_name, variable);	// Or return error   
}

int expr_algorithm(myStack_t *stack, tokStack_t *tokStack, token_t token, int context, int skipMaskingAsID)
{
	static token_t savedToken;	// Save token if it had some tokenValue_t (identifier/integer/decimal/string)
	
        // Check if algortihm is finished
        if(expr_isAlgotihmFinished(stack, token.type) == EXPR_TRUE)
                return EXPR_RETURN_SUCC;
        
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
			if(context == EXPRESSION_CONTEXT_ASSIGN || context == EXPRESSION_CONTEXT_PRINT)
			{
				expr_error("expr_algortihm: Logic oprator can't be in assignment or print expression");
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
		// === Arithmetic operators ===
		case TOK_plus:	type = TERM_plus;	break;	// Operator terminal '+'
		case TOK_minus:	type = TERM_minus;	break;	// Operator terminal '-'
		case TOK_divInt:	type = TERM_divInt;	break;	// Operator terminal '\'			
		case TOK_mul:	type = TERM_mul;	break;	// Operator terminal '*'
		case TOK_div:	type = TERM_div;	break;	// Operator terminal '/'
		case TOK_lParenth:	type = TERM_lBrac;	break;	// Left bracket terminlal = '('
		case TOK_rParenth:	type = TERM_rBrac;	break;	// Right bracket terminal = ')'
		case TOK_endOfFile:     type = TERM_stackEnd;      break;  // End of stack terminal '$' (Not really TOK_endOfFile, see header file)
        
        // === Logic operators ===
        case TOK_equal:	type = TERM_equal;	break;			// Operator "="
		case TOK_notEqual:	type = TERM_notEqual;	break;		// Operator "<>"
		case TOK_less:	type = TERM_less;	break;			// Operator "<"
		case TOK_lessEqual:	type = TERM_lessEqual;	break;		// Operator "<="
		case TOK_greater:	type = TERM_greater;	break;		// Operator ">"
		case TOK_greaterEqual:	type = TERM_greaterEqual;	break;	// Operator ">="
                
		// === Tokens with values ===
		// --- Numbers ---
		case TOK_integer: // Both represented by 'i', they have the same behavior
		case TOK_decimal:
			type = TERM_id;	// Identifier terminal = 'i'
			savedToken = token;	// Save token because it has value
			tokStack_Push(tokStack, token.type);
			break;
		// --- Strings ---
		case TOK_string:
			type = TERM_string;
			savedToken = token;	// Save token because it has value
			tokStack_Push(tokStack, TOK_string);
			break;
		// --- Variables ---
		case TOK_identifier:
			savedToken = token; // Save token because it has value
			if(skipMaskingAsID == EXPR_FALSE)
				type = TERM_id;	// Identifier terminal = 'i'
			else
				type = TERM_string;	// String terminal = 'str'
		break;

        // === Other tokens ===
        // --- Semicolon ---
        case TOK_semicolon:
			if(context == EXPRESSION_CONTEXT_PRINT)
			{
				// Finish the algorithm
				int retVal = expr_finishAlgorithm(stack, tokStack, token, context);
				if(retVal != EXPR_RETURN_SUCC)
					return retVal;
					
				// @todo Print write instruction
				
				// Return signal to start over expr_main
				return EXPR_RETURN_STARTOVER;
			}
			else
				return EXPR_RETURN_NOMORETOKENS;	// End function and report it's not an expression token
        break;
        // --- Nonexpression token ---
		default:        return EXPR_RETURN_NOMORETOKENS;	// End function and report it's not an expression token
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
                        success = expr_reduce(stack, tokStack, savedToken);
                        
                        if(success == EXPR_RETURN_ERROR_SYNTAX) // If rule not found
                                return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
                             
                        // Otherwise continue with algorithm
                        return expr_algorithm(stack, tokStack, token, context, skipMaskingAsID);       // Use recursion (don't ask why, that's just the way it should be)
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
			expr_error("expr_readTable: Invalid character in the precedent table");
			return ACTION_ilegal;
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
			expr_error("expr_getIndexFromChar: Invalid character");
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

int expr_reduce(myStack_t *stack, tokStack_t *tokStack, token_t token)
{
	DEBUG_PRINT("[DBG] Operation >\n");
	
	// Initialize variable for handle
	string handle;	// Right side of grammar rule
	strInit(&handle);	// Init string
        
        
	// Generating instruction
	char terminal = stackGetTerminal(stack);
	expr_generateInstruction(tokStack, terminal, token);


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
	/**
	 * @brief List of grammar rules.
	 * 
	 * There is only the right side, also called "handle".
	 * Left side would always be "E -> ..."
	 */
	char rule[RULES_COUNT][4] =
	{
		"E+E",
		"E-E",
		"E\\E",
		"E*E",
		"E/E",
		"(E)",
		"i",
		"E=E",
		{'E',TERM_notEqual,'E','\0'},	// E<>E
		"E<E",
		{'E',TERM_lessEqual,'E','\0'},	// E<=E
		"E>E",
		{'E',TERM_greaterEqual,'E','\0'},	// E>=E
        {TERM_string,'\0'}	// str
	};
	
	
	// Comparation
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
		case TOK_string:
                        return EXPR_TRUE;
                        
                // Other terminals
                default:
                        expr_error("expr_isFirstValid(): First token is not suitable for being first in expression");
                        return EXPR_FALSE;
        }
}

void expr_generateInstruction(tokStack_t *tokStack, char terminal, token_t token) // @todo
{
	switch(terminal)
	{
	// Operators
	case '+':
		expr_convertTypes(tokStack, terminal);
		
		if(tokStack_Top(tokStack) != TOK_string)	// Operator '+' uses different function for string concate and arithmetic plus
			add_instruction(ADDS, NULL, NULL, NULL);
		else
		{
			// --- String ---
			string *varString;
			strInit(varString);
			char *varChar = "$str";
			strCopyConst(varString, varChar);
	
			// Resetting variable in 3AC
			add_instruction(CONCAT, &token, varString, NULL);	// Add string at end of the temporary string
	
			// Free memory
			strFree(varString);
		}
		break;
	case '-':
		expr_convertTypes(tokStack, terminal);
		add_instruction(SUBS, NULL, NULL, NULL);		
		break;               
	case '*':
		expr_convertTypes(tokStack, terminal);
		add_instruction(MULS, NULL, NULL, NULL);		
		break;
	case '/':
		expr_convertTypes(tokStack, terminal);
		add_instruction(DIVS, NULL, NULL, NULL);	
		break;
	case '\\':
		expr_convertTypes(tokStack, terminal);
		add_instruction(DIVS, NULL, NULL, NULL);
		add_instruction(INT2FLOATS, NULL, NULL, NULL);	// divInt must have int result
		break;	
	
	// Identifier / integer / decimal
	case 'i':
		add_instruction(PUSHS, &token, NULL, NULL);
		break;

	// Logic operators
	case TERM_equal:
		add_instruction(EQS, NULL, NULL, NULL);
		break;
	case TERM_notEqual:
		add_instruction(EQS, NULL, NULL, NULL);
		add_instruction(NOTS, NULL, NULL, NULL);
		break;
	case TERM_less:
		add_instruction(LTS, NULL, NULL, NULL);
		break;
	case TERM_lessEqual:
		expr_error("expr_generateInstruction: @todo TERM_lessEqual not done");
		// @todo
		break;
	case TERM_greater:
		add_instruction(GTS, NULL, NULL, NULL);
		break;
	case TERM_greaterEqual:
		expr_error("expr_generateInstruction: @todo TERM_greaterEqual not done");
		// @todo
		break;
	}
}




// ========== CONVERT FUNCTIONS ==========

void expr_convertTypes(tokStack_t *tokStack, char terminal)
{
	switch(terminal)
	{		
		case '+':
		case '-':
		case '*':
		case '/':
		case '\\':
		case TERM_equal:	// For logic result the top of tokStack is wrong after this function but it doesn't matter
		case TERM_notEqual:	// @todo int < int = bool, not int (that means when using logic operation, the tokStack is wrong but it doesn't matter)
		case TERM_less:
		case TERM_lessEqual:
		case TERM_greater:
		case TERM_greaterEqual:
		{		
			tokenType_t typeRight = tokStack_Pop(tokStack);
			tokenType_t typeLeft = tokStack_Pop(tokStack);
			
			if(typeLeft == TOK_integer && typeRight == TOK_integer)	// int # int = int
			{	
				if(terminal != '/' && terminal != '\\')
					tokStack_Push(tokStack, TOK_integer);
				else
				{
					/* Example:	// DIV must have two dec operands
					[INT / INT = DEC]
					INT...a
					INT...b
					-------------
					POPS LF@$int
					INT2FLOATS
					PUSHS LF@$int
					INT2FLOATS
					DIVS
					*/
					
					// Prepare string with temporary variable
					string tmpString;
					strInit(&tmpString);
					char *tmpChar = "$int";
					strCopyConst(&tmpString, tmpChar);
					
					// Converting instructions
					add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$dec
					add_instruction(INT2FLOATS, NULL, NULL, NULL);
					add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$dec
					add_instruction(INT2FLOATS, NULL, NULL, NULL);
									
					// Update tokStack
					tokStack_Push(tokStack, TOK_decimal);
					
					// Free string memory
					strFree(&tmpString);
				}
			}
			else if(typeLeft == TOK_decimal && typeRight == TOK_decimal)	// dec # dec = dec
			{
				tokStack_Push(tokStack, TOK_decimal);
			}
			else if(typeLeft == TOK_string && typeRight == TOK_string)	// string # string = string
			{
				tokStack_Push(tokStack, TOK_string);
			}
			else if(typeLeft == TOK_integer && typeRight == TOK_decimal)	// (int) # dec = dec
			{	
				/* Example :
				[INT + DEC = DEC]
				INT...a
				DEC...b
				-------------
				POPS LF@$dec
				INT2FLOATS
				PUSHS LF@$dec
				ADDS
				*/
				
				
				// Prepare string with temporary variable
				string tmpString;
				strInit(&tmpString);
				char *tmpChar = "$dec";
				strCopyConst(&tmpString, tmpChar);
				
				
				// Converting instructions
				add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$dec
				add_instruction(INT2FLOATS, NULL, NULL, NULL);
				add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$dec
								
				// Update tokStack
				tokStack_Push(tokStack, TOK_decimal);
				
				// Free string memory
				strFree(&tmpString);
			}
			else if(typeLeft == TOK_decimal && typeRight == TOK_integer)	// dec # (int) = dec
			{
				/* Example:
				[DEC + INT = DEC]
				DEC...b
				INT...a
				-------------
				INT2FLOATS
				ADDS
				*/
				
				// Converting instruction
				add_instruction(INT2FLOATS, NULL, NULL, NULL);
				
				// Update tokStack
				tokStack_Push(tokStack, TOK_decimal);
			}
			else
			{
				expr_error("expr_convertTypes: Invalid data types combination");
				return; // @todo
			}
			break;
		}
		default: break;
	}
}

int expr_convertResultType(tokStack_t *tokStack, type_t el_type)
{
	// Check if result is same type as result varaible
	tokenType_t resVarType = expr_elTypeConvert(el_type);	// Result variable type
	tokenType_t resType = tokStack_Top(tokStack);	// Result expression type
	
	// Couldn't findout result variable type
	if(resVarType == TOK_FAIL)
	{
		expr_error("expr_convertResultType: Fail in expr_elTypeConvert");
		return EXPR_RETURN_ERROR_INTERNAL;
	}
        
	// Result expression and result variable are the same type
	if(resVarType == resType)
	{
		DEBUG_PRINT("expr_convertResultType: same type\n");
		return EXPR_RETURN_SUCC;
	}
        	  	
        	
	if(resVarType == TOK_integer && resType == TOK_decimal)	// int = dec
		add_instruction(FLOAT2R2EINTS, NULL, NULL, NULL);
	else if(resVarType == TOK_integer && resType == TOK_decimal)	// dec = int
		add_instruction(INT2FLOATS, NULL, NULL, NULL);
	else
	{
		expr_error("expr_convertResultType: Not compatible or convertable data types");
		return EXPR_RETURN_ERROR_TYPES;
	}	
}

tokenType_t expr_elTypeConvert(type_t el_type)
{
	switch(el_type)
	{
		case KW_integer:	return TOK_integer;	break;
		case KW_double:	return TOK_decimal;	break;
		case KW_string:	return TOK_string;	break;
		default:
			expr_error("expr_elTypeConvert: Invalid el_type");
			return TOK_FAIL;	// Represents error			
	}
}

// ========== OTHER FUNCTIONS ==========

void expr_error(char *msg)
{
	fprintf(stderr, "[ERROR] %s\n", msg);
}

int expr_generateResult(tokStack_t *tokStack, int context, st_globalTable_t *st_global, string *func_name, st_element_t *variable)
{
    DEBUG_PRINT("[DBG] Generating result\n");
    int retVal;
        
	switch(context)
	{
		// ===== Expression context assignment =====
		case EXPRESSION_CONTEXT_ASSIGN:
			// Check if variable exists
			if(variable == NULL)
			{
				expr_error("expr_generateResult: Result variable doesn't exist... Strange :O");
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return(EXPR_RETURN_ERROR_INTERNAL);
			}
			
			// Convert result to be same data type as result variable
			retVal = expr_convertResultType(tokStack, variable->el_type);
			if(retVal != EXPR_RETURN_SUCC)
			{
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return retVal;
			}
			
			// === Add instruction === (pop/move result value to variable)
			// --- Integer/Decimal value ---
			if(tokStack_Top(tokStack) != TOK_string)
			{
				add_instruction(POPS, NULL, &variable->key, NULL);	// Pop from stack to variable
			}
			else
			// --- String value ---
			{
				// Temporary varaible for strings
				string *varString;
				strInit(varString);
				char *varChar = "$str";
				strCopyConst(varString, varChar);
				
				// Temporary token for result variable
				token_t *resToken = TokenInit();
				resToken->type = TOK_identifier;
				strCopyString(resToken->value.stringVal, &variable->key);	// Simulate identifier token for result variable
	
				// Move result to result variable
				add_instruction(MOVE_LF_LF, resToken, varString, NULL);	// MOVE LF@result LF@$str
	
				// Free memory
				strFree(varString);
				TokenFree(resToken);
			}
			break;
		
		
		// ===== Expression context logic =====	
		case EXPRESSION_CONTEXT_LOGIC:
			add_instruction(JUMPIFEQS, NULL, NULL, NULL);
			break;
		
		
		// ===== Expression context print =====	
		case EXPRESSION_CONTEXT_PRINT:
		{
			// Get type of token on top of the stack
			tokenType_t topType = tokStack_Top(tokStack);
			if(topType == TOK_FAIL)
			{
				expr_error("expr_generateResult: Can't get top of the token stack");
				return EXPR_RETURN_ERROR_INTERNAL;				
			}
			
			// Create varaible name string
			string *varString;
			strInit(varString);
			char varChar[4]; 
			switch(topType)
			{
				case TOK_integer:	strcpy(varChar, "$int");
				case TOK_decimal:	strcpy(varChar, "$dec");
				case TOK_string:	strcpy(varChar, "$str");
				default:
					expr_error("expr_generateResult: Wrong token type on top of the stack");
					return EXPR_RETURN_ERROR_INTERNAL;
			}
			strCopyConst(varString, varChar);
			
			// Add print instruction
			add_instruction(WRITE, NULL, varString, NULL);
		
			// Free memory
			strFree(varString);
			break;
		}
		
		// ===== Expression context return =====	
		case EXPRESSION_CONTEXT_RETURN:
		{
			// --- Search for function ---
			st_localTable_t *function = st_find_func(st_global, func_name);
			
			// Check if function exists
			if(function == NULL)
			{
				expr_error("expr_generateResult: Function not found");
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return(EXPR_RETURN_ERROR_INTERNAL);			
			}
			
			
			// --- Convert result value data type --- (to match result variable)
			retVal = expr_convertResultType(tokStack, variable->el_type);
			if(retVal != EXPR_RETURN_SUCC)
			{
				DEBUG_PRINT("--- Expression module end (error) ---\n");
				return retVal;
			}
			
			// --- Add instruction --- (pop result value to variable)
			add_instruction(RETVAL_POP, NULL, NULL, NULL);
			
			break;
		}
	}
	
	DEBUG_PRINT("--- Expression module end (success)---\n");
	return EXPR_RETURN_SUCC;
}


int expr_finishAlgorithm(myStack_t *stack, tokStack_t *tokStack, token_t token, int context)
{
	while(expr_isAlgotihmFinished(stack, token.type) == EXPR_FALSE)  // Should algorithm continue?        
	{
		// --- Token indicating stop of loading ---
		token_t noMoreTokens;
		noMoreTokens.type = EXPR_RETURN_NOMORETOKENS; // (Not really TOK_endOfFile, see header file at expr_algorithm())

		// --- Continue with the algorithm ---
		int retVal;     // Return value of the algorithm
		retVal = expr_algorithm(stack, tokStack, noMoreTokens, context, EXPR_FALSE);  

		// --- Check for error ---
		if(retVal != EXPR_RETURN_SUCC)
		{
			DEBUG_PRINT("--- Expression module end (error) ---\n");
			return retVal;
		}
	}	
	return EXPR_RETURN_SUCC;
}
