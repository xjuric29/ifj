#include "expr-test.h"

// Note: First token sent from parser so you have to set it manually

char input[EXPR_TESTSTR_LENGTH];
char expected[EXPR_TESTSTR_LENGTH];
int expectedRetVal = 0; // Default value


void TEST_generateInputStr(int testNum)
{
        expected[0] = '$';
        switch(testNum)
        {
                case 1: // Basic test from lecture
                {
                        char *expr = "i+i*i";
                        char *output = "iii*+";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break;
                }
                case 2: // Basic test for error (basic operators from lecture)
                {
                        char *expr = "i+i*ii";
                        char *output = "ERROR";
                        expectedRetVal = 2;
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break;
                }
                case 3: // Basic test for brackets (basic operators from lecture)
                {
                        char *expr = "i+i*i*(i+i)";
                        char *output = "iii*ii+*+";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break;
                }
                case 4: // Advanced test for brackets (basic operators from lecture)
                {
                        char *expr = "i+(i+i*(i*(i+i*i)+i)*i)*i";
                        char *output = "iiiiiii*+*i+*i*+i*+";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                case 5: 
                {
                        char *expr = "+i+i*i";
                        char *output = "ERROR";
                        expectedRetVal = 2;
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                case 6: // Basic test for all arithmetic operators
                {
                        char *expr = "i+i\\i*i/i-i";
                        char *output = "iiii*i/\\+i-";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                case 7: // Advanced test for all arithmetic operators
                {
                        char *expr = "i/(i+i/(i*i*i*i+i*-(i-i*i)/i)-i*i/i)+i-i-i/i/i*i*i/i-i";
                        char *output = "iiiii*i*i*i*+iii*-i/-/+ii*i/-/i+i-ii/i/i*i*i/-i-";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                case 8: // Advanced test for all arithmetic operators (excluding integer divide)
                {
                        char *expr = "(i+(i/(i*i/i+i)*i*i*i/i-i+i*i)/i*i-i)/(i*(i-i/i)-i)+i";
                        char *output = "iiii*i/i+/i*i*i*i/i-ii*+i/i*+i-iiii/-*i-/i+";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                case 9:
                {
                        char *expr = "i/i*i/i";
                        char *output = "ii/i*i/";
                        
                        strcpy(input, expr);
                        strcpy(expected+1, output);
                        break; 
                }
                default:
                        fprintf(stderr, "[ERROR] Invalid test number\n");
        }
        
        
}

token_t TEST_getFirstToken()
{
        return TEST_sendTokens();
}

token_t TEST_sendTokens()
{
        static int counter = 0;       // Create static variable for counter
        token_t testToken;      // Create testing token to be sent
        
        if(input[counter] != '\0')     // If not end of expression string
        {
                switch(input[counter]) // Set tokenType depending on char in expression
                {
                        case '+':
                                testToken.type = TOK_plus;
                                break;
                        case '-':
                                testToken.type = TOK_minus;
                                break;
                        case '\\':
                                testToken.type = TOK_divInt;
                                break;
                        case '*':
                                testToken.type = TOK_mul;
                                break;
                        case '/':
                                testToken.type = TOK_div;
                                break;
                        case '(':
                                testToken.type = TOK_lParenth;
                                break;
                        case ')':
                                testToken.type = TOK_rParenth;
                                break;
                        case 'i':
                                testToken.type = TOK_identifier;
                                break;
                }
                
                counter++;
        }
        else
                testToken.type = TOK_semicolon; // Token type that doesn't belong to expression anymore
                
       
        return testToken;
}

void TEST_getToken(token_t *loadedToken)
{
        *loadedToken = TEST_sendTokens();
}
