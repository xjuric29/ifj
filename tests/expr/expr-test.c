#include "expr-test.h"

// Note: First token sent from parser so you have to set it manually

char input[EXPR_TESTSTR_LENGTH];
char expected[EXPR_TESTSTR_LENGTH];


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
                case 5:         // THIS TEST IS SUCCESFUL WHEN EXPECTED ERROR
                {
                        char *expr = "+i+i*i";
                        char *output = "ERROR";
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

#if TEST_NUM == 2
// === TEST 2 ===
// i + i * i i 
// Output: [ERROR]

void TEST_getToken(token_t *loadedToken)
{
	static int iter = 0;
	
	token_t id;
	id.type = TOK_identifier;
	token_t plus;
	plus.type = TOK_plus;
	token_t mul;
	mul.type = TOK_mul;
	token_t semicolon;
	semicolon.type = TOK_semicolon;
	
	switch(iter)
	{
		case 1:
		case 3:
                case 4:
			*loadedToken = id;
			break;
		case 0:
			*loadedToken = plus;
			break;
		case 2:
			*loadedToken = mul;
			break;
		default:
			*loadedToken = semicolon;
			break;
	}
	iter++;
}
#endif


#if TEST_NUM == 3
// === TEST 3 ===
// i + i * i * (i + i)
// Output: $iii*ii+*+

void TEST_getToken(token_t *loadedToken)
{
	static int iter = 0;
	
	token_t id;
	id.type = TOK_identifier;
	token_t plus;
	plus.type = TOK_plus;
	token_t mul;
	mul.type = TOK_mul;
	token_t semicolon;
	semicolon.type = TOK_semicolon;
        token_t lBrac;
	lBrac.type = TOK_lParenth;
        token_t rBrac;
	rBrac.type = TOK_rParenth;
	
	switch(iter)
	{
		case 1:
		case 3:
                case 6:
                case 8:
			*loadedToken = id;
			break;
		case 0:
                case 7:
			*loadedToken = plus;
			break;
		case 2:
                case 4:
			*loadedToken = mul;
			break;
                case 5:
			*loadedToken = lBrac;
			break;   
                case 9:
			*loadedToken = rBrac;
			break;   
		default:
			*loadedToken = semicolon;
			break;
	}
	iter++;
}
#endif

#if TEST_NUM == 4
// === TEST 4 ===
// i + (i + i * (i * (i + i * i ) + i) * i) * i
// Output: $iiiiiii*+*i+*i*+i*+

void TEST_getToken(token_t *loadedToken)
{
	static int iter = 0;
	
	token_t id;
	id.type = TOK_identifier;
	token_t plus;
	plus.type = TOK_plus;
	token_t mul;
	mul.type = TOK_mul;
	token_t semicolon;
	semicolon.type = TOK_semicolon;
        token_t lBrac;
	lBrac.type = TOK_lParenth;
        token_t rBrac;
	rBrac.type = TOK_rParenth;
	
	switch(iter)
	{
		case 2:
		case 4:
                case 7:
                case 10:
                case 12:
                case 14:
                case 17:
                case 20:
                case 23:
			*loadedToken = id;
			break;
		case 0:
                case 3:
                case 11:
                case 16:
			*loadedToken = plus;
			break;
		case 5:
                case 8:
                case 13:
                case 19:
                case 22:
			*loadedToken = mul;
			break;
                case 1:
                case 6:
                case 9:
			*loadedToken = lBrac;
			break;   
                case 15:
                case 18:
                case 21:
			*loadedToken = rBrac;
			break;   
		default:
			*loadedToken = semicolon;
			break;
	}
	iter++;
}
#endif
