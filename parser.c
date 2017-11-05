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
**/
int program(){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //In global variable with type token_t will be stored token from scanner
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }

    //Skip all empty lines at beggining of code/between declarations
    while(CurrentToken.type == TOK_endOfLine){
        if(getToken(&CurrentToken) == LEX_ERROR){
            return LEX_ERROR;
        }
    }
    //Switch rule
    switch(CurrentToken.type){
        //<prog> -> EOF
        case TOK_endOfFile:
            if (DecOrDefAndEOF){
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
    }
}
/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 **/
int FunctionDeclar(){
    string FunctionID;
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
    if (strCopyString(&FunctionID, (CurrentToken.value.stringVal)) == 1){
        return INTERNAL_ERROR;
    }
    //Praca s ID vlozit do hash table...//TODO

    //LEFT_BRACKET
    if (getToken(&CurrentToken) == LEX_ERROR){
        return LEX_ERROR;
    }
    if (CurrentToken.type != TOK_lParenth){
        return SYN_ERROR;
    }
    //<function-args>
    if (getToken(&CurrentToken) )
}

int ScopeBody(){


}

int main(){
    return 0;

};
