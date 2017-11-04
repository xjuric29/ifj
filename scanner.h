#ifndef IFJ_COMPILATOR_SCANNER_H
#define IFJ_COMPILATOR_SCANNER_H

typedef enum
{
    identifier, dataType, keyword, integer, decimal, string, leftParenth, rightParenth, startBlock, endBlock, plusOperator,
    minusOperator, mulOperator, divOperator, equalOperator, notEqualOperator, lessOperator, lessEqualOperator,
    greaterOperator, greaterEqualOperator, comma, semicolon, endOfLine, endOfFile
} tokenType;

/*
 * plusOperator: +, minusOperator: -, mulOperator: *, divOperator: /, equalOperator: =, notEqualOperator: <>,
 * lessOperator: <; lessEqualOperator: <=, greaterOperator: >, greaterEqualOperator: >=
 */

typedef union
{
    char *string;
    int integer;
    double decimal;
} tokenValue;

typedef struct
{
    tokenType type;
    tokenValue value;
} token;

int getToken (token *loadedToken);

#endif