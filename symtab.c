
#include "symtab.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************ JEDNODUCHE TESTY A PRIKLADY VOLANIA FUNKCII ************/

/*int main()
{
    st_globalTable_t *glob = st_global_init(100);
    printf("%u\t%u\t%p\n", glob->global_size, glob->global_n, (void*)glob->functions[50]);
    string key;
    printf("%s\t%d\t%d\n", key.str, key.length, key.allocSize);
    key.str = "haha";
    printf("%s\t%d\t%d\n", key.str, key.length, key.allocSize);
    st_add_func(glob, &key);
    st_localTable_t *tmp = st_add_func(glob, &key);
    printf("%s\t%d\t%d\n", tmp->key.str, tmp->key.length, tmp->key.allocSize);
    printf("%u\t%u\t%p\n", glob->global_size, glob->global_n, (void*)glob->functions[50]);
    string key_el;
    key_el.str = "argc";
    st_add_element(glob, &key, &key_el, 'P');
    st_delete(glob);
    return 0;
}*/

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
    st_local->declared = false;
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
        if(strCmpString(&st_local->key, key))
        {
            /*if(st_local->defined == true)
            {
                fprintf(stderr, "multiple function definition");
                return NULL;
            }
            else
            {
                return st_local;
            }*/
            
            return st_local;
        }

        st_local = st_local->next;
    }
    
    st_local = st_local_init(ST_SIZE);
    st_local->next = NULL;
    strCopyString(&st_local->key, key);
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

st_element_t *st_add_element(st_globalTable_t *st_global, string *func_name, string *key, char type)
{
    if(st_global == NULL || key->str == NULL)
        return NULL;

    unsigned int glob_hash = hash_function(func_name->str) % st_global->global_size;
    
    st_localTable_t *st_local = st_global->functions[glob_hash];

    while(st_local != NULL)
    {
        if(strCmpString(&st_local->key, key))
            break;
        st_local = st_local->next;
    }
    unsigned int loc_hash = hash_function(key->str) % st_local->local_size;
    
    st_element_t *st_elem = st_local->elements[loc_hash];
    while(st_elem != NULL)
    {
        if(strCmpString(&st_elem->key, key))
            return st_elem;
        st_elem = st_elem->next;
    }

    st_elem = malloc(sizeof(st_element_t));
    st_elem->next = NULL;
    strCopyString(&st_elem->key, key);

    if(st_local->elements[loc_hash] != NULL)
    {
        st_elem->next = st_local->elements[loc_hash];
        st_local->elements[loc_hash] = st_elem;
        st_local->elements[loc_hash]->el_n++;
    }

    else
    {
        st_local->elements[loc_hash] = st_elem;
        st_local->elements[loc_hash]->el_n++;
        st_local->local_n++;
    }

    switch(type)
    {
        case 'V':
            break;
        case 'P':
            if(st_local->params == NULL)
            {
                st_local->params = malloc(sizeof(st_params_t));
                st_local->params->params_n++;
                st_local->params->first = st_elem;
                st_local->params->last = st_elem;
            }
            else
            {
                st_local->params->params_n++;
                st_local->params->last->next_param = st_elem;               
                st_local->params->last = st_elem;
            }
            break;
    }
    
    return st_elem;
    
}

void st_delete(st_globalTable_t *st_global)
{
    for(unsigned int i = 0; i < st_global->global_size; i++)
    {   
        st_localTable_t *st_local = st_global->functions[i];
        while(st_local != NULL)
        {
            for(unsigned int j = 0; j < st_local->local_size; j++)
            {
                    st_element_t *st_element = st_local->elements[j];
                    while(st_element != NULL)
                    {
                        st_element_t *tmp = st_element;
                        st_element = st_element->next;
                        strFree(&tmp->key);
                        free(tmp);   
                    }
            }
         //   free(st_local->elements);
            st_localTable_t *tmp = st_local;
            st_local = st_local->next;
            if(tmp->params != NULL)
                free(tmp->params);
            strFree(&tmp->key);
            free(tmp);
            
        }
    }
    free(st_global);
}

// Přeloženo: gcc 5.4
