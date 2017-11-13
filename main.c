#include <stdio.h>
#include "scanner.h"

int main ()
{
    token_t loadedToken;
    string a;
    strInit(&a);
    loadedToken.value.stringVal = &a; //dobre

    while (loadedToken.type != TOK_endOfFile) {
        if (getToken(&loadedToken) == 1) {
            printf ("Error!\n");
            return 1;
        }
        else if (loadedToken.type == TOK_identifier) printf ("Var name: %s\n", loadedToken.value.stringVal->str);
        else if (loadedToken.type >= 0 && loadedToken.type < 35) printf ("Keyword: %d\n", loadedToken.type);
        else printf("Other token: %d\n", loadedToken.type);
    }
    return 0;
}