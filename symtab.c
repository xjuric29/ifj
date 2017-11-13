#include "symtab.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    return 0;
}

unsigned int hash_function(const char *str)
{
    unsigned int h=0;
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
    h = 65599*h + *p;
    return h;
}

st_globalTable_t *st_global_init(unsigned int size)
{
    assert(size != 0);
    st_globalTable_t *st_global = NULL;
    
    st_global = malloc(sizeof(st_globalTable_t) + size*sizeof(st_localTable_t *));
    if(st_global == NULL)
    {
        fprintf(stderr, "Allocation error");
        return NULL;
    }
    
    for(unsigned int i = 0; i < size; i++)
        st_global->functions[i] = NULL;
    
    st_global->global_size = size;
    st_global->global_n = 0;
    
    return st_global;
}

st_localTable_t *st_local_init(unsigned int size)
{
    assert(size != 0);
    st_localTable_t *st_local = NULL;
    
    st_local = malloc(sizeof(st_localTable_t) + size*sizeof(st_element_t *));
    if(st_local == NULL)
    {
        fprintf(stderr, "Allocation error");
        return NULL;
    }
    
    for(unsigned int i = 0; i < size; i++)
        st_local->elements[i] = NULL;

    st_local->local_size = size;
    st_local->local_n = 0;
    st_local->next = NULL;
    st_local->defined = false;
    st_local->params = NULL;

    return st_local;
}

st_localTable_t *st_add_func(st_globalTable_t *st_global, string *key)
{
    if(st_global == NULL || key->str == NULL)
        return NULL;

    unsigned int hash = hash_function(key->str) % st_global->global_size;
    
    st_localTable_t *st_local = st_global->functions[hash];
    
    while(st_local != NULL)
    {
        if(strcmp(st_local->key, key->str))
        {
            if(st_local->defined == true)
            {
                fprintf(stderr, "multiple function definition");
                return NULL;
            }
            else
            {
                return st_local;
            }
        }

        st_local = st_local->next;
    }
    
    st_local = st_local_init(ST_SIZE);
    st_local->next = NULL;
    
    if(st_global->functions[hash] != NULL)
    {   
        st_local->next = st_global->functions[hash];
        st_global->functions[hash] = st_local;
        st_global->functions[hash]->local_n++;
    }
    
    else
    {
        st_global->functions[hash] = st_local;
        st_global->functions[hash]->local_n++;
        st_global->global_n++;
    }
    return st_local;
}

st_element_t st_add_element(st_globalTable_t *st_global, string *func_name, string *key)
{
    if(st_global == NULL || key->str == NULL)
        return NULL;

    unsigned int glob_hash = hash_function(func_name->str) % st_global->global_size;
    
    st_func = st_global->functions[glob_hash];

    while(st_func != NULL)
    {
        if(strcmp(st_func->key, key->str) == 0)
            break;
        st_func = st_func->next;
    }
}
