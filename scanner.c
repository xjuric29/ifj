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
 * @param input - char
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
 * Simple function for shorter write. Sets just token type.
 * @param loadedToken - token which will be send
 * @param type - type of token to be used
 * @return always 0
 */
int setTokenType(token_t *loadedToken, tokenType_t type)
{
    loadedToken->type = type;
    return 0;
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

/**
 * Converts escaped numbers from stdin stream to int
 * @param curChar - first loaded digit
 * @return 1-255 on success else -1
 */
int escapedNumber (char curChar)
{
    char textNumber[] = {curChar, '0', '0', '\0'};    // For three-digit number prepare the string
    int convertedNumber;

    for (int i = 1; i < 3; i++)
    {
        if ((curChar = getchar()) >= '0' && curChar <= '9') textNumber[i] = curChar;
        else return -1;
    }
    if (sscanf (textNumber, "%d", &convertedNumber) == 1 && convertedNumber >= 1 && convertedNumber <= 255)
    {
        return convertedNumber;
    }
    else return -1;
}

/**
 * Assembles name of the var or keyword and then sends the correct token type
 * @param loadedToken - token which will be send
 * @param curChar
 * @return 0 if everything is ok else 99 for allocation error
 */
int getIDKeyword (token_t *loadedToken, char *curChar)
{
    char *keywords[] = {"as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function", "if", "input",
                         "integer", "length", "loop", "print", "return", "scope", "string", "substr", "then", "while",
                         "and", "boolean", "continue", "elseif", "exit", "false", "for", "next", "not", "or", "shared",
                         "static", "true"};
    string id;

    if (strInit(&id)) return 99;

    do    // String assembling
    {
        if (*curChar >= 'A' && *curChar <= 'Z')
        {
            *curChar += 32;  // From uppercase to lowercase
            if (strAddChar(&id, *curChar)) return 99;
            *curChar = getchar();
        }
        else if ((*curChar >= 'a' && *curChar <= 'z') || *curChar == '_' || (*curChar >= '0' && *curChar <= '9'))
        {
            if (strAddChar(&id, *curChar)) return 99;
            *curChar = getchar();
        }
        else break;
    } while (true);

    for (int i = 0; i < 35; i++)    // Finding if string is a keyword
    {
        if (!strCmpConstStr(&id, keywords[i])) {
            strFree(&id);
            return setTokenType(loadedToken, i);
        }
    }

    loadedToken->type = TOK_identifier;
    strCopyString(loadedToken->value.stringVal, &id);
    strFree(&id);
    return 0;
}

/**
 * Assembles ints or floats and then sends the correct token type
 * @param loadedToken - token which will be send
 * @param curChar
 * @return 0 if everything is ok, 1 if is fault in number string else 99 for allocation error
 */
int getNumber (token_t *loadedToken, char *curChar)
{
    string id;
    bool decDot = false, sciNotation = false;
    char previousChar;

    if (strInit(&id)) return 99;
    strAddChar(&id, *curChar);
    previousChar = *curChar;
    setTokenType(loadedToken, TOK_integer);

    while (true)    // Number assembling
    {
        *curChar = getchar();
        if (*curChar >= '0' && *curChar <= '9')
        {
            if (strAddChar(&id, *curChar)) return 99;
        }
        else if (*curChar == '.' && !sciNotation && !decDot)
        {
            if (strAddChar(&id, *curChar)) return 99;
            decDot = true;
            setTokenType(loadedToken, TOK_decimal);
        }
        else if ((*curChar == 'e' || *curChar == 'E') && !sciNotation && previousChar >= '0' && previousChar <= '9') {
            if (strAddChar(&id, 'e')) return 99;
            sciNotation = true;
            setTokenType(loadedToken, TOK_decimal);
        }
        else if ((*curChar == '+' || *curChar == '-') && (previousChar == 'e' || previousChar == 'E'))
        {
            if (strAddChar(&id, *curChar)) return 99;
        }
        else
        {
            if (previousChar >= '0' && previousChar <= '9') break;
            else return 1;
        }
        previousChar = *curChar;
    }

    if (loadedToken->type == TOK_integer)   // Converts string to correct data type
    {
        if (sscanf(strGetStr(&id), "%d", &loadedToken->value.integer) != 1) return 99;
    }
    else
    {
        if (sscanf(strGetStr(&id), "%lf", &loadedToken->value.decimal) != 1) return 99;
    }
    return 0;
}

/**
 * Assembles string and sends it as token
 * @param loadedToken - token which will be send
 * @return 0 if everything is ok, 1 if is fault in string else 99 for allocation error
 */
int getString(token_t *loadedToken)
{
    string id;
    char curChar, previousChar = '0';   // The previousChar must be initialized for while loop
    int escapedChar;

    if (strInit(&id)) return 99;
    setTokenType(loadedToken, TOK_string);

    while (true)
    {
        curChar = getchar();
        if (curChar == '\"' && previousChar != '\\') break;
        if (curChar == '\n' || curChar == EOF) return 1;
        if (curChar == '\\')    // Escape sequences part
        {
            curChar = getchar();
            switch (curChar)
            {
                case 'n':
                    if (strAddChar(&id, '\n')) return 99;
                    break;
                case 't':
                    if (strAddChar(&id, '\t')) return 99;
                    break;
                case '\"':
                    if (strAddChar(&id, '\"')) return 99;
                    break;
                case '\\':
                    if (strAddChar(&id, '\\')) return 99;
                    break;
                case '0' ... '9':
                    if ((escapedChar = escapedNumber(curChar)) == -1) return 1;
                    else
                    {
                        if (strAddChar(&id, escapedChar)) return 99;
                    }
                    break;
                default:
                    return 1;
            }
        } else
        {
            if (strAddChar(&id, curChar)) return 99;
        }
        previousChar = curChar;
    }
    strCopyString(loadedToken->value.stringVal, &id);
    strFree(&id);
    return 0;
}

int getToken(token_t *loadedToken)
{
    char currChar;
    int retCode;
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
            /// Comments and floating point division operator
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
                lastChar = '\n';
                return setTokenType(loadedToken, TOK_endOfLine);

            /// ID|keyword
            case '_':
            case 'A' ... 'Z':   /// Range in case is supported only in gcc compiler!
            case 'a' ... 'z':
                if (getIDKeyword(loadedToken, &currChar) == 99) return 99;
                else
                {
                    lastChar = isWhiteChar(currChar) ? getNotWhiteChar() : currChar;
                    useLastChar = true;
                    return 0;
                }

            /// Values
            case '0' ... '9':
                if ((retCode = getNumber(loadedToken, &currChar))) return retCode;
                else
                {
                    lastChar = isWhiteChar(currChar) ? getNotWhiteChar() : currChar;
                    useLastChar = true;
                    return 0;
                }
            case '!':
                currChar = getchar();
                if (currChar == '\"')
                {
                    if ((retCode = getString(loadedToken))) return retCode;
                    lastChar = '\"';
                    return 0;
                }
                else return 1;

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
            case '\\':
                lastChar = currChar;
                return setTokenType(loadedToken, TOK_divInt);
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
