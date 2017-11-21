#ifndef IFJ_COMPILATOR_SCANNER_H
#define IFJ_COMPILATOR_SCANNER_H

typedef enum
{
    // TOKEN TYPES
    TOK_identifier,
    TOK_integer,
    TOK_decimal,
    TOK_string,
    TOK_lParenth,		/// Left parenthesis = "("
    TOK_rParenth,		/// Right parenthesis ")"
    TOK_startBlock,
    TOK_endBlock,
    TOK_plus,			/// Operator "+"
    TOK_minus,			/// Operator "-"
    TOK_mul,			/// Operator "*"
    TOK_div,			/// Operator "/"
    TOK_divInt,			/// Operator "\"
    TOK_equal,			/// Operator "="
    TOK_notEqual,		/// Operator "<>"
    TOK_less,			/// Operator "<"
    TOK_lessEqual,		/// Operator "<="
    TOK_greater,		/// Operator ">"
    TOK_greaterEqual,	/// Operator ">="
    TOK_comma,			/// Operator ","
    TOK_semicolon,		/// Operator ";"
    TOK_endOfLine,		/// "\n"
    TOK_endOfFile
} tokenType_t;



///	@brief An union type representing all possible data types of token value
typedef struct
{
//    string *stringVal;
    int integer;
    double decimal;
} tokenValue_t;


///	@brief A structure type representing token - its type and value
typedef struct
{
    tokenType_t type;
    tokenValue_t value;
} token_t;

#endif
