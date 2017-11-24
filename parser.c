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
#include "expr.h"
#include "ilist.h"




//strInit(CurrentToken.value.stringVal);
bool DecOrDefAndEOF = false; //To check if program have scope and then eof
int ScannerInt; //For int returned by function getToken() to control LEX_ERROR or INTERNAL_ERROR;
string FunctionID; //To know in which function we are
int ParamNumber; //To store number of parameter we are checking

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

    if (addBuiltTable(GlobalTable) != SUCCESS){
        return INTERNAL_ERROR;
    }

    //Structure to check if we are inside Scope or While or If
    struct check ToCheck;
    ToCheck.InScope = false; ToCheck.InWhile = false; ToCheck.InIf = false; ToCheck.InElse = false;
    //Start recursive descent
    Result = program(CurrentToken, ToCheck, GlobalTable);

    //Testy *********/
    /*st_localTable_t *FF = st_find_func(GlobalTable, &FunctionID);
    if (FF->params != NULL){
        st_element_t *Param = FF->params->first;
        int i = 0;
        printf("Pocet parametrov je: %d\n", FF->params->params_n);
        while(Param != NULL){
            i++;
            printf("Parameter %d: %s --> %d\n", i, Param->key.str, Param->el_type);
            Param = Param->next_param;
        }
    }
    printf ("Funkcii v tabulke je = %d\n", GlobalTable->global_n);
    //printf ("Prvkov vo funkcii je = %d\n", GlobalTable->functions[40]->local_n);
    /**********/
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

            //Scope in HashTable represented as #Scope
            char *name = "Scope";
            strClear(&FunctionID);
            for(int i = 0; name[i] != '\0'; i++){
                if(strAddChar(&FunctionID, name[i])){
                    return INTERNAL_ERROR;
                }
            }
            //Put scope in global hash table
            if (st_add_func(GlobalTable, &FunctionID) == NULL){
                return INTERNAL_ERROR;
            }

            //Copy Scope to token so 'add_instruction can read it'
            //strCopyString(CurrentToken->value.stringVal, &FunctionID);
            //Create label Scope
            add_instruction(SCOPE, NULL, NULL, NULL);

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

    //Check if Function wasn`t already in Global Tabel which means it was already declared or defined
    if (Function->declared || Function->defined){
        fprintf(stderr,"Dvojita deklaracia\n");
        return SEM_ERROR_FUNC;
    }

    //LEFT_BRACKET
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_lParenth){
        return SYN_ERROR;
    }

    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken, GlobalTable);
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
            Function->func_type = CurrentToken->type; //Save function type
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

    Function->declared = true; //Function was declared

    return SUCCESS;
}


/**@brief RULES:
  * 1) <function-args> -> RIGHT_BRACKET
  * 2) <function-args> -> ID AS <data-type> <more-function-args>
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @return type of error or succes
  **/
int FunctArgs(token_t *CurrentToken, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1; //Variable for checking of recursive descent
    st_element_t *Parameter; //Variable to store pointer on parameter we are working with in Symtab
    st_localTable_t *Function = st_find_func(GlobalTable, &FunctionID); //Pointer to function we are proccessing
    ParamNumber = 1; //Set that we start with parameter number 1

    //Get token and swich which of the rules will be used
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            if (Function->declared){ //If function was declared
                if (Function->params != NULL){ //Definition has 0 params, but declaration has >0
                    fprintf(stderr,"Definicia nema ziadne argumenty ale deklaracia ma\n");
                    return SEM_ERROR_FUNC;
                }
            }
            return SUCCESS;

        //ID
        case TOK_identifier:
            //If FunctArgs is called from Definition we need to check if there was declaration and check arguments

            if (Function->declared){ //We are executing definition of function that was declared.. we need to check arguments
                if (Function->params == NULL){ //Declaration hasn`t any arguments
                    fprintf(stderr,"Deklaracia nema ziadne argumenty ale definicia ma\n");
                    return SEM_ERROR_FUNC;
                }
                //Check if ID of first argument is equal to first argument in declaration if not, we need to save new ID
                if (strCmpString(CurrentToken->value.stringVal, &Function->params->first->key)){
                    //printf("%s\n", Function->params->first->key.str);
                    if (strCopyString(&Function->params->first->key, CurrentToken->value.stringVal)){
                        return INTERNAL_ERROR;
                    }
                }
                //printf("%s\n", Function->params->first->key.str);
                //printf("-----------------\n");

            }else{ //We are executing declaration, or definition of function that wasn`t declared..

                //Check If parameter ID isn`t also ID of any created Function
                if(st_find_func(GlobalTable, CurrentToken->value.stringVal) != NULL){
                    fprintf(stderr,"Parameter je ID sa rovna func ID\n");
                    return SEM_ERROR_FUNC;
                }

                //Save element to Local Table of function.. Save it as parameter
                Parameter = st_add_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal, 'P');
                //TODO v Symtab.c kontroly..
                if (Parameter == NULL){
                    return INTERNAL_ERROR;
                }
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
                    if (Function->declared){ //If was declared we need to check data type
                        if (CurrentToken->type != Function->params->first->el_type){
                            fprintf(stderr,"Nesedi typ prveho argumentu\n");
                            return SEM_ERROR_FUNC;
                        }
                        ParamNumber++;
                    }else{
                        Parameter->el_type = CurrentToken->type; //Set type of parameter
                    }
                break;
                //Token isn`t data-type
                default:
                    return SYN_ERROR;
            }
            //<more-function-args>
            RecurCallResult = MoreFunctArgs(CurrentToken, GlobalTable);
            if(RecurCallResult != SUCCESS){
                return RecurCallResult;
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
int MoreFunctArgs(token_t *CurrentToken, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;
    st_element_t *Parameter; //Variable to store pointer on parameter we are working with in Symtab
    st_localTable_t *Function = st_find_func(GlobalTable, &FunctionID); //Pointer to function we are proccessing

    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    switch(CurrentToken->type){
        //RIGHT_BRACKET
        case TOK_rParenth:
            if (Function->declared){ //If function was declared we need to check arguments
                if (Function->params->params_n >= ParamNumber){ //Token is ), we need to check if we don`t have less arguments then in declaration                    return SEM_ERROR_FUNC;
                    fprintf(stderr,"Definicia ma menej argumentov ako deklaracia\n");
                    return SEM_ERROR_FUNC;
                }
            }
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
            if (Function->declared){
                //Check If parameter ID isn`t also ID of any created Function
                if (st_find_func(GlobalTable, CurrentToken->value.stringVal) != NULL){
                    return SEM_ERROR_FUNC;
                }

                //Check if number of params in declaration is >= as order of parameter we are checking
                if (Function->params->params_n < ParamNumber){
                    return SEM_ERROR_FUNC;
                }

                //Get to needed parameter + check colisisions with ID of previos parameters
                Parameter = Function->params->first;
                while(Parameter->param_number != ParamNumber){
                    //If some of the previos parameters has same name as new paramater
                    if (strCmpString(&Parameter->key, CurrentToken->value.stringVal) == STR_SUCCESS){
                        return SEM_ERROR_FUNC;
                    }
                    Parameter = Parameter->next_param;
                }

                //Check if ID of argument is equal to argument in declaration, if not we need to save new ID
                if (strCmpString(&Parameter->key, CurrentToken->value.stringVal)){
                    //printf("%s\n", Parameter->key.str);
                    if (strCopyString(&Parameter->key, CurrentToken->value.stringVal)){
                        return INTERNAL_ERROR;
                    }
                }
                //printf("%s\n", Parameter->key.str);
                //printf("-----------------\n");

                //TODO Vymysliet kontrolu ak sme v definicii a bola deklarovana..
                //Save element to Local Table of function.. Save it as parameter
            }else{

                //Check If parameter ID isn`t also ID of any created Function
                if (st_find_func(GlobalTable, CurrentToken->value.stringVal) != NULL){
                    return SEM_ERROR_FUNC;
                }

                //Check If parameter with this ID wasn`t already used
                if (st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal) != NULL){
                    return SEM_ERROR_FUNC;
                }

                //Put parameter into local hash table
                Parameter = st_add_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal, 'P');
                //TODO v Symtab.c kontroly..
                if (Parameter == NULL){
                    return INTERNAL_ERROR;
                }
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
                    if (Function->declared){
                        if (Parameter->el_type != CurrentToken->type){ //If data-type does`t correspondent
                            fprintf(stderr,"Nesedi typ parametru, <more-function-args>\n");
                            return SEM_ERROR_FUNC;
                        }
                        ParamNumber++;
                    }else{
                        Parameter->el_type = CurrentToken->type; //Set type of parameter
                    }
                break;

                default:
                    return SYN_ERROR;
            }

            //<more-function-args>
            RecurCallResult = MoreFunctArgs(CurrentToken, GlobalTable);
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
    st_localTable_t *Function;

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

    //Check redefinition..
    if (Function->defined){
        fprintf(stderr,"Redefinicia\n");
        return SEM_ERROR_FUNC;
    }

    //Create label with Function name
    add_instruction(FUNC, CurrentToken, NULL, NULL);

    //LEFT_BRACKET
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_lParenth){
        return SYN_ERROR;
    }

    //<function-args>
    RecurCallResult = FunctArgs(CurrentToken, GlobalTable);
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
            if (Function->declared){
                if (Function->func_type != CurrentToken->type){
                    fprintf(stderr,"Nesedi typ funkcie s deklaraciou\n");
                    return SEM_ERROR_FUNC;
                }
            }else{
                Function->func_type = CurrentToken->type;
            }

        break;

        default:
            return SYN_ERROR;
    }

    //TODO move do returnvall prazdny string alebo 0 alebo 0.0

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
    Function->defined = true;
    return SUCCESS;
}


/**@brief: Function or Scope BODY
 * @param CurrentToken is pointer to the structure where is current loaded token
 * @param ToCheck -> structure with values to check if we are in scope if or while
 * @return Type of error or SUCCESS
 **/
int Stats(token_t *CurrentToken, struct check ToCheck, st_globalTable_t *GlobalTable){
    int RecurCallResult = -1;
    st_element_t *Variable; //To save pointer on variable in hashTable
    struct check SolveProblems; //struct to solve problem with conflicts in while and if blocks
    //st_localTable_t *CalledFunction; //Pointer to hash table, for work with function Call

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
                fprintf(stderr,"End vo While cyke\n");
                return SYN_ERROR;
            }
            if (ToCheck.InIf){ //If we are inside IF we expect Else to end recursi not End
                fprintf(stderr,"End v casti If bloku\n");
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
            //Check if ID exist in functions
            if (st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal) == NULL){
                fprintf(stderr,"Nedefinovany ID\n");
                return SEM_ERROR_FUNC;
            }

            //Generate instruction for read
            add_instruction(READ, CurrentToken, NULL, NULL);

            //EOL
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR;
            }

            //<stats>
            return Stats(CurrentToken, ToCheck, GlobalTable);

        //DIM ID AS <data-type> (EQUAL <expresion>) EOL <stats>
        case KW_dim:
            if (ToCheck.InWhile || ToCheck.InIf || ToCheck.InElse){
                return SYN_ERROR;
            }
            //ID
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_identifier){
                return SYN_ERROR;
            }

            //Check if doesn`t exist function with same ID as variable
            if (st_find_func(GlobalTable, CurrentToken->value.stringVal) != NULL){
                return SEM_ERROR_FUNC;
            }
            //Check if variable with same id wasn`t already declared
            if (st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal) != NULL){
                fprintf(stderr,"Pokus o redefiniciu premmennej: %s\n", CurrentToken->value.stringVal->str);
                return SEM_ERROR_FUNC;
            }
            if ((Variable = st_add_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal, 'V')) == NULL){
                return INTERNAL_ERROR;
            }

            //Create variable
            add_instruction(DEFVAR_LF, CurrentToken, NULL, NULL);

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
                    Variable->el_type = CurrentToken->type;
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
                    //TODO premennu inicializovat na nulu alebo prazdny string

                    CurrentToken->type = Variable->el_type;
                    add_instruction(MOVE, CurrentToken, &Variable->key, NULL);

                    return Stats(CurrentToken, ToCheck, GlobalTable);
                //EQUAL
                case TOK_equal:
                    //TODO predat riadenie precedencnej analyze
                    //Zrejme bude vracat aj posledny nacitany token ktorym by mal byt EOL
                    //takze to treba ceknut

                    //Get token and resolve if pass to expr or deal with function call
                    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                        return ScannerInt;
                    }

                    //Call function which choose between function call and expresion
                    if ((RecurCallResult = ResAssignInParser(CurrentToken, GlobalTable, Variable)) != SUCCESS){
                        return RecurCallResult;
                    }

                    return Stats(CurrentToken, ToCheck, GlobalTable);

                default:
                    return SYN_ERROR;
            }

        //ID EQUAL <expresion>
        //          ID (function) <function-params-call> <stats>
        case TOK_identifier:
            //Check if variable exist in function
            if ((Variable = st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal)) == NULL){
                fprintf(stderr,"Pokus o priradenie do neexistujucej premennej: %s\n", CurrentToken->value.stringVal->str);
                return SEM_ERROR_FUNC;
            }

            //EQUAL
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            if (CurrentToken->type != TOK_equal){
                return SYN_ERROR;
            }

            //ID - we need to check if it`s function CALL or expresion
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }

            //Call function which choose between function call and expresion
            if ((RecurCallResult = ResAssignInParser(CurrentToken, GlobalTable, Variable)) != SUCCESS){
                return RecurCallResult;
            }

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

            //TODO Kde vyriesit kontrolu ci vraciame spravny typ akeho je typu funkcia

            //expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *Variable);
            if ((RecurCallResult = expr_main(EXPRESION_CONTEXT_RETURN, CurrentToken, GlobalTable, &FunctionID, NULL)) != SUCCESS){
                return RecurCallResult;
            }

            //Return
            add_instruction(RETURN, NULL, NULL, NULL);

            //Check token from expresion
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR;
            }

            //<stats>
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

            SolveProblems = ToCheck;
            SolveProblems.InIf = true; //Set InIf to true so token ELSE is SUCCESS
            SolveProblems.InWhile = false; //Set InWhile to false so LOOP is Error

            //Call function that will check whole structure IF <condition> THEN EOL <stat> ELSE EOL <stat> END IF EOL
            RecurCallResult = IfStat(CurrentToken, SolveProblems, GlobalTable);
            if (RecurCallResult != SUCCESS){
                return RecurCallResult;
            }

            //IF
            add_instruction(IF, NULL, NULL, NULL);

            //last <stats>
            return Stats(CurrentToken, ToCheck, GlobalTable);

        case KW_else:
            //If we are not inside of if block
            if (!ToCheck.InIf){
                fprintf(stderr,"Else mimo bloku if\n");
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
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
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
                fprintf(stderr,"Loop mimo bloku while\n");
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

    add_instruction(WHILE, NULL, NULL, NULL);
    //EXPRESION
    //TODO
    //expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *Variable);
    if ((RecurCallResult = expr_main(EXPRESION_CONTEXT_LOGIC, CurrentToken, GlobalTable, &FunctionID, NULL)) != SUCCESS){
        return RecurCallResult;
    }

    //Malo by vratit EOL takze tieto riadky pravdepodobne vymazat
    /*if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }*/

    //Check token from expresion
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }
    //<stat>
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }

    add_instruction(LOOP, NULL, NULL, NULL);
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

    //expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *Variable);
    if ((RecurCallResult = expr_main(EXPRESION_CONTEXT_LOGIC, CurrentToken, GlobalTable, &FunctionID, NULL)) != SUCCESS){
        return RecurCallResult;
    }

    /*//Check token from expresion
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }*/

    //Check then
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

    add_instruction(ELSE, NULL, NULL, NULL);

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }

    //else <stat>
    ToCheck.InIf = false; //We are entering Else <stat> so we are not inside if.. another ELSE is error
    ToCheck.InElse = true; //Set just for control of DIM ID.. inside ELSE
    RecurCallResult = Stats(CurrentToken, ToCheck, GlobalTable);
    if (RecurCallResult != SUCCESS){
        return RecurCallResult;
    }
    ToCheck.InElse = false; //We are back from else
    //END IF
    //End was already checked inside Stats, checking just IF
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != KW_if){
        return SYN_ERROR;
    }

    //ENDIF
    add_instruction(ENDIF, NULL, NULL, NULL);

    //EOL
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_endOfLine){
        return SYN_ERROR;
    }

    return SUCCESS;
}

/**
  * @brief: Function to check Function CalledFunction
  * Check arguments, types and conversions
  * ID (<params>) EOL
  * @param CurrentToken is pointer to the structure where is current loaded token
  * @param CalledFunction is pointer to function in Hash Table, function that is called
  * @oaram variable is pointer to element in local hashTable, its variable we are assigning in, here to check data type
  **/
int FuncCallCheck(token_t *CurrentToken, st_globalTable_t *GlobalTable, st_localTable_t *CalledFunction, st_element_t *Variable){
    //Expect '('
    if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
        return ScannerInt;
    }
    if (CurrentToken->type != TOK_lParenth){
        return SYN_ERROR;
    }

    //Called function has 0 params
    if (CalledFunction->params == NULL){

        //Token must be )
        if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
            return ScannerInt;
        }
        if (CurrentToken->type != TOK_rParenth){
            fprintf(stderr, "Funkcia bola volana s viac argumentamy ako ich ma\n");
            return SEM_ERROR_COMP;
        }

    }else{
        int pNumber = 1; //To Check if we finished with right amount of parameters
        st_element_t *param = CalledFunction->params->first;
        st_element_t *IDparameter;
        //Check paramaters
        while(param != NULL){
            //Get token
            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //Choose what type is parameter.. Constant or variable
            switch(CurrentToken->type){

                //If we get ')' -> sem. error, function has more arguments
                case TOK_rParenth:
                    fprintf(stderr, "Funkcia bola volana s menej argumentamy ako ma jej definicia\n");
                    return SEM_ERROR_COMP;

                //First argument is ID
                case TOK_identifier:
                    //Check if ID was defined
                    if ((IDparameter = st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal)) == NULL){
                        fprintf(stderr, "Funkcia %s volana s neexistujucim parametrom: %s\n", CalledFunction->key.str, CurrentToken->value.stringVal->str);
                        return SEM_ERROR_FUNC;
                    }

                    //Check if types of parameter and given variable are same
                    if(param->el_type != IDparameter->el_type){

                        //If not, check if neither of parameter or variable is string -> string is only compatibile with string
                        if (IDparameter->el_type == st_string || param->el_type == st_string){
                            return SEM_ERROR_COMP;
                        }

                        //If given variable is decimal -> parameter in funct is integer
                        if (IDparameter->el_type == st_decimal){
                            //TODO volanie prislusnej...
                            fprintf(stderr, "Prevadzam ID z float na int\n");
                        }

                        //If given variable is int -> parameter is float
                        if (IDparameter->el_type == st_integer){
                            //TODO stuff...
                            fprintf(stderr, "Prevadzam ID z int na float\n");
                        }
                    }

                    //TODO Presunut z jedneho framu do druheho

                    break;

                //Parameter is represented as constant of type integer
                case TOK_integer:

                    //If parameter in definition isn`t integer type
                    if (param->el_type != st_integer){

                        //If parameter in definition is type string
                        if (param->el_type == st_string){
                            fprintf(stderr, "Parameter bol typu string ale dostal som konstantu int\n");
                            return SEM_ERROR_COMP;
                        }

                        //If parameter in definition is type of double, do conversion
                        if (param->el_type == st_decimal){
                            fprintf(stderr, "Prevadzam konstantu INT na DOUBLE\n");
                            //TODO stuff
                        }

                    }

                    //TODO Presunut z jedneho farmu do druheho
                    break;

                //Parameter is represented as constant of type float
                case TOK_decimal:

                    //If parameter definition isn`t float type
                    if (param->el_type != st_decimal){

                        //If parameter in definition is type string
                        if (param->el_type == st_string){
                            fprintf(stderr, "Parameter bol typu string ale dostal som float\n");
                            return SEM_ERROR_COMP;
                        }

                        //If parameter in definition is type int, do conversion
                        if (param->el_type == st_integer){
                            fprintf(stderr, "Prevadzam konstantu float na int\n");
                            //TODO stuff
                        }
                    }

                    //TODO Frame
                    break;

                //Parameter is represented as constant of type string
                case TOK_string:

                    //If parameter definition isn`t string type
                    if (param->el_type != st_string){
                        return SEM_ERROR_COMP;
                    }

                    break;

                default:
                    return SYN_ERROR;

            }

            if ((ScannerInt = getToken(CurrentToken)) != SUCCESS){
                return ScannerInt;
            }
            //If we are checking last parameter expect token ')'
            if (pNumber == CalledFunction->params->params_n){
                if (CurrentToken->type != TOK_rParenth){
                    fprintf(stderr, "Cakal som zatvorku, neprisla\n");
                    return SEM_ERROR_COMP;
                }
            //otherwise expect token ','
            }else{
                if (CurrentToken->type != TOK_comma){
                    fprintf(stderr, "Cakal som ciarku, neprisla\n");
                    return SEM_ERROR_COMP;
                }
            }
            param = param->next_param;
            pNumber++;
        }
    }

    //Test of return type of function and type of Variable we are assigning in
    if (Variable->el_type != CalledFunction->func_type){
        //If one is string -> no conversions
        if (Variable->el_type == st_string || CalledFunction->func_type == st_string){
            fprintf(stderr, "Len jedna z dvojice funkcia/premenna do ktorej sa funkcia priraduje, ma typ string\n");
            return SEM_ERROR_COMP;
        }

        //If Variable is int -> function is double
        if (Variable->el_type == st_integer){
            fprintf(stderr, "Navratovy typ funkce dobule prevadzam na int\n");
            //TODO konverzia
        }

        if (Variable->el_type == st_decimal){
            fprintf(stderr, "Navratov typ funkce int prevadzam na float\n");
            //TODO konverzia
        }

    }else{
        //TODO len prirad
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


/**@brief Function to resolve assignment:
  * if we need to pass <expresion> to expr.c or if it`s function call
  * @param CurrentToken is current loaded token
  * @param GlobalTable is global table of functions
  * @oaram variable is pointer to element in local hashTable, its variable we are assigning
  * @return SUCCESS or type of error
  */
int ResAssignInParser(token_t *CurrentToken, st_globalTable_t *GlobalTable, st_element_t *Variable){
    int RecurCallResult = -1;
    st_localTable_t *CalledFunction;
    //string BuiltinFuncName;

    //Check for Builtin Function
    switch (CurrentToken->type){

        //If we get ID we need to check if it`s function call or just expresion
        case TOK_identifier:
            //Check if it`s function call
            if ((CalledFunction = st_find_func(GlobalTable, CurrentToken->value.stringVal)) == NULL){
                //If we don`t find function, check if we find variable in current function
                if (st_find_element(GlobalTable, &FunctionID, CurrentToken->value.stringVal) == NULL){
                    fprintf(stderr, "Prva premenna je hned nedefinovana\n");
                    return SEM_ERROR_FUNC;
                }
                //fprintf(stderr, "Spracovava expresion\n");

                //expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *Variable);
                if ((RecurCallResult = expr_main(EXPRESION_CONTEXT_ARIGH, CurrentToken, GlobalTable, &FunctionID, Variable)) != SUCCESS){
                    return RecurCallResult;
                }

                //Check token from expresion
                if (CurrentToken->type != TOK_endOfLine){
                    return SYN_ERROR;
                }

            }else{
                //Test if function vas declared or defined -> just because of recurcive call of function without declaration
                if ((CalledFunction->declared || CalledFunction->defined) == false){
                    fprintf(stderr, "Rekurzivne volanie funkcie ktora nebola deklarovana\n");
                    return SEM_ERROR_FUNC;
                }

                //Function was found check params, it checks also EOL
                if ((RecurCallResult = FuncCallCheck(CurrentToken, GlobalTable, CalledFunction, Variable)) != SUCCESS){
                    return RecurCallResult;
                }

                //TODO Skontrolovat navratovy typ mozem to urobit vo FuncCallCheck?
            }

            break;

        //Case built in functions
        case KW_length:

            //Save name to Token
            if (strCopyConst(CurrentToken->value.stringVal, "length")){
                return INTERNAL_ERROR;
            }

            //Find function in HashTable, no need to controle, because builtin function is in hashTable
            CalledFunction = st_find_func(GlobalTable, CurrentToken->value.stringVal);

            //Check params and return type, it checks also EOL
            if ((RecurCallResult = FuncCallCheck(CurrentToken, GlobalTable, CalledFunction, Variable)) != SUCCESS){
                return RecurCallResult;
            }
            break;

        case KW_subStr:

            //Save name to Token
            if (strCopyConst(CurrentToken->value.stringVal, "substr")){
                return INTERNAL_ERROR;
            }

            //Find function in HashTable, no need to controle, because builtin function is in hashTable
            CalledFunction = st_find_func(GlobalTable, CurrentToken->value.stringVal);

            //Check params and return type, it checks also EOL
            if ((RecurCallResult = FuncCallCheck(CurrentToken, GlobalTable, CalledFunction, Variable)) != SUCCESS){
                return RecurCallResult;
            }
            break;

        case KW_asc:

            //Save name to Token
            if (strCopyConst(CurrentToken->value.stringVal, "asc")){
                return INTERNAL_ERROR;
            }

            //Find function in HashTable, no need to controle, because builtin function is in hashTable
            CalledFunction = st_find_func(GlobalTable, CurrentToken->value.stringVal);

            //Check params and return type, it checks also EOL
            if ((RecurCallResult = FuncCallCheck(CurrentToken, GlobalTable, CalledFunction, Variable)) != SUCCESS){
                return RecurCallResult;
            }
            break;

        case KW_chr:

            //Save name to Token
            if (strCopyConst(CurrentToken->value.stringVal, "chr")){
                return INTERNAL_ERROR;
            }

            //Find function in HashTable, no need to controle, because builtin function is in hashTable
            CalledFunction = st_find_func(GlobalTable, CurrentToken->value.stringVal);

            //Check params and return type, it checks also EOL
            if ((RecurCallResult = FuncCallCheck(CurrentToken, GlobalTable, CalledFunction, Variable)) != SUCCESS){
                return RecurCallResult;
            }

            break;

        default:
            //TODO Call expresion
            //expr_main(int context, token_t *parserToken, st_globalTable_t *st_global, string *func_name, st_element_t *Variable);
            if ((RecurCallResult = expr_main(EXPRESION_CONTEXT_ARIGH, CurrentToken, GlobalTable, &FunctionID, Variable)) != SUCCESS){
                return RecurCallResult;
            }

            //Check token from expresion
            if (CurrentToken->type != TOK_endOfLine){
                return SYN_ERROR;
            }
            break;
    }
    return SUCCESS;
}


/*
int main(){
    instr_init();
    int ret = parse();
    print_all();
    inst_free();

    printf("return %d\n", ret);
}*/
