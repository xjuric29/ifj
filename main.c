#include <stdio.h>
#include "scanner.h"

int main ()
{
    token_t loadedToken;
    string a;
    strInit(&a);
    int retCode;
    loadedToken.type = 0;
    loadedToken.value.stringVal = &a; //dobre

    while (loadedToken.type != TOK_endOfFile) {
        if ((retCode = getToken(&loadedToken)) == 1) {
            printf ("Lex error!\n");
            return 1;
        }
        else if (retCode == 99) {
            printf ("Internal error!\n");
            return 99;
        }
        else if (loadedToken.type == TOK_identifier) printf ("Var name: %s\n", loadedToken.value.stringVal->str);
        else if (loadedToken.type == TOK_string) printf ("String: \n%s\n", loadedToken.value.stringVal->str);
        else if (loadedToken.type == TOK_integer) printf ("Int: %d\n", loadedToken.value.integer);
        else if (loadedToken.type == TOK_decimal) printf ("Dec: %lf %le\n", loadedToken.value.decimal,loadedToken.value.decimal);
        else if (loadedToken.type >= 0 && loadedToken.type < 35) printf ("Keyword: %d\n", loadedToken.type);
        else printf("Other token: %d\n", loadedToken.type);
    }
    strFree(&a);
    return 0;
}