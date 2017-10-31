// Přeloženo: gcc 5.4

#ifndef HTAB_H_INCLUDED
#define HTAB_H_INCLUDED

#include "hash_function.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct htab_listitem {
  char *key; //word find
  unsigned long data; //number of occurrences
  struct htab_listitem *next; //Pointer to another structure
};

typedef struct htabt {
  unsigned int arr_size; //Size of data field
  unsigned int n; //Number of htab_listitem in data
  struct htab_listitem *HashData[];
} htab_t;

/**Constructor, creates and initializes hash table
*/
htab_t *htab_init(unsigned int size);

/**Move constructor, creates and initializes hash table with data of table2, table2 stays empty
*/
htab_t *htab_move(unsigned int newsize, htab_t *t2);

/**@return Number of elements of hash_table t (n)
*/
size_t htab_size(htab_t *t);

/**@return Number of elements of field data (arr_size)
*/
size_t htab_bucket_count(htab_t *t);

/**Looking for key in hash_table if its not find, create struct with key
 *@return Pointer to struct with key
 **/
struct htab_listitem *htab_lookup_add(htab_t *t, const char *key);

/**Looking for key in hash_table, if its not find return NULL
*/
struct htab_listitem *htab_find(htab_t *t, char *key);

/**Calling function f for each element of hash_table
*/
void htab_foreach(htab_t *t, void (*f)(char* key, unsigned int data));

/**Find and remove struct with key, if doesnt exist return False
*/
bool htab_remove(htab_t *t,char *key);

/**Hash_table stays empty
*/
void htab_clear(htab_t *t);

/**Destructor, calling htab_clear
*/
void htab_free(htab_t *t);

#endif
