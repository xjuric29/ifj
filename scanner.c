#include <stdbool.h>
#include <stdio.h>
#include "scanner.h"

/**
 * Returns char from stdin which isn't space, tab, VT, FF and CR. These white space chars will be skipped.
 * @return char from stdin
 */
char getNotSpaceChar()
{
    char temp = getchar();
    return (temp != ' ' && temp != '\t' && (temp < 11 || temp > 13)) ? temp : getNotSpaceChar();
}

/**
 * Skips all char from stdin until EOL or EOF.
 * @return char which stop the recursion
 */
char skipRowComment()
{
    char temp = getchar();
    return (temp == '\n' || temp == EOF) ? temp : skipRowComment();
}

/**
 * Skips all char until get sequence "'/"
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

    /*
     * Step for calling this function after EOF token was send.
     */
    if (lastChar == EOF) return setTokenType(loadedToken, TOK_endOfLine);
    if (useLastChar)
    {
        currChar = lastChar;
        useLastChar = false;
    } else currChar = getNotSpaceChar();

    while (true)
    {
        switch (currChar)
        {
            /// Comments and division operator
            case '\'':  // Skip comment and start new round of loop
                currChar = skipRowComment();
                break;
            case '/':
                currChar = getNotSpaceChar();
                /*
                 * If new curChar is "'" skip comment and start new round of loop. Else save new char for next calling
                 * this function and return division token.
                 */
                if (currChar == '\'')
                {
                    if (skipRowComment()) return 1;
                    else
                    {
                        currChar = getNotSpaceChar();
                        break;
                    }
                } else
                {
                    lastChar = currChar;
                    useLastChar = true;
                    return setTokenType(loadedToken, TOK_div);
                }

            /// New line
            case '\n':
                /*
                 * If is in code multiple blank EOLs in succession the EOL token will be send just once
                 */
                if (lastChar == '\n')
                {
                    currChar = getNotSpaceChar();
                    break;
                } else
                {
                    lastChar = '\n';
                    return setTokenType(loadedToken, TOK_endOfLine);
                }

            /// Operators
            case '<':
                currChar = getNotSpaceChar();
                lastChar = currChar;
                if (currChar == '=') return setTokenType(loadedToken, TOK_lessEqual);
                else if (currChar == '>') return setTokenType(loadedToken, TOK_notEqual);
                else
                {
                    useLastChar = true;
                    return setTokenType(loadedToken, TOK_less);
                }
            case '>':
                currChar = getNotSpaceChar();
                lastChar = currChar;
                if (currChar == '=') return setTokenType(loadedToken, TOK_greaterEqual);
                else
                {
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
