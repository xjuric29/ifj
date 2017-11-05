#ifndef PARSER_H
#define PARSER_H

#define SUCCESS 0
#define LEX_ERROR 1
#define SYN_ERROR 2
#define SEM_ERROR_FUNC 3
#define SEM_ERROR_COMP 4
#define SEM_ERROR_OTHER 5
#define INTERNAL_ERROR 99


int parse();
int program();
int FunctionDeclar();
int FunctionDefinition();
int ScopeBody();

#endif
