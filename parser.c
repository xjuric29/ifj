/** @file parser.c
  * @autor Matej Stano (xstano04)
**/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"
#include "str.h"




//strInit(CurrentToken.value.stringVal);
bool DecOrDefAndEOF = false; //To check if program have scope and then eof
int ScannerInt; //For int returned by function getToken() to control LEX_ERROR or INTERNAL_ERROR;

int parse(){
    int Error;
    token_t CurrentToken;
    CurrentToken.value.stringVal = malloc(sizeof(string));
    strInit(CurrentToken.value.stringVal);
    Error = program(CurrentToken);
    strFree(CurrentToken.value.stringVal);
    free(CurrentToken.value.stringVal);
    return Error;

}

//TODO: Skontrolovanie prazdneho suboru

/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @return type of error or succes
**/
int program(token_t CurrentToken){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //In global variable with type token_t will be stored token from scanner
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    //Skip all empty lines at beggining of code/between declarations
    while(CurrentToken.type == TOK_endOfLine){
        if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
            return ScannerInt;
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
            RecurCallResult = FunctionDeclar(CurrentToken);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            DecOrDefAndEOF = true; //?
            //<prog>
            RecurCallResult = program(CurrentToken);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        //<prog>	-> <function-definition> <prog>
        case KW_function:
            RecurCallResult = FunctionDefinition(CurrentToken);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            //prog
            RecurCallResult = program(CurrentToken);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        // <prog>	-> SCOPE EOL <scope-body> SCOPE EOF
        case KW_scope:
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_endOfLine){
                return SYN_ERROR; //Je to syntakticky error?
            }
            // <scope-body>
            RecurCallResult = Stats(CurrentToken, true);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }

            //SCOPE
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != KW_scope){
                return SYN_ERROR;
            }

            //EOF
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //Skip EOLs..
            while(CurrentToken.type == TOK_endOfLine){
                if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                    return ScannerInt;
                }
            }
            if (CurrentToken.type != TOK_endOfFile){
                return SYN_ERROR;
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
int FunctionDeclar(token_t CurrentToken){
    string FunctionID;
    int RecurCallResult = -1;
    //FUNCTION
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != KW_function){
        return SYN_ERROR;
    }

    //ID
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
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
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != TOK_lParenth){
        return SYN_ERROR;
    }


    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //AS
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != KW_as){
        return SYN_ERROR;
    }

    //<function-type>
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }

    switch(CurrentToken.type){
        case KW_string:
        case KW_double:
        case KW_integer:
            //TODO ulozit niekde -> prerobit tento switch na vlastnu funkciu?

        break;

        default:
            return SYN_ERROR;
    }
    //EOL
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
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
int FunctArgs(token_t CurrentToken){
    int RecurCallResult = -1;
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken.type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;
        //ID
        case TOK_identifier:
            //TODO Ulozit do tabulky,

            //AS
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken.type){
                case KW_string:
                case KW_double:
                case KW_integer:
                    //TODO pridelit do struktury funkcie

                RecurCallResult = MoreFunctArgs(CurrentToken);
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
int MoreFunctArgs(token_t CurrentToken){
    int RecurCallResult = -1;
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken.type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;

        //COMMA ID AS ...
        case TOK_comma:
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //ID
            if(CurrentToken.type != TOK_identifier){
                return SYN_ERROR;
            }
            //TODO ulozit do tabulky

            //AS
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken.type){
                case KW_string:
                case KW_double:
                case KW_integer:
                    //TODO pridelit do struktury tabulky
                break;

                default:
                    return SYN_ERROR;
            }

            //<more-function-args>
            RecurCallResult = MoreFunctArgs(CurrentToken);
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
  * <function-definition> -> FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL <function-body> FUNCTION EOL
  * FUNCTION was already check in <prog> so we start with ID
  * @return Type of error or success
  */
int FunctionDefinition(token_t CurrentToken){
    int RecurCallResult = -1;

    //ID
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != TOK_identifier){
        return SYN_ERROR;
    }
    //TODO vlozit tam kde treba..

    //LEFT_BRACKET
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != TOK_lParenth){
        return SYN_ERROR;
    }

    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //AS
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != KW_as){
        return SYN_ERROR;
    }

    //<data-type>
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken.type){
        case KW_string:
        case KW_double:
        case KW_integer:
            //TODO pridelit do struktury tabulky
        break;

        default:
            return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != TOK_endOfLine){
        return SYN_ERROR;
    }

    //<function-body>
    RecurCallResult = Stats(CurrentToken, false);
    if(RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //FUNCTION
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != KW_function){
        return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken.type != TOK_endOfLine){
        return SYN_ERROR;
    }
    return SUCCESS;
}


/**@brief: Function or Scope BODY
 * @return Type of error or SUCCESS
 * @param InScope --> signlize if stats was called from user function or from scope
 **/
int Stats(token_t CurrentToken, bool InScope){
    int RecurCallResult = -1;

    if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    //Delete EOLs
    while(CurrentToken.type == TOK_endOfLine){
        if((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
            return ScannerInt;
        }
    }

    switch (CurrentToken.type){
        //END --- functions and scope end with END
        case KW_end:
            return SUCCESS;

        //INPUT ID EOL <stats>
        case KW_input:
            //ID
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_identifier){
                return SYN_ERROR;
            }

            //EOL
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_endOfLine){
                return SYN_ERROR;
            }
            //TODO Kontrola ci ID existuje a ine veci..

            //<stats>
            return Stats(CurrentToken, InScope);

        //DIM ID AS <data-type> (EQUAL <expresion>) EOL <stats>
        case KW_dim:
            //ID
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_identifier){
                return SYN_ERROR;
            }
            //TODO Ulozit do tabulky
            //Treba kontrolovat aj s globalnou tabulkou ci sa nejaka funkcia nevola ako premenna?

            //AS
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken.type){
                case KW_string:
                case KW_double:
                case KW_integer:
                    //TODO pridelit do struktury tabulky
                break;

                default:
                    return SYN_ERROR;
            }

            //EOL or EQUAL <stats>
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch (CurrentToken.type) {
                case TOK_endOfLine:
                    //<stats>
                    return Stats(CurrentToken, InScope);
                //EQUAL
                case TOK_equal:
                    //TODO predat riadenie precedencnej analyze
                    //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
                    //takze to treba ceknut

                    return Stats(CurrentToken, InScope);

                default:
                    return SYN_ERROR;
            }

        //ID EQUAL <expresion>
        //          ID (function) <function-params-call> TODO
        case TOK_identifier:
            //EQUAL
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_equal){
                return SYN_ERROR;
            }


            //<expresion>
            //TODO Kedy predat riadenie precedencnej analyze?
            //Treba rozhodnut ci sa jedna o vyraz alebo volanie funkcie
            //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
            //takze to treba ceknut

            return Stats(CurrentToken, InScope);

        //RETURN <expresion> EOL <stats>
        //Return can by use only inside functions, not inside Scope
        case KW_return:
            //Test if we are inside scope or inside user function, if inside Scope --> syn. error
            if (InScope){
                return SYN_ERROR;
            }

            //<expresion>
            //TODO predat riadenie precedencnej analyze
            //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
            //takze to treba ceknut

            return Stats(CurrentToken, InScope);

        //PRINT <expresion> SEMICOLON <more-print> <stats>
        case KW_print:
            //<expresion>
            //TODO predat riadenie precedencnej analyze
            //Tu by mohol aj skontrolovat ci po ; ide EOL alebo dalsi vyraz,
            //Da sa to tak?

            return Stats(CurrentToken, InScope);

        //TODO If a While --> zatial neviem ako sa s tym bude pracovat co sa tyka instrukcnej pasky..
        //IF <condition> THEN EOL <stat> ELSE EOL <stat> END IF EOL
        /*case KW_if:
            //<condition>
            //TODO predat riadenie precedencnej
            //Vrati mi zrejme THEN - treba to skontrolovat

            //EOL
            if ((ScannerInt = getToken(&CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken.type != TOK_endOfLine){
                return SYN_ERROR;
            }

            //Stats need to be called as they are inside of if so at the begining of line can be else
            return Stats();
            */
        case KW_if:
            //TODO

        case KW_while:
            //TODO

        default:
            return SYN_ERROR;
    }
    return SUCCESS;
}


int main(){
    int ret = parse();
    printf("return %d\n", ret);
}
