#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "symtab.h"

/**Struct to check if we are inside Scope or While or If
  * If we are entering If block, set InIf = true.. If inside If block comes While.. we Set While = True
  * and InIf = false so if comes Else its error, when we Get back from While block, InWhile = false and InIf == True..
  *
  */
struct check{
    bool InScope; //If comes token scope set to True
    bool InWhile; //If comes token while set to True
    bool InIf; //If comes token If set to True
    bool InElse; //If comes token Else set to True
};

int parse();


/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @return type of error or succes
**/

int program(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable);

/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 * @return type of error or succes
 **/
int FunctionDeclar(token_t *CurrentToken, st_globalTable_t *GlobalTable);

/** @brief RULE:
  * <function-definition> -> FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL <function-body> FUNCTION EOL
  * FUNCTION was already check in <prog> so we start with ID
  * @param ToCheck is struct with variables to check if we are inside of While, If or Scope
  * @return Type of error or success
  */
int FunctionDefinition(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable);

/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @return type of error or succes
  **/
int FunctArgs(token_t *CurrentToken, st_globalTable_t *GlobalTable);

/** @brief RULES:
  * 1) <more-function-args>    -> COMMA ID AS <data-type> <more-function-args>
  * 2) <more-function-args>    -> RIGHT_BRACKET
  * @return error type or success
  */
int MoreFunctArgs(token_t *CurrentToken, st_globalTable_t *GlobalTable);

/**@brief: Function or Scope BODY
 * @param Current
 * @return Type of error or SUCCESS
 **/
int Stats(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable);

/**
  * @brief: Function for dealing with while in program
  * RULE: DO WHILE <expresion> EOL <stats> LOOP EOL <stats>
  * @param ToCheck is struct with variables to check if we are inside of While, If or Scope
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return Type of error or SUCCESS
 **/
int WhileStat(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable);

/**@brief: Function for dealing with IF
  * RULE: IF <expresion> then EOL <stats> ELSE <stats> END IF EOL <stats>
  * @param ToCheck is structure with variables to check if we are inside of While, If or Scope
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return Type of error or SUCCESS
  */
int IfStat(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable);

/**
  * @brief: Function to check Function CalledFunction
  * Check arguments, types and conversions
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @param CalledFunction is pointer to function in Hash Table, function that is called
  **/
int FuncCallCheck(token_t *CurrentToken, st_globalTable_t *GlobalTable, st_localTable_t *CalledFunction);

#endif
