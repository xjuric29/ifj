/** @file parser.c
  * @autor Matej Stano (xstano04)
**/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"
#include "str.h"
#include "symtab.h"




//strInit(CurrentToken.value.stringVal);
bool DecOrDefAndEOF = false; //To check if program have scope and then eof
int ScannerInt; //For int returned by function getToken() to control LEX_ERROR or INTERNAL_ERROR;
string FunctionID; //To know in which function we are

/**@Brief Function to malloc space for token
  *@param Token
  *@return Pointer to alocated token
  */
token_t *TokenInit(){
    token_t *Token;
    if ((Token = malloc(sizeof(token_t))) == NULL){
        return NULL;
    }
    if ((Token->value.stringVal = malloc(sizeof(string))) == NULL){
        free(Token);
        return NULL;
    }
    if (strInit(Token->value.stringVal)){
        free(Token->value.stringVal);
        free(Token);
        return NULL;
    }
    return Token;
}

/**@brief Function to free token
  */
void TokenFree(token_t *Token){
    if (Token != NULL){
        if (Token->value.stringVal != NULL){
            strFree(Token->value.stringVal);
            free(Token->value.stringVal);
        }
        free(Token);
    }
}

/**DOPLNIT, zatial len na testovanie
    */
int parse(){
    //Variable for result of syn., and sem. analyze
    int Result;
    //In variable token will be stored token from Scanner
    token_t *CurrentToken = TokenInit(); //Inicialize token
    if (CurrentToken == NULL){
        return INTERNAL_ERROR;
    }
    strInit(&FunctionID);
    //Global table of functions
    st_globalTable_t *GlobalTable = st_global_init(50);
    //Structure to check if we are inside Scope or While or If
    struct check ToCheck;
    ToCheck.InScope = false; ToCheck.InWhile = false; ToCheck.InIf = false;

    //Start recursive descent
    Result = program(CurrentToken, ToCheck, GlobalTable);

    strFree(&FunctionID);
    TokenFree(CurrentToken); //Free Token
    st_delete(GlobalTable); //Free Global table

    return Result;
}

//TODO: Skontrolovanie prazdneho suboru

/**
    * @brief RULES:
    * 1) <prog>	-> SCOPE EOL <scope-body>
	* 2) <prog>	-> <function-declaration> <prog>
	* 3) <prog>	-> <function-definition> <prog>
    * 4) <prog> -> EOF
    * @param CurrentToken is pointer to the structure where is current loaded token
    * @return type of error or succes
**/
int program(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //In global variable with type token_t will be stored token from scanner
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    //Skip all empty lines at beggining of code/between declarations
    while(CurrentToken->type == TOK_endOfLine){
        if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
            return ScannerInt;
        }
    }
    //Switch rule
    switch(CurrentToken->type){
        //<prog> -> EOF
        case TOK_endOfFile: //Here just for the empty file
            if (DecOrDefAndEOF){ //If there was declaration or definition and no scope its error
                return SYN_ERROR;
            }
            return SUCCESS;

        //<prog>	-> <function-declaration> <prog>
        case KW_declare:
            RecurCallResult = FunctionDeclar(CurrentToken, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            DecOrDefAndEOF = true; //Set to true, variable is checked in switch with EOF
            //<prog>
            RecurCallResult = program(CurrentToken, ToCheck, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        //<prog>	-> <function-definition> <prog>
        case KW_function:
            RecurCallResult = FunctionDefinition(CurrentToken, ToCheck, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            DecOrDefAndEOF = true; //Set to true, variable is checked in switch with EOF
            //<prog>
            RecurCallResult = program(CurrentToken, ToCheck, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            return SUCCESS;

        // <prog>	-> SCOPE EOL <scope-body> SCOPE EOF
        case KW_scope:
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR; //Je to syntakticky error?
            }
            // <scope-body>
            ToCheck.InScope = true; //Set that we are entering scope.. return in scope is error
            RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }

            //SCOPE
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != KW_scope){
                return SYN_ERROR;
            }

            //EOF
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //Skip EOLs..
            while(CurrentToken->type == TOK_endOfLine){
                if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                    return ScannerInt;
                }
            }
            if (CurrentToken->type != TOK_endOfFile){
                return SYN_ERROR;
            }

            return SUCCESS;
        //None of SCOPE, FUNCTION, DEFINE, EOF
        default:
            return SYN_ERROR;
    }
}

/**@brief RULE:
 * <function-declaration>   -> DECLARE FUNCTION ID LEFT_BRACKET <function-args> AS <function-type> EOL
 * DECLARE was already checked so we start with FUNCTION
 * @param CurrentToken is pointer to the structure where is current loaded token
 * @param GlobalTable is pointer to Symbol Table...
 * @return type of error or succes
 **/
int FunctionDeclar(token_t *CurrentToken, st_globalTable_t *GlobalTable){
    st_localTable_t *Function; //Pointer to function in Hash Table
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //FUNCTION
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_function){
        return SYN_ERROR;
    }

    //ID
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_identifier){
        return SYN_ERROR;
    }

    //Store ID to GlobalVariable FunctionID
    if (strCopyString(&FunctionID, (CurrentToken->value.stringVal)) == STR_ERROR){
        return INTERNAL_ERROR;
    }

    //Put ID to Global hash table
    Function = st_add_func(GlobalTable, &FunctionID);
    if (Function == NULL){ //If returns null -> error
        return INTERNAL_ERROR;
    }

    //LEFT_BRACKET
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_lParenth){
        return SYN_ERROR;
    }


    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //AS
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_as){
        return SYN_ERROR;
    }

    //<function-type>
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        case KW_string:
        case KW_double:
        case KW_integer:
            //TODO ulozit niekde -> prerobit tento switch na vlastnu funkciu?

        break;

        default:
            return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }

    return SUCCESS;
}


/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return type of error or succes
  **/
int FunctArgs(token_t *CurrentToken){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    //Get token and swich whitch of the rules will be used
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;
        //ID
        case TOK_identifier:
            //TODO Ulozit do tabulky,

            //AS
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken->type){
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
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return error type or success
  */
int MoreFunctArgs(token_t *CurrentToken){
    int RecurCallResult = -1;
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            return SUCCESS;

        //COMMA ID AS ...
        case TOK_comma:
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //ID
            if(CurrentToken->type != TOK_identifier){
                return SYN_ERROR;
            }
            //TODO ulozit do tabulky

            //AS
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken->type){
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
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @param ToCheck is struct with variables to check if we are inside of While, If or Scope
  * @return Type of error or success
  */
int FunctionDefinition(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;

    //ID
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_identifier){
        return SYN_ERROR;
    }
    //TODO vlozit tam kde treba..

    //LEFT_BRACKET
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_lParenth){
        return SYN_ERROR;
    }

    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //AS
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_as){
        return SYN_ERROR;
    }

    //<data-type>
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        case KW_string:
        case KW_double:
        case KW_integer:
            //TODO pridelit do struktury tabulky
        break;

        default:
            return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }

    //<function-body>
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if(RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //FUNCTION
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_function){
        return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }
    return SUCCESS;
}


/**@brief: Function or Scope BODY
 * @param CurrentToken is pointer to the structure where is current loaded token
 * @param ToCheck -> structure whit values to check if we are in scope if or while
 * @return Type of error or SUCCESS
 **/
int Stats(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;
    struct check SolveProblems; //auxiliary struct to solve problem for example with just ELSE in While:  While expresion EOL ....-> ELSE <-.... LOOP

    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    //Delete EOLs
    while(CurrentToken->type == TOK_endOfLine){
        if((ScannerInt = getToken(CurrentToken)) != SUCCESS){
            return ScannerInt;
        }
    }

    switch (CurrentToken->type){
        //END --- functions and scope end with END
        case KW_end:
            if (ToCheck.InWhile){ //If we are inside While and comes end its error..
                printf("Padlo na end in while\n");
                return SYN_ERROR;
            }
            if (ToCheck.InIf){ //If we are inside IF we expect Else to end recursi not End
                printf("Padlo na end in if\n");
                return SYN_ERROR;
            }
            return SUCCESS;

        //INPUT ID EOL <stats>
        case KW_input:
            //ID
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_identifier){
                return SYN_ERROR;
            }

            //EOL
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR;
            }
            //TODO Kontrola ci ID existuje a ine veci..

            //<stats>
            return Stats(CurrentToken, ToCheck, GlobalTable);

        //DIM ID AS <data-type> (EQUAL <expresion>) EOL <stats>
        case KW_dim:
            //ID
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_identifier){
                return SYN_ERROR;
            }
            //TODO Ulozit do tabulky
            //Treba kontrolovat aj s globalnou tabulkou ci sa nejaka funkcia nevola ako premenna?

            //AS
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != KW_as){
                return SYN_ERROR;
            }

            //<data-type>
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch(CurrentToken->type){
                case KW_string:
                case KW_double:
                case KW_integer:
                    //TODO pridelit do struktury tabulky
                break;

                default:
                    return SYN_ERROR;
            }

            //EOL or EQUAL <stats>
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            switch (CurrentToken->type) {
                case TOK_endOfLine:
                    //<stats>
                    return Stats(CurrentToken, ToCheck, GlobalTable);
                //EQUAL
                case TOK_equal:
                    //TODO predat riadenie precedencnej analyze
                    //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
                    //takze to treba ceknut

                    return Stats(CurrentToken, ToCheck, GlobalTable);

                default:
                    return SYN_ERROR;
            }

        //ID EQUAL <expresion>
        //          ID (function) <function-params-call> TODO
        case TOK_identifier:
            //EQUAL
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_equal){
                return SYN_ERROR;
            }


            //<expresion>
            //TODO Kedy predat riadenie precedencnej analyze?
            //Treba rozhodnut ci sa jedna o vyraz alebo volanie funkcie
            //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
            //takze to treba ceknut

            return Stats(CurrentToken, ToCheck, GlobalTable);

        //RETURN <expresion> EOL <stats>
        //Return can by use only inside functions, not inside Scope
        case KW_return:
            //Test if we are inside scope or inside user function, if inside Scope --> syn. error
            if (ToCheck.InScope){
                return SYN_ERROR;
            }

            //<expresion>
            //TODO predat riadenie precedencnej analyze
            //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
            //takze to treba ceknut

            return Stats(CurrentToken, ToCheck, GlobalTable);

        //PRINT <expresion> SEMICOLON <more-print> <stats>
        case KW_print:
            //<expresion>
            //TODO predat riadenie precedencnej analyze
            //Tu by mohol aj skontrolovat ci po ; ide EOL alebo dalsi vyraz,
            //Da sa to tak?

            return Stats(CurrentToken, ToCheck, GlobalTable);

        //TODO If a While --> zatial neviem ako sa s tym bude pracovat co sa tyka instrukcnej pasky..


        //IF <condition> THEN EOL <stat> ELSE EOL <stat> END IF EOL <stats>
        case KW_if:
            //TODO
            SolveProblems = ToCheck;
            SolveProblems.InIf = true; //Set InIf to true so token ELSE is SUCCESS
            SolveProblems.InWhile = false; //Set InWhile to false so LOOP is Error
            //Call function that will check whole structure IF <condition> THEN EOL <stat> ELSE EOL <stat> END IF EOL
            RecurCallResult = IfStat(CurrentToken, SolveProblems, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }

            //last <stats>
            return Stats(CurrentToken, ToCheck, GlobalTable);

        case KW_else:
            //If we are not inside if
            if (!ToCheck.InIf){
                printf("Padlo na elsee in while\n");
                return SYN_ERROR;
            }
            return SUCCESS;
            //TODO Vygenerovanie instrukcii na skok?

        //DO WHILE <expresion> EOL <stats> LOOP EOL
        case KW_do:
            SolveProblems = ToCheck;
            SolveProblems.InIf = false; //Set InIf to false so token ELSE that come before LOOP won`t be evaluated as SUCCESS
            SolveProblems.InWhile = true; //Set InWhile to true so LOOP is success
            //Call function to result while..
            RecurCallResult = WhileStat(CurrentToken, SolveProblems, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }
            //TODO nejake navestia...

            //Check EOL
            if (ScannerInt = getToken(CurrentToken) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR;
            }

            return Stats(CurrentToken, ToCheck, GlobalTable); //Continue to check other stats, recurively

        //We must test if we are inside WHILE otherwise its error
        case KW_loop:
            if(ToCheck.InWhile){ //If we are in While its OK
                return SUCCESS;
            }else{ //otherwise syn. error
                printf("Padlo na loop\n");
                return SYN_ERROR;
            }

        default:
            return SYN_ERROR;
    }
    return SUCCESS;
}

/**
  * @brief: Function for dealing with while in program
  * RULE: DO WHILE <expresion> EOL <stats> LOOP EOL <stats>
  * @param ToCheck is struct with variables to check if we are inside of While, If or Scope
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return Type of error or SUCCESS
 **/
int WhileStat(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;
    //WHILE, do was already checked
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_while){
        return SYN_ERROR;
    }

    //EXPRESION
    //TODO
    //Malo by vratit EOL takze tieto riadky pravdepodobne vymazat
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }
    //<stat>
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }
    //TODO treba navestia testovat uz tu alebo az ked odtialto vystupim?
    //Ak nie tu staci return Stats...
    return RecurCallResult;
}

/**@brief: Function for dealing with IF
  * RULE: IF <expresion> then EOL <stats> ELSE <stats> END IF EOL <stats>
  * @param ToCheck is structure with variables to check if we are inside of While, If or Scope
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return Type of error or SUCCESS
  */
int IfStat(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;
    //<condition>
    //TODO predat riadenie precedencnej
    //Vrati mi zrejme THEN - treba to skontrolovat

    //THEN bude inak..
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_then){
        return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }
    //RecurCallResult will be SUCCESS only if we found ELSE...
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //else <stat>
    ToCheck.InIf = false; //We are entering Else <stat> so we are not inside if.. another ELSE is error
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    //END IF
    //End was already checked inside Stats, checking just IF
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_if){
        return SYN_ERROR;
    }

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }
    return SUCCESS;
}

int main(){
    int ret = parse();
    printf("return %d\n", ret);
}
