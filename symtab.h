/**********************************
TREBA SKONTROLOVAT, CO TREBA V TABULKE A NIE JE TO TAM
PRAVDEPODOBNE STRUKTURA NA TROJADRESNY KOD, TO SA PORIESI NESKOR
**********************************/


#ifndef SYMTAB_H
#define SYMTAB_H

#define ST_SIZE 100

#include <stdbool.h>
#include "str.h"

 
typedef enum
{
    st_integer,
    st_decimal,
    st_string

} el_type_t;


typedef union
{
    int integer;
    double decimal;
    string *stringVal;

} st_value_t;   /// bez rozsireni su toto jedine datove typy


typedef struct st_element_t
{
    string key; /// Variable name
    unsigned int el_n;
    bool defined;
    st_value_t val;
    struct st_element_t *next_param; ///NULL if element is not a parameter or if element is last parameter
    struct st_element_t *next;
    el_type_t el_type;

} st_element_t;

typedef struct
{
    int params_n;
    struct st_element_t *first;
    struct st_element_t *last;

} st_params_t;


typedef struct st_localTable_t
{
    string key; /// Function name
    bool declared;
    bool defined;
    unsigned int local_size;
    unsigned int local_n;
    st_params_t *params;
    struct st_localTable_t *next;
    st_element_t *elements[];

} st_localTable_t; /// Local table for a fuction

typedef struct st_globalTable_t
{
    unsigned int global_size;
    unsigned int global_n;
    st_localTable_t *functions[];   
} st_globalTable_t;    /// Global table of functions

/*******************************************************/

/**
* @brief use at the beginning of parser, everything is in here
* 
* Function allocates memory for global hash table and initializes it
* 
* @param size requested size of the table
* @return pointer to global hash table
*
*/
st_globalTable_t *st_global_init(unsigned int size); 

/**
* @brief for adding/finding function in global hash table
* 
* Looks for the function name in global hash table
* if the function allocates memory for local hash table and initializes it
* 
* @param st_global searched global hash table
* @param key function id
* @return pointer to local hash table (function)
*
*/

st_localTable_t *st_add_func(st_globalTable_t *st_global, string *key); ///

/**
* @brief for adding/finding an element (variable) in local hash table (function) 
* 
* Looks for the variable name in local hash table
* if the element is not found, it is added there
* if the element is a parameter, it is added into parameter list
* 
* @param st_global searched global hash table
* @func_name function id in which we are looking for key
* @key variable id
* @param type determines whether the element is variable or paramater ('P' for parameter, 'V' for variable)
* @return pointer to the element
*
*/

st_element_t *st_add_element(st_globalTable_t *st_global, string *func_name, string *key, char type);

void st_delete(st_globalTable_t *st_global);



#endif
