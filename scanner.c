#include <stdbool.h>
#include <stdio.h>
#include "scanner.h"
#include "str.h"

/**	@file scanner.c
 *	@brief C file for scanner interface
 *	@author Jiri Jurica (xjuric29)
 *	@todo Another solution for case ranges
 */

/**
 * Tests if input char is white char (space, tab, VT, FF and CR)
 * @param input char
 * @return true if input is white char else false
 */
int isWhiteChar(char input)
{
    return (input == ' ' || input == '\t' || (input >= 11 && input <= 13));
}

/**
 * Returns char from stdin which isn't white. These chars will be skipped.
 * @return char from stdin
 */
char getNotWhiteChar()
{
    char temp = getchar();
    return (isWhiteChar(temp)) ? getNotWhiteChar() : temp;
}

/**
 * Skips all char from stdin until EOL or EOF.
 * @return char which stop the recursion
 */
char skipRowComment()
{
    char temp = getchar();
    return (temp != '\n' && temp != EOF) ? skipRowComment() : temp;
}

/**
 * Skips all char until get sequence "'/".
 * @return 0 if comment ends correctly else 1
 */
int skipBlockComment()
{
    char curChar, lastChar;
    while (true)
    {
        curChar = getchar();
        if (curChar == EOF) return 1;
        if (lastChar == '\'' && curChar == '/') return 0;
        lastChar = curChar;
    }
}

int getIDKeyword (token_t *loadedToken, char curChar, char *lastChar)
{
    /*char *keywords[] = {"as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function", "if", "input",
                         "integer", "length", "loop", "print", "return", "scope", "string", "substr", "then", "while",
                         "and", "boolean", "continue", "elseif", "exit", "false", "for", "next", "not", "or", "shared",
                         "static", "true"};*/
    string id;

    if (strInit(&id)) return 99;
    strAddChar(&id, curChar);

    while (true)    // String assembling
    {
        curChar = getchar();
        if (curChar >= 'A' && curChar <= 'Z')
        {
            curChar += 32;  // From uppercase to lowercase
            if (strAddChar(&id, curChar)) return 99;
        }
        else if ((curChar >= 'a' && curChar <= 'z') || curChar == '_' || (curChar >= '0' && curChar <= '9'))
        {
            if (strAddChar(&id, curChar)) return 99;
        }
        else break;
    }
    *lastChar = curChar;

    for (int i = 0; i )
    return 0;
}

int setTokenType(token_t *loadedToken, tokenType_t type)
{
    loadedToken->type = type;
    loadedToken->value.stringVal = NULL;
    return 0;
}

int getToken(token_t *loadedToken)
{
    char currChar;
    static char lastChar;
    static bool useLastChar = false;

    // Step for calling this function after EOF token was send.
    if (lastChar == EOF) return setTokenType(loadedToken, TOK_endOfLine);
    if (useLastChar)
    {
        currChar = lastChar;
        useLastChar = false;
    } else currChar = getNotWhiteChar();

    while (true)
    {
        switch (currChar)
        {
            /// Comments and division operator
            case '\'':  // Skip comment and start new round of loop.
                currChar = skipRowComment();
                break;
            case '/':
                currChar = getchar();
                /*
                 * If new curChar is "'" skip comment and start new round of loop. Else save new char for next calling
                 * this function and return division token.
                 */
                if (currChar == '\'')
                {
                    if (skipBlockComment()) return 1;
                    else
                    {
                        currChar = getNotWhiteChar();
                        break;
                    }
                } else
                {
                    if (isWhiteChar(currChar)) lastChar = getNotWhiteChar();
                    useLastChar = true;
                    return setTokenType(loadedToken, TOK_div);
                }

            /// New line
            case '\n':
                //If is in code multiple blank EOLs in succession the EOL token will be send just once.
                if (lastChar == '\n')
                {
                    currChar = getNotWhiteChar();
                    break;
                } else
                {
                    lastChar = '\n';
                    return setTokenType(loadedToken, TOK_endOfLine);
                }

            /// ID|keyword and var values
            case '_':
            case 'A' ... 'Z':   /// Range in case is supported only in gcc compiler!
            case 'a' ... 'z':

            /// Operators
            case '<':
                currChar = getchar();
                lastChar = currChar;
                // If the new curChar is white char load next char and send just less token
                if (currChar == '=') return setTokenType(loadedToken, TOK_lessEqual);
                else if (currChar == '>') return setTokenType(loadedToken, TOK_notEqual);
                else
                {
                    if (isWhiteChar(currChar)) lastChar = getNotWhiteChar();
                    useLastChar = true;
                    return setTokenType(loadedToken, TOK_less);
                }
            case '>':
                currChar = getchar();
                lastChar = currChar;
                // Same example as less operator
                if (currChar == '=') return setTokenType(loadedToken, TOK_greaterEqual);
                else
                {
                    if (isWhiteChar(currChar)) lastChar = getNotWhiteChar();
                    useLastChar = true;
                    return setTokenType(loadedToken, TOK_greater);
                }
            case '(':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_lParenth);
            case ')':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_rParenth);
            case '{':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_startBlock);
            case '}':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_endBlock);
            case '+':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_plus);
            case '-':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_minus);
            case '*':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_mul);
            case '=':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_equal);
            case ',':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_comma);
            case ';':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_semicolon);

            /// End of file
            case EOF:
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_endOfFile);

            /// Errors
            default:
                return 1;
        }
    }
}
