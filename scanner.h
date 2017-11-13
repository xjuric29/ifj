#ifndef IFJ_COMPILATOR_SCANNER_H
#define IFJ_COMPILATOR_SCANNER_H

#include "str.h"
/**	@file token.h
 *	@brief Header file for scanner and token interface
 *	@author Jiri Jurica (xjuric29), Jiri Furda (xfurda00)
 *	@todo Does scanner create nonToken type or print error and die?
 */

 #define SUCCESS 0
 #define LEX_ERROR 1
 #define SYN_ERROR 2
 #define SEM_ERROR_FUNC 3
 #define SEM_ERROR_COMP 4
 #define SEM_ERROR_OTHER 5
 #define INTERNAL_ERROR 99

// --- Constants and structures ---

/**	@brief An enum type to determine token type
 *  @todo What means "Term" data type?
 */
typedef enum
{
    // KEY WORDS
    // Don't change order of keywords! This is important for correct identification
    KW_as,			/// Keyword "As"
    KW_asc,			/// Keyword "Asc"
    KW_declare,		/// Keyword "Declare"
    KW_dim,			/// Keyword "Dim"
    KW_do,			/// Keyword "Do"
    KW_double,		/// Keyword "Double"
    KW_else,		/// Keyword "Else"
    KW_end,			/// Keyword "End"
    KW_chr,			/// Keyword "Chr"
    KW_function,	/// Keyword "Function"
    KW_if,			/// Keyword "If"
    KW_input,		/// Keyword "Input"
    KW_integer,		/// Keyword "Integer"
    KW_length,		/// Keyword "Length"
    KW_loop,		/// Keyword "Loop"
    KW_print,		/// Keyword "Print"
    KW_return,		/// Keyword "Return"
    KW_scope,		/// Keyword "Scope"
    KW_string,		/// Keyword "String"
    KW_subStr,		/// Keyword "SubStr"
    KW_then,		/// Keyword "Then"
    KW_while,		/// Keyword "While"

    // -- Reserved words below (Maybe won't use them but they are part of extensions) --
    KW_and,			///  Reserved keyword "And"
    KW_boolean,		///  Reserved keyword "Boolean"
    KW_continue,	///  Reserved keyword "Continue"
    KW_elseif,		///  Reserved keyword "Elseif"
    KW_exit,		///  Reserved keyword "Exit"
    KW_false,		///  Reserved keyword "False"
    KW_for,			///  Reserved keyword "For"
    KW_next,		///  Reserved keyword "Next"
    KW_not,			///  Reserved keyword "Not"
    KW_or,			///  Reserved keyword "Or"
    KW_shared,		///  Reserved keyword "Shared"
    KW_static,		///  Reserved keyword "Static"
    KW_true,		///  Reserved keyword "True"

    // TOKEN TYPES
    TOK_identifier,
    TOK_dataType,
    TOK_keyword,		/// Specified in keywordType
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
    string *stringVal;
    int integer;
    double decimal;
} tokenValue_t;


///	@brief A structure type representing token - its type and value
typedef struct
{
    tokenType_t type;
    tokenValue_t value;
} token_t;

int getToken (token_t *loadedToken);

char getNotSpaceChar ();

#endif
