#include "htable.h"
//WORK IN PROGRESS

//Constructor pre Tabulku funkcie
FunctionsHashT *HashInitFunc(unsigned int size){
  FunctionsHashT *t = NULL;
  if (size < 1){
    fprintf(stderr, "Size of Hash Table cannot be 0\n");
    return NULL;
  }
  t = malloc(sizeof(FunctionHashT) + size*sizeof(FuncStruct *)); //Malloc size of htab_t + size * pointer to struct of keys
  if (t == NULL){
    fprintf(stderr, "Cannot allocate memory for Hash Table\n");
    return NULL;
  }
  for (unsigned int i = 0; i < size; i++){ //All pointers in data shoud be NULL
    t->HashTData[i] = NULL;
  }
  t->arr_size = size; //Save size to hash_table
  t->n = 0;
  return t;
}

//Destructor pre Tabulku funkcie
void htab_free(htab_t *t){
  htab_clear(t);
  free(t);
  t = NULL;
}

//Function is looking for key in hash_table
//If key is in hash_table function returns pointer to struct with key
//If key is not in hash_table function put struct with key into ht
struct htab_listitem *htab_lookup_add(htab_t *t, const char *key){
  if (t == NULL || key == NULL){
    return NULL;
  }
  unsigned int hash = hash_function(key); //Create hash from key
  struct htab_listitem *Item = t->HashData[hash % t->arr_size];
  struct htab_listitem *NewOne = NULL;
  while(Item != NULL){
    if (strcmp(Item->key, key) == 0){ //If key is found return pointer to Item
      return Item;
    }
    Item = Item->next;
  }
  //If key wasnt found -> malloc new item
  NewOne = malloc(sizeof(*NewOne));
  if (NewOne == NULL){ //Check
    fprintf(stderr, "Cannot allocate memory for new struct; htab_lookup_add\n");
    return NULL;
  }
  NewOne->key = malloc((strlen(key) + 1) *sizeof(char)); //Malloc memory for key
  if (NewOne->key == NULL){
    free(NewOne);
    fprintf(stderr, "Cannot allocate memory for new struct; htab_lookup_add\n");
    return NULL;
  }
  strcpy(NewOne->key, key); //Copy key to struct
  NewOne->next = NULL;
  if(t->HashData[hash % t->arr_size] != NULL){ //If there is any list of keys
    NewOne->next = t->HashData[hash % t->arr_size]; //Put new key at the beggining of list
    t->HashData[hash % t->arr_size] = NewOne;
    t->HashData[hash % t->arr_size]->data = 0;
  }else{
    t->HashData[hash % t->arr_size] = NewOne;
    t->HashData[hash % t->arr_size]->data = 0;
  }
  t->n++; //Increase number of structures in hash_table
  return NewOne;
}

void htab_clear(htab_t *t){
  if (t != NULL){ //Check if pointer is right
    struct htab_listitem *Item = NULL;
    for(unsigned int i = 0; i < t->arr_size; i++){
      Item = t->HashData[i]; //Item set to data[i]
      while(Item != NULL){
        t->HashData[i] = Item->next; //Move data[i] pointer to next struct
        free(Item->key); //Free Item (previos data[i])
        free(Item);
        Item = t->HashData[i]; //Move pointer item to data[i]
      }
    }
  for(unsigned int j = 0; j < t->arr_size; j++){
    t->HashData[j] = NULL;
  }
  t->n = 0;
  }
}

/**Looking for key in hash_table, if its not find return NULL
*/
struct htab_listitem *htab_find(htab_t *t, char *key){
  if (t == NULL || key == NULL){
    return NULL;
  }
  unsigned int hash = hash_function(key); //Create hash from key
  struct htab_listitem *Item = t->HashData[hash % t->arr_size];
  while(Item != NULL){
    if (strcmp(Item->key, key) == 0){ //If key is found return pointer to Item
      return Item;
    }
    Item = Item->next;
  }
  return NULL;
}

/**Calling function f for each element of hash_table
*/
void htab_foreach(htab_t *t, void (*f)(char* key, unsigned int data)){
  if (t == NULL){
    fprintf(stderr, "ERROR: Invalid pointer to Hash Table; htab_foreach\n");
    return;
  }
  struct htab_listitem *Item;
  for (unsigned int i = 0; i < t->arr_size; i++){
    Item = t->HashData[i];
    while(Item != NULL){
      f(Item->key, Item->data);
      Item = Item->next;
    }
  }
}

//Vytvori novu HashTable a "preleje" do nej polozky z HashTable 2
//Create new HashTable and move struct from t2 to HastTable t1
htab_t *htab_move(unsigned int newsize, htab_t *t2){
  htab_t *t1;
  if ((t1 = htab_init(newsize)) == NULL){
    return NULL;
  }
  unsigned int Hash;
  struct htab_listitem *t2cmp;
  struct htab_listitem *t1cmp;
  for(unsigned int i = 0; i < t2->arr_size; i++){ //For in Data t2
    t2cmp = t2->HashData[i];
    while(t2cmp != NULL){
      Hash = hash_function(t2cmp->key); //Hash key of t2cmp
      t1cmp = t1->HashData[Hash%t1->arr_size]; //When we do % newsize -> index can be different from index in t2
      if (t1cmp == NULL){ //In case that there is no list
        t1->HashData[Hash%t1->arr_size] = t2cmp; //Put t2cmp into t1
        t2cmp = t2cmp->next; //Move to another one
        t2->HashData[i] = t2cmp; //At the and of cycle there is NULL
        t1->HashData[Hash%t1->arr_size]->next = NULL; //Set in struct in t1 next to NULL, so it doesnt point to another struct in t2
      }else{ //In case there is a list
        while(t1cmp->next != NULL){ //Until t1cmp->next != NULL
          t1cmp = t1cmp->next;
        }
        t1cmp->next = t2cmp; //Put t2cmp
        t2cmp = t2cmp->next; //Move to another one
        t2->HashData[i] = t2cmp;
        t1cmp->next->next = NULL; //In struct in t1 set next to NULL so it doesnt point to another struct in t1
      }
    }
  }
  t1->n = t2->n;
  t2->n = 0;
  return t1;
}

bool htab_remove(htab_t *t,char *key){
  if (t == NULL || key == NULL){
    fprintf(stderr, "Invalid parameters to function 'htab_remove'\n");
    return false;
  }
  if (htab_find(t, key) == NULL){
    return false;
  }
  unsigned int Hash = hash_function(key);
  struct htab_listitem *ptr = t->HashData[Hash%t->arr_size];
  struct htab_listitem *ptrPrevios; //We need to store one struct before struct we want to delete
  if (ptr == NULL){ //If queue is empty return false
    return false;
  }
  if (strcmp(ptr->key, key) == 0){ //Check first struct of queue
    t->HashData[Hash%t->arr_size] = ptr->next;
    free(ptr->key);
    free(ptr);
    return true;
  }
  // Check other structs of queue, and starts with second
  ptrPrevios = t->HashData[Hash%t->arr_size];
  ptr = ptrPrevios->next; //Second struct
  while(ptr != NULL){ //if first ptr == NULL cycle doesnt start
    if (strcmp(ptr->key, key) == 0){
      ptrPrevios->next = ptr->next;
      free(ptr->key);
      free(ptr);
      return true;
    }else{
      ptrPrevios = ptr;
      ptr = ptr->next;
    }
  }
  return false;
}

size_t htab_size(htab_t *t){
  return t->n;
}

size_t htab_bucket_count(htab_t *t){
  return t->arr_size;
}
