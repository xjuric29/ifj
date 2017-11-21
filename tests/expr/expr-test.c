#include "expr-test.h"

#define TEST_NUM 4

// Note: First token is always id, it's send from parser because it is in charge

#if TEST_NUM == 1
// === TEST 1 ===
// i + i * i
// Output: $iii*+

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
// Output: i i i * i i + * +

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
// Output: i i i i i i i * + * i + * i * + i * +

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
