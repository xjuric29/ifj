/** @file parser.c
  * @autor Matej Stano (xstano04)
**/

#include <stdio.h>
#include <stdbool.h>
#include "parser.h"
#include "scanner.h"
#include "str.h"




int parse(){
    int Error;
    Error = program();
    return Error;
}



token_t CurrentToken;
bool DecOrDefAndEOF = false; //To check if program have scope and then eof


//TODO: Skontrolovanie prazdneho suboru

/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @return type of error or succes
**/
int program(){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //In global variable with type token_t will be stored token from scanner
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }

    //Skip all empty lines at beggining of code/between declarations
    while(CurrentToken.type == TOK_endOfLine){
        if (getToken(&CurrentToken) == LEX_ERROR){
            return LEX_ERROR;
        }
    }
    //Switch rule
    switch(CurrentToken.type){
        //<prog> -> EOF
        case TOK_endOfFile:
            if (DecOrDefAndEOF){ //If there was declaration or definition and no scope
                return SYN_ERROR;
            }
            return SUCCESS;

        //<prog>	-> <function-declaration> <prog>
        case KW_declare:
            RecurCallResult = FunctionDeclar();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            DecOrDefAndEOF = true; //?
            //<prog>
            RecurCallResult = program();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        //<prog>	-> <function-definition> <prog>
        case KW_function:
            RecurCallResult = FunctionDefinition();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            //prog
            RecurCallResult = program();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        // <prog>	-> SCOPE EOL <scope-body>
        case KW_scope:
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            if (CurrentToken.type != TOK_endOfLine){
                return SYN_ERROR; //Je to syntakticky error?
            }
            // <scope-body>
            RecurCallResult = ScopeBody();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        default:
            return SYN_ERROR;
    }
}

/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 * @return type of error or succes
 **/
int FunctionDeclar(){
    string FunctionID;
    int RecurCallResult = -1;
    //FUNCTION
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != KW_function){
        return SYN_ERROR;
    }

    //ID
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != TOK_identifier){
        return SYN_ERROR;
    }

    //Save ID to local variable FunctionID
    /*if (strCopyString(&FunctionID, (CurrentToken.value.stringVal)) == STR_ERROR){
        return INTERNAL_ERROR;
    }*/
    //Praca s ID vlozit do hash table...//TODO

    //LEFT_BRACKET
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != TOK_lParenth){
        return SYN_ERROR;
    }

    //<function-args>
    RecurCallResult = FunctArgs();
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }
    //AS
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != KW_as){
        return SYN_ERROR;
    }

    //<function-type>
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    switch(CurrentToken.type){
        case TOK_string:
        case TOK_decimal:
        case TOK_integer:
            //TODO ulozit niekde -> prerobit tento switch na vlastnu funkciu?

            break;

        default:
            return SYN_ERROR;
    }

    //EOL
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != TOK_endOfLine){
        return SYN_ERROR;
    }

    return SUCCESS;
}


/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @return type of error or succes
  **/
int FunctArgs(){
    int RecurCallResult = -1;
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    switch(CurrentToken.type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;
        //ID
        case TOK_identifier:
            //TODO Ulozit do tabulky,

            //AS
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            if (CurrentToken.type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            switch(CurrentToken.type){
                case TOK_string:
                case TOK_integer:
                case TOK_decimal:
                    //TODO pridelit do struktury funkcie

                RecurCallResult = MoreFunctArgs();
                if(RecurCallResult != SUCCESS){
                    return RecurCallResult;
                }
                break;
                //Token isn`t data-type
                default:
                    return SYN_ERROR;
            }
            break;
        //Token isn`t ) or ID
        default:
            return SYN_ERROR;
    }
    return SUCCESS;
}

/** @brief RULES:
  * <more-function-args>    -> COMMA ID AS <data-type> <more-function-args>
  * <more-function-args>    -> RIGHT_BRACKET
  * @return error type or success
  */
int MoreFunctArgs(){
    int RecurCallResult = -1;
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    switch(CurrentToken.type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;

        //COMMA ID AS ...
        case TOK_comma:
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            //ID
            if(CurrentToken.type != TOK_identifier){
                return SYN_ERROR;
            }
            //TODO ulozit do tabulky

            //AS
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            if (CurrentToken.type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if (getToken(&CurrentToken) == LEX_ERROR){
                return LEX_ERROR;
            }
            switch(CurrentToken.type){
                case TOK_integer:
                case TOK_string:
                case TOK_decimal:
                    //TODO pridelit do struktury tabulky

                default:
                    return SYN_ERROR;
            }

            //<more-function-args>
            RecurCallResult = MoreFunctArgs();
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            break;

        //Not comma or )
        default:
            return SYN_ERROR;
    }
    return SUCCESS;
}


/** @brief RULE:
  * <function-definition> -> FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL <function-body>
  * @return Type of error or success
  */
int FunctionDefinition(){
    printf("FUNCTION DEFINITION\n");
    return SUCCESS;
}

int ScopeBody(){
    printf("SCOPE BODY\n");
    return SUCCESS;
}


/**TEST**/

int a = 0;
int getToken(token_t *loadedToken){
    a++;
    if(a < 2){
    loadedToken->type = TOK_endOfLine;
    return SUCCESS;
    }
    if(a == 2){
        loadedToken->type = KW_declare;
        return SUCCESS;
    }
    if(a == 3){
        loadedToken->type = KW_function;
        return SUCCESS;
    }
    if(a == 4){
        loadedToken->type = TOK_identifier;
        return SUCCESS;
    }
    if(a == 5){
        loadedToken->type = TOK_lParenth;
        return SUCCESS;
    }
    if(a ==  6){
        loadedToken->type = TOK_rParenth;
        return SUCCESS;
    }
    if(a == 7){
        loadedToken->type = KW_as;
        return SUCCESS;
    }
    if(a == 8){
        loadedToken->type = TOK_string;
        return SUCCESS;
    }
    if(a >= 9 && a < 12){
        loadedToken->type = TOK_endOfLine;
        return SUCCESS;
    }
    if(a == 12){
        loadedToken->type = KW_scope;
        return SUCCESS;
    }
    if(a == 13){
        loadedToken->type = TOK_endOfLine;
        return SUCCESS;
    }
}

int main(){
    int ret = parse();
    printf("return %d\n", ret);
};
