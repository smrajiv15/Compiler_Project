
#include <stdio.h>
#include <stdlib.h>

#define NEW_LINE 10
#define SPACE	 32
#define TAB	 9

#define SUCCESS   0

//types
#define KEYWORD      	1
#define IDENTIFIER   	2
#define OPERATOR     	3
#define NUMBER       	4
#define LP		5
#define RP 		6
#define SC		7
#define ASGN		8
#define BOOL_LIT	9
#define ADDITIVE	10
#define COMPARE		11
#define MULTIPLICATIVE	12

// Valid Codes

#define SYMBOL_VALID 	   1
#define KEYWORD_VALID	   2
#define BUILTFUNC_VALID	   3

// Invalid Error Codes
#define ERR_INVALID_SYM    -1
#define ERR_INVALID_OP     -2
#define ERR_INVALID_KEY    -3
#define ERR_OUT_RANGE	   -4
#define LEX_ERROR	   -5
#define ERR_INVALID_IDENT  -6
#define ERR_INVALID_NUM	   -7

#define ERR_NO_MEM	   -100
