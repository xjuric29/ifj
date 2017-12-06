#ifndef BUILTIN_H
#define BUILTIN_H

#include "symtab.h"
#include "scanner.h"
#include "str.h"

typedef struct BuiltIn{
    bool Length;
    bool SubStr;
    bool Asc;
    bool Chr;
} BuiltInStructure;

extern BuiltInStructure BuiltInUnique;
//extern struct BuiltIn *BuiltIn;
int addBuiltTable(st_globalTable_t *GlobalTable);

#endif
