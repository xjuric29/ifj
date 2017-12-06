/**
 * @file expr.c
 * @author Jiri Furda (xfurda00)
 * @brief Source file for precedent analysis of expressions
 */

// --- TESTING ---
//#define DEBUG   // Print stack, operations and table indexes


// Header file
#include "expr.h"




int algotihmFinished = EXPR_FALSE;  // Used static because I was lost in recursion                     first str  <  second str
int firstString = EXPR_TRUE;  // Indicates if we are working with first string or second string (e.g. str1 + str2 < str3 + str4)
int skipJUMPIFEQS = EXPR_FALSE;	// If true then it skips the generate result instrucion because it's done in ilist.c (used in <= and >= for int or dec)

/**
 * @brief Precedental table determinating next action.
 * @see https://docs.google.com/spreadsheets/d/1S_9ILug0ak7JUh5eQEjLLjyhXUuf_DkzCt4iOQbTlP8/edit?usp=sharing
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
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<'},  // +
  {'>', '>', '<', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#'},  // -
  {'>', '>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#'},  // \    // The space after '\' is important
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#'},  // *
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#'},  // /
  {'<', '<', '<', '<', '<', '<', '=', '<', '#', '<', '<', '<', '<', '<', '<', '<'},  // (
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#'},  // )
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#'},  // i
  {'<', '<', '<', '<', '<', '<', '#', '<', '#', '<', '<', '<', '<', '<', '<', '<'},  // $
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // =
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <>
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // <=
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // >
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<'},  // >=
  {'>', '#', '#', '#', '#', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#'},  // str
};


// ========== CORE FUNCTIONS ==========

int expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *variable)
{
	DEBUG_PRINT("--- Expression module start ---\n");
	DEBUG_PRINT("Context: %d\n", context);

	

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
 	int firstToken = EXPR_TRUE;     // Indicates if the processed token is the first in the expression
 	int resetTempStr = EXPR_TRUE;	// Deterimines if reset of $str is necessary
 	// Reset global variables
	algotihmFinished = EXPR_FALSE;
	firstString = EXPR_TRUE;
	skipJUMPIFEQS = EXPR_FALSE;


	// --- Initializing stacks ---
	myStack_t stack;	// Create terminal stack
	stackInit(&stack);	// Init stack (Push "$")
	tokStack_t tokStack;

	int success;
	success = tokStack_Init(&tokStack);
	if(success == FAIL)
	{
		expr_error("expr_main: Couldn't init token stack");
		DEBUG_PRINT("--- Expression module end (error #01) ---\n");
		return EXPR_RETURN_ERROR_INTERNAL;
	}


	// --- Loading first token ---
	token_t loadedToken;	// Loaded token for this iterration of cycle
	loadedToken = *parserToken;	// It has no purpose, it's just to be more read friendly

	if(context != EXPRESSION_CONTEXT_ASSIGN)	// If context is arithmetic, first token is already loaded by parser
	{
		// Load token from scanner
		int scannerReturn = getToken(&loadedToken);
		if (scannerReturn != SUCCESS)
		{
			expr_error("expr_main: Scanner error");
			return scannerReturn;
		}
	}
	DEBUG_PRINT("[DBG] First token with type %d\n", loadedToken.type);


	// --- Check first token type ---
	if(expr_isFirstValid(loadedToken) == EXPR_FALSE) // Can be token used as beginning of an expression?
	{
		if(context == EXPRESSION_CONTEXT_PRINT)	// If found semicolon in print context
		{
			*parserToken = loadedToken;	// Save token for parser
			DEBUG_PRINT("--- Expression module success ---\n");
			return EXPR_RETURN_SUCC;	// Return to parser
		}
		else
		{
			expr_error("expr_main: Wrong first token");
			DEBUG_PRINT("--- Expression module end (error #02) ---\n");
			return EXPR_RETURN_ERROR_SYNTAX;        // If not -> Syntax error
		}
	}

	// --- Loading tokens ---
	while(continueLoading)
	{
		DEBUG_PRINT("[DBG] ---Loading token (type=%d)---\n", loadedToken.type);

        int skipMaskingAsID = EXPR_FALSE;	// Reset variable to default

		// --- Search for varibale in symbol table ---
		if(loadedToken.type == TOK_identifier)  // If token is variable
		{
			st_element_t *element = st_find_element(st_global, func_name, loadedToken.value.stringVal);

			// --- Check if variable exists ---
			if(element == NULL)   // Haven't found it in the table
			{
				expr_error("expr_main: Tried to work with nonexisting variable");
				DEBUG_PRINT("--- Expression module end (error #02) ---\n");
				return EXPR_RETURN_ERROR_SEM;   // Return semantics error
			}

			// --- Find out variable type ---
			tokenType_t tokenType = expr_elTypeConvert(element->el_type);
			//DEBUG_PRINT("tokenType %d\n", tokenType);

			if(tokenType == TOK_FAIL)
			{
				// Error message already printed in expr_elTypeConvert()
				DEBUG_PRINT("--- Expression module end (error #03) ---\n");
				return EXPR_RETURN_ERROR_INTERNAL;
			}
			else
			{
				// Push variable type to the stack
				// (If it's value and not variable, then this is done in expr_algorithm())
				//DEBUG_PRINT("Pushing on tokStack type %d\n", tokenType);
				tokStack_Push(&tokStack, tokenType);

				if(tokenType == TOK_string)
					skipMaskingAsID = EXPR_TRUE;	// Can't save string as TERM_identifier because it has it's own teriminal TERM_string
			}
		}




		// --- CORE OF THE FUNCTION ---
		int retVal;	// Internal terminal type
		retVal = expr_algorithm(&stack, &tokStack, loadedToken, context, skipMaskingAsID, &resetTempStr);	// Use algorithm on the loaded token
		if(retVal == EXPR_RETURN_STARTOVER)
		{
			DEBUG_PRINT("{DBG} Startover\n");
			// Found semicolon in print -> Start process over
			expr_generateResult(&tokStack, context, st_global, func_name, variable);
			return expr_main(context, parserToken, st_global, func_name, variable);
		}

		if(retVal == EXPR_RETURN_NOMORETOKENS)    // EXPR_RETURN_NOMORETOKENS = Found token that doesn't belong to expression anymore
		{
			DEBUG_PRINT("{DBG} No more tokens\n");
			
			// --- End of expression ---
			continueLoading = 0;	// Stop the loading cycle
			*parserToken = loadedToken;	// Save token for parser to proceed 
			
			if(context == EXPRESSION_CONTEXT_PRINT)	// Found different token than string, plus or semicolon in print context
			{
				expr_error("expr_main: Found invalid token in print (semicolon is missing)");
				return EXPR_RETURN_ERROR_SYNTAX;
			}
		}
		else
		{
			DEBUG_PRINT("{DBG} Else\n");
			// --- Check for error ---
			if(retVal != EXPR_RETURN_SUCC)  // If an error occurred
			{
				DEBUG_PRINT("--- Expression module end (error #04) ---\n");
				return retVal;  // End module and report error
			}

			//DEBUG_PRINT("[DBG] retVal = %d\n", retVal);

			// --- Load next token ---
			DEBUG_PRINT("{DBG} getToken\n");
			int scannerReturn = getToken(&loadedToken);
			if (scannerReturn != SUCCESS)
			{
				expr_error("expr_main: Scanner error");
				return scannerReturn;
			}
		}
	}
	DEBUG_PRINT("[DBG] Loading done\n");


	// --- Finish the algorith ---
	// (This happens when there are no more tokens to load but algorithm is still not finished)
	int retVal = expr_finishAlgorithm(&stack, &tokStack, loadedToken, context, &resetTempStr);
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

int expr_algorithm(myStack_t *stack, tokStack_t *tokStack, token_t token, int context, int skipMaskingAsID, int *resetTempStr)
{
	static token_t savedToken;	// Save token if it had some tokenValue_t (identifier/integer/decimal/string)

	// Check if algortihm is finished
	if(expr_isAlgotihmFinished(stack, token.type) == EXPR_TRUE)
		return EXPR_RETURN_SUCC;

	// Initializing varables
	precTableIndex_t type;	// Internal type of token = Column index
	int logicOperator = EXPR_FALSE;

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
				return EXPR_RETURN_ERROR_TYPES;
			}
			break;
                        
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
        case TOK_equal:	type = TERM_equal;	logicOperator = EXPR_TRUE;	break;			// Operator "="
		case TOK_notEqual:	type = TERM_notEqual;	logicOperator = EXPR_TRUE;	break;		// Operator "<>"
		case TOK_less:	type = TERM_less;	logicOperator = EXPR_TRUE;	break;			// Operator "<"
		case TOK_lessEqual:	type = TERM_lessEqual;	logicOperator = EXPR_TRUE;	break;		// Operator "<="
		case TOK_greater:	type = TERM_greater;	logicOperator = EXPR_TRUE;	break;		// Operator ">"
		case TOK_greaterEqual:	type = TERM_greaterEqual;	logicOperator = EXPR_TRUE;	break;	// Operator ">="

		// === Tokens with values ===
		// --- Numbers ---
		case TOK_integer: // Both represented by 'i', they have the same behavior
		case TOK_decimal:
			type = TERM_id;	// Identifier terminal = 'i'
			savedToken = token;	// Save token because it has value
			tokStack_Push(tokStack, token.type);
			break;
		// --- Variables ---
		case TOK_identifier:
			savedToken = token; // Save token because it has value
			if(skipMaskingAsID == EXPR_FALSE)
			{
				type = TERM_id;	// Identifier terminal = 'i'
				break;
			}
			// WATCHOUT!!! If token is string variable then it doesn't break here and continue to case TOK_string!!!
		// --- Strings ---
		case TOK_string:
			// Setting things for algorithm
			type = TERM_string;
			savedToken = token;	// Save token because it has value
			
			
			// ----- Concating string -----
			// (It's done here instead of expr_reduce() because of first string not printed bug)
			// Creating name string for temporary variable
			string varString;
			strInit(&varString);

			// --- Switch from $str to $str2 (if necessary) ---
			// It's not cancating (term != +) of strings or it's comparasion (logicOperator == EXPR_TRUE)
			if((stackGetTerminal(stack) != '+' && tokStack_Empty(tokStack) == FALSE && *resetTempStr == EXPR_FALSE) || logicOperator == EXPR_TRUE) 
			{
				if(firstString == EXPR_TRUE)	// Switch $str to $str2
				{
					firstString = EXPR_FALSE;	// Use $str2
					*resetTempStr = EXPR_TRUE;	// Set flag to reset $str2
				}
				else
				{
					DEBUG_PRINT("[DBG] @todo Risking some error leak");
					//expr_error("expr_algorithm: Third string is not allowed unless concating (not exiting yet, module should detect this error later");
				}
			}


			// --- Choose $str or $str2 ---
			if(firstString == EXPR_TRUE)	// Determines if using first temporary string variable or second ($str or $str2)
			{
				char *varChar = "$str";
				strCopyConst(&varString, varChar);
			}
			else
			{
				char *var2Char = "$str2";
				strCopyConst(&varString, var2Char);
			}


			// --- Reset temporary string value ---
			if(*resetTempStr == EXPR_TRUE)
			{
				// Creating token with empty string
				token_t *resetToken = TokenInit(); // Initialize token
				resetToken->type = TOK_string;	
				
				// Add instruction to reset temporary string
				add_instruction(MOVE_LF_LF, resetToken, &varString, NULL);

				// Change reset flag (don't reset $str)
				*resetTempStr = EXPR_FALSE;

				// Free memory
				TokenFree(resetToken);
			}
			

			// Concating string in 3AC
			add_instruction(CONCAT, &token, &varString, NULL);	// Add string at end of the temporary string

			// Updating token stack
			tokStack_Push(tokStack, TOK_string);

			// Free memory
			strFree(&varString);
			break;

        // === Other tokens ===
        case EXPR_RETURN_NOMORETOKENS:
				type = TERM_stackEnd;	// End of input terminal '$'
			break;
        // --- Semicolon ---
        case TOK_semicolon:
			if(context == EXPRESSION_CONTEXT_PRINT)
			{
				// Finish the algorithm
				int retVal = expr_finishAlgorithm(stack, tokStack, token, context, resetTempStr);
				if(retVal != EXPR_RETURN_SUCC)
					return retVal;

				// @todo Print write instruction (maybe done in expr_main)

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
		case ACTION_shift:
			return expr_shift(stack, expr_getCharFromIndex(type));

                // Operation REDUCE '>'
		case ACTION_reduce:
		{
			int retVal;    // Return value of reducing (= searching for rule)
			retVal = expr_reduce(stack, tokStack, savedToken);

			if(retVal != EXPR_RETURN_SUCC) // If rule not found
				return retVal;        // Return syntax error

			// Otherwise continue with algorithm
			return expr_algorithm(stack, tokStack, token, context, skipMaskingAsID, resetTempStr);       // Use recursion (don't ask why, that's just the way it should be)
		}

		// Operation SPECIAL SHIFT '='
		case ACTION_specialShift:
			return expr_specialShift(stack, expr_getCharFromIndex(type));

		// ILLEGAL OPERATON '#'
		case ACTION_illegal:
			if(row == TERM_string && type > TERM_plus && type < TERM_lBrac)	// Stack is string and operation is arithmetic (and not plus)
			{
				expr_error("expr_algorithm: Operation not suitable for string operands");
				return EXPR_RETURN_ERROR_TYPES;        // Return types error				
			}
			else
			{
				expr_error("expr_algorithm: Tried to perform an illegal action");
				return EXPR_RETURN_ERROR_SYNTAX;        // Return syntax error
			}
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
		case '#':	return ACTION_illegal;

		default:	// Invalid character
			expr_error("expr_readTable: Invalid character in the precedent table");
			return ACTION_illegal;
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
	DEBUG_PRINT("[DBG] -[Operation <]-\n");

	stackShiftPush(stack); // Push '<' after closest terminal to the end of the stack
	stackPush(stack, character);    // Push the terminal at the end of the stack

	stackInfo(stack);	// Debug

	return EXPR_RETURN_SUCC;        // @todo return values for error when working with stack
}

int expr_reduce(myStack_t *stack, tokStack_t *tokStack, token_t token)
{
	DEBUG_PRINT("[DBG] -[Operation >]-\n");

	// Initialize variable for handle
	string handle;	// Right side of grammar rule
	strInit(&handle);	// Init string


	// Generating instruction
	char terminal = stackGetTerminal(stack);
	int retVal;
	retVal = expr_generateInstruction(tokStack, terminal, token);
	if(retVal != EXPR_RETURN_SUCC)
		return retVal;


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
	DEBUG_PRINT("[DBG] -[Operation =]-\n");

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

	if(stack->top == 1 && stackTop(stack) == 'E' && tokenType == EXPR_RETURN_NOMORETOKENS)  // If finishing now
	{
		algotihmFinished = EXPR_TRUE;   // Change static int
		return EXPR_TRUE;
	}
	stackInfo(stack);

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
		DEBUG_PRINT("[DBG] expr_isFirstValid(): First token is not suitable for being first in expression\n");
		return EXPR_FALSE;
	}
}

int expr_generateInstruction(tokStack_t *tokStack, char terminal, token_t token) // @todo
{
	// --- Converting types ---
	tokenType_t topType = tokStack_Top(tokStack);
	if((topType == TOK_integer || topType == TOK_decimal || topType == TOK_string) && terminal != 'i' && terminal != TERM_string && terminal != ')')	// No need to convert identifier (int and dec) and string && no converting when reducing i to E or str to E
	{
		int retVal;
		retVal = expr_convertTypes(tokStack, terminal);
		if(retVal != EXPR_RETURN_SUCC)
			return retVal;
	}
	
	
	// --- Add instruction ---
	switch(terminal)
	{
	// Operators
	case '+':
		if(tokStack_Top(tokStack) != TOK_string)	// Operator '+' used as arithmetic plus
			add_instruction(ADDS, NULL, NULL, NULL);
		// If it's used for strings then it is already concated upon loading (kinda strange but easier)
		break;
	case '-':
		add_instruction(SUBS, NULL, NULL, NULL);
		break;
	case '*':
		add_instruction(MULS, NULL, NULL, NULL);
		break;
	case '/':
		add_instruction(DIVS, NULL, NULL, NULL);
		break;
	case '\\':	
		add_instruction(DIVS, NULL, NULL, NULL);	// Div have always dec result
		add_instruction(FLOAT2INTS, NULL, NULL, NULL);	// But divInt must have int result @todo TRUCNATE
		// Update token stack
		tokStack_Pop(tokStack);
		tokStack_Push(tokStack, TOK_integer);
		break;

	// Identifier / integer / decimal
	case 'i':
		add_instruction(PUSHS, &token, NULL, NULL);
		break;

	// Logic operators
	case TERM_equal:
		if(topType == TOK_string)
			add_instruction(EQ, NULL, NULL, NULL);
		else
			add_instruction(EQS, NULL, NULL, NULL);
		break;
	case TERM_notEqual:
		if(topType == TOK_string)
			add_instruction(NOTEQ, NULL, NULL, NULL);	// Nonexisting instruction, but ilist does some magic
		else
		{
			add_instruction(EQS, NULL, NULL, NULL);
			add_instruction(NOTS, NULL, NULL, NULL);
		}
		break;
	case TERM_less:
		if(topType == TOK_string)
			add_instruction(LT, NULL, NULL, NULL);
		else
			add_instruction(LTS, NULL, NULL, NULL);
		break;
	case TERM_lessEqual:
                skipJUMPIFEQS = EXPR_TRUE;      // Do not generate result instruction (it is done in ilist.c)
                
		if(topType == TOK_string)
			add_instruction(LTEQ, NULL, NULL, NULL);	// Nonexisting instruction, but ilist does some magic
		else
			add_instruction(LTEQS, NULL, NULL, NULL);	// Nonexisting instruction, but ilist does some magic
		break;
	case TERM_greater:
		if(topType == TOK_string)
			add_instruction(GT, NULL, NULL, NULL);
		else
			add_instruction(GTS, NULL, NULL, NULL);
		break;
	case TERM_greaterEqual:
                skipJUMPIFEQS = EXPR_TRUE;      // Do not generate result instruction (it is done in ilist.c)
                
		if(topType == TOK_string)
			add_instruction(GTEQ, NULL, NULL, NULL);	// Nonexisting instruction, but ilist does some magic
		else
			add_instruction(GTEQS, NULL, NULL, NULL);	// Nonexisting instruction, but ilist does some magic
		break;
		
	// Terminals generating no instruction
	case ')':
	case TERM_string:	// No need to add instructuon (already done while loading token)
		break;	
		
	// Invalid terminals
	default:
		expr_error("generateInstructuon: Unexpected terminal");
		return EXPR_RETURN_ERROR_INTERNAL;
	}
	
	return EXPR_RETURN_SUCC;
}





// ========== CONVERT FUNCTIONS ==========

int expr_convertTypes(tokStack_t *tokStack, char terminal)
{
	// Determine if it's logic operation
	int logicOperator;
	switch(terminal)
	{
		case TERM_equal:
		case TERM_notEqual:
		case TERM_less:
		case TERM_lessEqual:
		case TERM_greater:
		case TERM_greaterEqual:	
			logicOperator = EXPR_TRUE;
			break;
		default:
			logicOperator = EXPR_FALSE;
			break;
	}
	
	
	// Operand types to be converted
	tokenType_t typeRight = tokStack_Pop(tokStack);
	tokenType_t typeLeft = tokStack_Pop(tokStack);
	
	// Check for error
	if(typeRight == TOK_FAIL || typeLeft == TOK_FAIL)
	{
		// Kinda hacks the automatic tests - Detects syntax error only in basic expressions
		expr_error("convertTypes: Couldn't load operands from tokStack (wrong syntax in expression?)");
		return EXPR_RETURN_ERROR_SYNTAX;
	}

	// --- Operands conversion ---
	if(typeLeft == TOK_integer && typeRight == TOK_integer)	// int # int
	{
		if(terminal != '/')	//  If not using divade then result is int
			tokStack_Push(tokStack, TOK_integer);
		else	// If using divade then operands must be decimal
		{
			// Prepare string with temporary variable
			string tmpString;
			strInit(&tmpString);
			char *tmpChar = "$int";
			strCopyConst(&tmpString, tmpChar);

			// Converting instructions
			add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$int
			add_instruction(INT2FLOATS, NULL, NULL, NULL);	// INT2FLOATS
			add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$int
			add_instruction(INT2FLOATS, NULL, NULL, NULL);	// INT2FLOATS

			// Update tokStack
			tokStack_Push(tokStack, TOK_decimal);	// Result of div is always dec

			// Free string memory
			strFree(&tmpString);
		}
	}
	else if(typeLeft == TOK_decimal && typeRight == TOK_decimal)	// dec # dec = dec
	{
		if(terminal == '\\')	// divInt need int on both sides
		{
			// Prepare string with temporary variable
			string tmpString;
			strInit(&tmpString);
			char *tmpChar = "$dec";
			strCopyConst(&tmpString, tmpChar);

			// Converting instructions
			add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$dec
			add_instruction(FLOAT2R2EINTS, NULL, NULL, NULL);	// FLOAT2R2EINTS
			add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$dec
			add_instruction(FLOAT2R2EINTS, NULL, NULL, NULL);	// FLOAT2R2EINTS

			// Update tokStack
			tokStack_Push(tokStack, TOK_integer);

			// Free string memory
			strFree(&tmpString);	
		}
		else	// Other instructions need dec on both sides
			tokStack_Push(tokStack, TOK_decimal);
	}
	else if(typeLeft == TOK_string && typeRight == TOK_string)	// str # str
	{
		tokStack_Push(tokStack, TOK_string);	
	}
	else if(typeLeft == TOK_integer && typeRight == TOK_decimal)	// (int) # dec
	{
		if(terminal == '\\')	// divInt need int on both sides
		{
			// Convert decimal to integer
			add_instruction(FLOAT2R2EINTS, NULL, NULL, NULL);
			
			// Update tokStack
			tokStack_Push(tokStack, TOK_integer);
		}
		else	// Other instructions need dec on both sides
		{
			// Prepare string with temporary variable
			string tmpString;
			strInit(&tmpString);
			char *tmpChar = "$dec";
			strCopyConst(&tmpString, tmpChar);

			// Converting instructions
			add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$dec
			add_instruction(INT2FLOATS, NULL, NULL, NULL);	// INT2FLOATS
			add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$dec

			// Update tokStack
			tokStack_Push(tokStack, TOK_decimal);

			// Free string memory
			strFree(&tmpString);
			
			// Save types after conversion (used for duplication)
			typeLeft = TOK_decimal; 
		}
	}
	else if(typeLeft == TOK_decimal && typeRight == TOK_integer)	// dec # (int)
	{
		if(terminal == '\\')	// divInt need int on both sides
		{
			// Prepare string with temporary variable
			string tmpString;
			strInit(&tmpString);
			char *tmpChar = "$int";
			strCopyConst(&tmpString, tmpChar);

			// Converting instructions
			add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$int
			add_instruction(FLOAT2R2EINTS, NULL, NULL, NULL);	// FLOAT2R2EINTS
			add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$int

			// Update tokStack
			tokStack_Push(tokStack, TOK_integer);

			// Free string memory
			strFree(&tmpString);			
		}
		else	// Other instructions need dec on both sides
		{
			// Converting instruction
			add_instruction(INT2FLOATS, NULL, NULL, NULL);

			// Update tokStack
			tokStack_Push(tokStack, TOK_decimal);
			
			// Save types after conversion (used for duplication)
			typeRight = TOK_decimal; 
		}
	}
	else
	{
		expr_error("expr_convertTypes: Invalid data types combination");
		return EXPR_RETURN_ERROR_TYPES;
	}
	
	
	// --- Extra steps for special cases ---
	if(logicOperator == EXPR_TRUE)	// If this is logic operation
	{
		tokStack_Pop(tokStack);	// Remove previous tokenType
		tokStack_Push(tokStack, TOK_BOOLEAN);	// Push boolean type (result of logic operations is always bool)
		
		// --- Duplicating operands on the stack ---
		if((terminal == TERM_greaterEqual || terminal == TERM_lessEqual) && (typeLeft != TOK_string && typeRight != TOK_string))	// If instruction is made from two comparsions combined 
		{			
			// Create name strings for temporary variables
			string leftString;
			string rightString;
			if(strInit(&leftString) == STR_ERROR || strInit(&rightString) == STR_ERROR)
			{
				expr_error("expr_convertTypes: Couldn't init strings");
				return EXPR_RETURN_ERROR_INTERNAL;
			}
		
			
			// Pop right operand to temporary variable
			char rightChar[6];
			switch(typeRight)
			{
				case TOK_integer:	strcpy(rightChar, "$int2");	break;
				case TOK_decimal:	strcpy(rightChar, "$dec2");	break;
				case TOK_string:	break;	// No need to duplicate strings, they are already in $str and $str2
				default:
					expr_error("expr_convertTypes: Right operadns is invalid type");
					strFree(&leftString);
					strFree(&rightString);
					return EXPR_RETURN_ERROR_INTERNAL;
			}
			strCopyConst(&rightString, rightChar);
			add_instruction(POPS, NULL, &rightString, NULL);	// POPS LF@$int or POPS LF@$dec
		
		
			// Pop left operand to temporary variable
			char leftChar[5];
			switch(typeLeft)
			{
				case TOK_integer:	strcpy(leftChar, "$int");	break;
				case TOK_decimal:	strcpy(leftChar, "$dec");	break;
				case TOK_string:	break;	// No need to duplicate strings, they are already in $str and $str2
				default:
					expr_error("expr_convertTypes: Left operadns is invalid type");
					strFree(&leftString);
					strFree(&rightString);
					return EXPR_RETURN_ERROR_INTERNAL;
			}
			strCopyConst(&leftString, leftChar);
			add_instruction(POPS, NULL, &leftString, NULL);	// POPS LF@$int or POPS LF@$dec
		
		
			// Push operands back to stack two times (duplication)
			add_instruction(PUSHS, NULL, &leftString, NULL);	// PUSHS LF@$int or POPS LF@$dec
			add_instruction(PUSHS, NULL, &rightString, NULL);	// PUSHS LF@$int or POPS LF@$dec
			add_instruction(PUSHS, NULL, &leftString, NULL);	// PUSHS LF@$int or POPS LF@$dec
			add_instruction(PUSHS, NULL, &rightString, NULL);	// PUSHS LF@$int or POPS LF@$dec
		
		
			// Free memory
			strFree(&leftString);
			strFree(&rightString);
		}
		
	}
	else if(terminal == '\\')	// If operation is divInt
	{
		// Both operands are now int, but we can divide only decimal values

		// Prepare string with temporary variable
		string tmpString;
		strInit(&tmpString);
		char *tmpChar = "$dec";
		strCopyConst(&tmpString, tmpChar);

		// Converting instructions
		add_instruction(POPS, NULL, &tmpString, NULL);	// POPS LF@$dec
		add_instruction(INT2FLOATS, NULL, NULL, NULL);	// INT2FLOATS
		add_instruction(PUSHS, NULL, &tmpString, NULL);	// PUSHS LF@$dec
		add_instruction(INT2FLOATS, NULL, NULL, NULL);	// INT2FLOATS

		// Free string memory
		strFree(&tmpString);
	}
	
	return EXPR_RETURN_SUCC;
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
	else if(resVarType == TOK_decimal && resType == TOK_integer)	// dec = int
		add_instruction(INT2FLOATS, NULL, NULL, NULL);
	else
	{
		expr_error("expr_convertResultType: Not compatible or convertable data types");
		return EXPR_RETURN_ERROR_TYPES;
	}
}

tokenType_t expr_elTypeConvert(type_t el_type)
{
	//DEBUG_PRINT("eltype = %d\n", el_type);
	switch(el_type)
	{
		case st_integer:	return TOK_integer;	break;
		case st_decimal:	return TOK_decimal;	break;
		case st_string:	return TOK_string;	break;
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
				DEBUG_PRINT("--- Expression module end (error #05) ---\n");
				return(EXPR_RETURN_ERROR_INTERNAL);
			}

			// Convert result to be same data type as result variable
			retVal = expr_convertResultType(tokStack, variable->el_type);
			if(retVal != EXPR_RETURN_SUCC)
			{
				DEBUG_PRINT("--- Expression module end (error #06) ---\n");
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
				string varString;
				strInit(&varString);
				char *varChar = "$str";
				strCopyConst(&varString, varChar);

				// Token for temporary variable $str
				token_t *tmpToken = TokenInit();
				tmpToken->type = TOK_identifier;
				strCopyString(tmpToken->value.stringVal, &varString);	// Simulate identifier token for temporary variable

				// Move result to result variable
				add_instruction(MOVE_LF_LF, tmpToken, &variable->key, NULL);	// MOVE LF@result LF@$str

				// Free memory
				strFree(&varString);
				TokenFree(tmpToken);
			}
			break;


		// ===== Expression context logic =====
		case EXPRESSION_CONTEXT_LOGIC:
			if(tokStack_Top(tokStack) != TOK_BOOLEAN)
			{
				expr_error("expr_generateResult: Result of logic expression is not boolean");
				return EXPR_RETURN_ERROR_TYPES;
			}	
						
			if(skipJUMPIFEQS == EXPR_FALSE)
				add_instruction(JUMPIFEQS, NULL, NULL, NULL);
			break;


		// ===== Expression context print =====
		case EXPRESSION_CONTEXT_PRINT:
		{
			DEBUG_PRINT("{DBG} Generating print\n");
			
			// Get type of token on top of the stack
			tokenType_t topType = tokStack_Top(tokStack);
			if(topType == TOK_FAIL)
			{
				expr_error("expr_generateResult: Can't get top of the token stack");
				return EXPR_RETURN_ERROR_INTERNAL;
			}

			// Create name string for temporary variable
			string varString;
			if(strInit(&varString) == STR_ERROR)
			{
				expr_error("expr_generateResult: Couldn't init string");
				return EXPR_RETURN_ERROR_INTERNAL;
			}
			char varChar[5];
			switch(topType)
			{
				case TOK_integer:	strcpy(varChar, "$int");	break;
				case TOK_decimal:	strcpy(varChar, "$dec");	break;
				case TOK_string:	strcpy(varChar, "$str");	break;	// e.g. boolean = str + str < str + str isn't alllowed so we don't need $str2 temporaty variable
				default:
					DEBUG_PRINT("top type is %d\n",topType);
					expr_error("expr_generateResult: Wrong token type on top of the stack");
					strFree(&varString);
					return EXPR_RETURN_ERROR_INTERNAL;
			}
			strCopyConst(&varString, varChar);

			// Add instructions
			if(topType == TOK_integer || topType == TOK_decimal)	// If it's a number value
				add_instruction(POPS, NULL, &varString, NULL);	// Pop the value into temporary varaible (string is already in variable $str)
			add_instruction(WRITE, NULL, &varString, NULL);	// Write the result

			// Free memory
			strFree(&varString);
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
				DEBUG_PRINT("--- Expression module end (error #07) ---\n");
				return(EXPR_RETURN_ERROR_INTERNAL);
			}


			// --- Convert result value data type --- (to match result variable)
			retVal = expr_convertResultType(tokStack, function->func_type);
			if(retVal != EXPR_RETURN_SUCC)
			{
				DEBUG_PRINT("--- Expression module end (error #08) ---\n");
				return retVal;
			}

			// --- Add instruction ---
			if(tokStack_Top(tokStack) != TOK_string)	// Returning number value
				add_instruction(RETVAL_POP, NULL, NULL, NULL);	// Pop result value into return
			else	// Returning string value
			{
				// Name for temporary string variable
				string varString;
				strInit(&varString);
				char *varChar = "$str";
				strCopyConst(&varString, varChar);

				// Token for temporary variable $str
				token_t *tmpToken = TokenInit();
				tmpToken->type = TOK_identifier;
				strCopyString(tmpToken->value.stringVal, &varString);	// Simulate identifier token for temporary variable

				// Move result to result variable
				add_instruction(RETVAL_IN, tmpToken, NULL, NULL);	// Return $str

				// Free memory
				strFree(&varString);
				TokenFree(tmpToken);
			}
			break;
		}
	}

	DEBUG_PRINT("--- Expression module end (success)---\n");
	return EXPR_RETURN_SUCC;
}


int expr_finishAlgorithm(myStack_t *stack, tokStack_t *tokStack, token_t token, int context, int *resetTempExpr)
{
	while(expr_isAlgotihmFinished(stack, token.type) == EXPR_FALSE)  // Should algorithm continue?
	{
		// --- Token indicating stop of loading ---
		token_t noMoreTokens;
		noMoreTokens.type = EXPR_RETURN_NOMORETOKENS; // (Not really TOK_endOfFile, see header file at expr_algorithm())

		// --- Continue with the algorithm ---
		int retVal;     // Return value of the algorithm
		retVal = expr_algorithm(stack, tokStack, noMoreTokens, context, EXPR_FALSE, resetTempExpr);

		// --- Check for error ---
		if(retVal != EXPR_RETURN_SUCC && retVal != EXPR_RETURN_NOMORETOKENS)
		{
			DEBUG_PRINT("retVal=%d\n",retVal);
			DEBUG_PRINT("--- Expression module end (error #09) ---\n");
			return retVal;
		}
	}
	return EXPR_RETURN_SUCC;
}
