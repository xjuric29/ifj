#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

int parse();


/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @return type of error or succes
**/
int program(token_t CurrentToken);

/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 * @return type of error or succes
 **/
int FunctionDeclar(token_t CurrentToken);

/** @brief RULE:
  * <function-definition> -> FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL <function-body> FUNCTION EOL
  * FUNCTION was already check in <prog> so we start with ID
  * @return Type of error or success
  */
int FunctionDefinition(token_t CurrentToken);

/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @return type of error or succes
  **/
int FunctArgs(token_t CurrentToken);

/** @brief RULES:
  * 1) <more-function-args>    -> COMMA ID AS <data-type> <more-function-args>
  * 2) <more-function-args>    -> RIGHT_BRACKET
  * @return error type or success
  */
int MoreFunctArgs(token_t CurrentToken);

/**@brief: Function or Scope BODY
 * @return Type of error or SUCCESS
 **/
int Stats(token_t CurrentToken, bool InScope);

#endif
