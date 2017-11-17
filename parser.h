#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
<<<<<<< HEAD

/**Struct to check if we are inside Scope or While or If
  */
struct check{
    bool InScope;
    bool InWhile;
    bool InIf;
};
=======
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

int parse();


/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @return type of error or succes
**/
<<<<<<< HEAD
int program(token_t *CurrentToken, struct check ToCheck);
=======
int program(token_t CurrentToken);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 * @return type of error or succes
 **/
<<<<<<< HEAD
int FunctionDeclar(token_t *CurrentToken);
=======
int FunctionDeclar(token_t CurrentToken);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

/** @brief RULE:
  * <function-definition> -> FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL <function-body> FUNCTION EOL
  * FUNCTION was already check in <prog> so we start with ID
  * @param ToCheck is struct with variables to check if we are inside of While, If or Scope
  * @return Type of error or success
  */
<<<<<<< HEAD
int FunctionDefinition(token_t *CurrentToken, struct check ToCheck);
=======
int FunctionDefinition(token_t CurrentToken);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @return type of error or succes
  **/
<<<<<<< HEAD
int FunctArgs(token_t *CurrentToken);
=======
int FunctArgs(token_t CurrentToken);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

/** @brief RULES:
  * 1) <more-function-args>    -> COMMA ID AS <data-type> <more-function-args>
  * 2) <more-function-args>    -> RIGHT_BRACKET
  * @return error type or success
  */
<<<<<<< HEAD
int MoreFunctArgs(token_t *CurrentToken);
=======
int MoreFunctArgs(token_t CurrentToken);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

/**@brief: Function or Scope BODY
 * @return Type of error or SUCCESS
 **/
<<<<<<< HEAD
int Stats(token_t *CurrentToken, struct check ToCheck);

int WhileStat(token_t *CurrentToken, struct check ToCheck);

int IfStat(token_t *CurrentToken, struct check ToCheck);
=======
int Stats(token_t CurrentToken, bool InScope);
>>>>>>> e7fb055d0550973a614110a0bc68c2b460576d8a

#endif
