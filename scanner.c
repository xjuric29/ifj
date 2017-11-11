#include <stdbool.h>
#include <stdio.h>
#include "scanner.h"

void destroyWhiteSpaces ()
{
    //Nedelat tohle, udelat get char ktery ignoruje white spaces - mozna by mohl i ignorovat komentare a zrusit while
}

int setTokenType (token_t *loadedToken, tokenType_t type)
{
    loadedToken->type = type;
    loadedToken->value.stringVal = NULL;
    return 0;
}

int getToken (token_t *loadedToken)
{
    char currChar;

    while (true)
    {
        currChar = getchar();
        switch (currChar) {
            case '(':
                return setTokenType(loadedToken, TOK_lParenth);
            case ')':
                return setTokenType(loadedToken, TOK_rParenth);
            case '{':
                return setTokenType(loadedToken, TOK_startBlock);
            case '}':
                return setTokenType(loadedToken, TOK_endBlock);
            case '+':
                return setTokenType(loadedToken, TOK_plus);
            case '-':
                return setTokenType(loadedToken, TOK_minus);
            case '*':
                return setTokenType(loadedToken, TOK_mul);
            case '=':
                return setTokenType(loadedToken, TOK_equal);
            case ',':
                return setTokenType(loadedToken, TOK_comma);
            case ';':
                return setTokenType(loadedToken, TOK_semicolon);
        }
    }
}
