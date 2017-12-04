/**
 * @brief Symbol Table
 * @author Peter Havan (xhavan00), Matej Stano (xstano04)
 */

#include "symtab.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************ JEDNODUCHE TESTY A PRIKLADY VOLANIA FUNKCII ************/
//TODO KONTROLY MALLOCKU, KED SA NIECO NEPODARI UVOLNIT NAALOKOVANE STRUKTURY VO FUNKCII A VRATIT NULL,

/*
int main()
{
    st_globalTable_t *glob = st_global_init(100);
    printf("%u\t%u\t%p\n", glob->global_size, glob->global_n, (void*)glob->functions[50]);
    string key;
    //   printf("%s\t%d\t%d\n", key.str, key.length, key.allocSize);
    strInit(&key);
    strAddChar(&key, 'A');
    strAddChar(&key, 'B');
    printf("%s\t%d\t%d\n", key.str, key.length, key.allocSize);
    st_add_func(glob, &key);
    st_localTable_t *tmp = st_find_func(glob, &key);
    printf("%s\t%d\t%d\n", tmp->key.str, tmp->key.length, tmp->key.allocSize);
    printf("%u\t%u\t%p\n", glob->global_size, glob->global_n, (void*)glob->functions[1]);
    string key_el;
    strInit(&key_el);
    strAddChar(&key_el, 'A');
    strAddChar(&key_el, 'C');
    st_add_element(glob, &key, &key_el, 'P');
    strAddChar(&key_el, 'A');
    strAddChar(&key_el, 'A');
    st_add_element(glob, &key, &key_el, 'P');
    strAddChar(&key_el, 'B');
    strAddChar(&key_el, 'D');
    st_add_element(glob, &key, &key_el, 'P');
    st_element_t *parametre = tmp->params->first;
    parametre->val.stringVal = malloc(sizeof(string));
    strInit(parametre->val.stringVal);
    strAddChar(parametre->val.stringVal, 'A');
    parametre->el_type = st_string;
    while(parametre != NULL){
        //parametre->val.integer = 5;
        //parametre->el_type = st_integer;
        printf("Parameter: %s je v poradi: %d\n", parametre->key.str, parametre->param_number);
        parametre = parametre->next_param;
    }
    st_element_t *Najdeny = st_find_element(glob, &key, &key_el);
    if (Najdeny != NULL){
        printf("Najdeny %s\n", Najdeny->key.str);
    }
    //struct st_localTable_t *st_local = glob->functions[50];
    //printf("%s\t%s\t%s\n\n", st_local->params->first->key.str, st_local->params->first->next_param->key.str, st_local->params->last->key.str);

    strFree(&key_el);
    strFree(&key);
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

	//printf("%d\n", hash);

    st_localTable_t *st_local = st_global->functions[hash];

    while(st_local != NULL)
    {
        if(!strCmpString(&st_local->key, key))
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
    if(st_local == NULL){
        return NULL;
    }
    strInit(&st_local->key);
    st_local->next = NULL;
    strCopyString(&st_local->key, key);
    if(st_global->functions[hash] != NULL)
    {
        st_local->next = st_global->functions[hash];
        st_global->functions[hash] = st_local;
        //st_global->functions[hash]->local_n++;
    }

    else
    {
        st_global->functions[hash] = st_local;
        //st_global->functions[hash]->local_n++;
        st_global->global_n++;
    }
    return st_local;
}

st_localTable_t *st_find_func(st_globalTable_t *st_global, string *key)
{
    if(st_global == NULL || key->str == NULL)
        return NULL;

    unsigned int hash = hash_function(key->str) % st_global->global_size;

	//printf("%d\n", hash);

    st_localTable_t *st_local = st_global->functions[hash];

    while(st_local != NULL)
    {
        if(!strCmpString(&st_local->key, key))
        {

            return st_local;
        }

        st_local = st_local->next;
    }

   return NULL;
}

st_element_t *st_find_element(st_globalTable_t *st_global, string *func_name, string *key)
{
    if(st_global == NULL || key->str == NULL || func_name->str == NULL)
        return NULL;

    st_localTable_t *function = st_find_func(st_global, func_name);

    unsigned int loc_hash = hash_function(key->str) % function->local_size;

    st_element_t *st_elem = function->elements[loc_hash];

    while(st_elem != NULL)
    {
        if(!strCmpString(&st_elem->key, key))
            return st_elem;
        st_elem = st_elem->next;
    }
    return NULL;
}



st_element_t *st_add_element(st_globalTable_t *st_global, string *func_name, string *key, char type)
{
    if(st_global == NULL || key->str == NULL)
        return NULL;

    unsigned int glob_hash = hash_function(func_name->str) % st_global->global_size;

    st_localTable_t *st_local = st_global->functions[glob_hash];

    while(st_local != NULL)
    {
        if(!strCmpString(&st_local->key, func_name))
            break;
        st_local = st_local->next;
    }
    unsigned int loc_hash = hash_function(key->str) % st_local->local_size;

    st_element_t *st_elem = st_local->elements[loc_hash];
    while(st_elem != NULL)
    {
        if(!strCmpString(&st_elem->key, key))
            return st_elem;
        st_elem = st_elem->next;
    }

    st_elem = malloc(sizeof(st_element_t));
    if (st_elem == NULL){return NULL;} //Error in malloc
    st_elem->next = NULL;
    st_elem->next_param = NULL;
    st_elem->param_number = -1; //Set to -1, if its parameter it will be changed later
    strInit(&st_elem->key);
    if (strCopyString(&st_elem->key, key)){ //error in realloc
        return NULL;
    }

    if(st_local->elements[loc_hash] != NULL)
    {
        st_elem->next = st_local->elements[loc_hash];
        st_local->elements[loc_hash] = st_elem;
        st_local->local_n++;
    }

    else
    {
        st_local->elements[loc_hash] = st_elem;
        //st_local->elements[loc_hash]->el_n++;
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
                st_local->params->params_n = 1;
                //st_local->params->params_n++;
                st_local->params->first = st_elem;
                st_local->params->last = st_elem;
                st_elem->param_number = 1; //Set order of parameter
            }
            else
            {
                st_local->params->params_n++;
                st_local->params->last->next_param = st_elem;
                st_elem->param_number = st_local->params->last->param_number + 1; //Set order of parameter
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
                        if (tmp->el_type == st_string){
                            if (tmp->val.stringVal != NULL){
                                strFree(tmp->val.stringVal);
                                free(tmp->val.stringVal);
                            }
                        }
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



bool st_element_move(st_localTable_t *func, st_element_t *Parameter, string *NewKey){
    unsigned int NewHash = hash_function(NewKey->str) % func->local_size;
    unsigned int OldHash = hash_function(Parameter->key.str) % func->local_size;

    //No need to move element
    if (OldHash == NewHash){
        return true;
    }

    st_element_t *Oldptr = func->elements[OldHash];
    st_element_t *OldptrPrevios; //We need to store one struct before struct we want to move
    if (Oldptr == NULL){
        return false;
    }

    if (strCmpString(&Parameter->key, &func->elements[OldHash]->key) == 0){ //Check first struct of queue
        func->elements[OldHash] = Oldptr->next;
    }else{
        // Check other structs of queue, and starts with second
        OldptrPrevios = Oldptr;
        Oldptr = OldptrPrevios->next; //Second struct
        while(Oldptr != NULL){ //if first ptr == NULL cycle doesnt start
            if (strCmpString(&Parameter->key, &Oldptr->key) == 0){
                OldptrPrevios->next = Oldptr->next;
                break;
            }else{
                OldptrPrevios = Oldptr;
                Oldptr = Oldptr->next;
            }
        }
    }

    //Put at beggining of NewHash
    Parameter->next = func->elements[NewHash];
    func->elements[NewHash] = Parameter;
    strClear(&Parameter->key);
    strCopyString(&Parameter->key, NewKey);
    return true;
}

// Přeloženo: gcc 5.4
