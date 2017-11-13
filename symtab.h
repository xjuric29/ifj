

/**********************************
TREBA SKONTROLOVAT, CO TREBA V TABULKE A NIE JE TO TAM
PRAVDEPODOBNE STRUKTURA NA TROJADRESNY KOD, TO SA PORIESI NESKOR
**********************************/


#ifndef SYMTAB_H
#define SYMTAB_H

#define ST_SIZE 100

#include <stdbool.h>
#include "str.h"

unsigned int hash_function(const char *str);
 
/*typedef enum
{
    st_el_var
    st_el_param

} el_type_t /// Rozlisovat medzi parametrom a premennymi je potrebne?*/


typedef union
{
    int integer;
    double decimal;
    string *stringVal;

} st_value_t;   /// bez rozsireni su toto jedine datove typy


typedef struct st_element_t
{
    char *key; /// Variable name
    bool defined;
    st_value_t val;
    struct st_element_t *next;
  //  el_type_t el_type;

} st_element_t;

typedef struct
{
    int params_n;
    struct st_element_t *first;
    struct st_element_t *next;

} st_params_t;


typedef struct st_localTable_t
{
    char *key; /// Function name
    bool defined;
    unsigned int local_size;
    unsigned int local_n;
    st_params_t *params;
    struct st_localTable_t *next;
    st_element_t *elements[];

} st_localTable_t; /// Local table for a fuction

typedef struct
{
    unsigned int global_size;
    unsigned int global_n;
    st_localTable_t *functions[];   
} st_globalTable_t;    /// Global table of functions

  
st_globalTable_t *st_global_init(unsigned int size);

st_localTable_t *st_local_init(unsigned int size);

st_localTable_t *st_add_func(st_globalTable_t *st_global, string *key);



#endif
