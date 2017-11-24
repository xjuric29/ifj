//hlavickovy soubor pro praci s nekonecne dlouhymi retezci
#ifndef STR_H
#define STR_H

#define STR_ERROR   1
#define STR_SUCCESS 0

typedef struct
{
  char* str;		// misto pro dany retezec ukonceny znakem '\0'
  int length;		// skutecna delka retezce
  int allocSize;	// velikost alokovane pameti
} string;

int strInit(string *s);
void strFree(string *s);
void strClear(string *s);
int strAddChar(string *s1, char c);
int strCopyString(string *s1, string *s2);
int strCmpString(string *s1, string *s2);
int strCmpConstStr(string *s1, char *s2);
int strCopyConst(string *s, char *c);

char *strGetStr(string *s);
int strGetLength(string *s);

#endif
