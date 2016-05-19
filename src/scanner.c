#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "scanner_codes.h"
#include "double_link.h"

unsigned long line_pos  = 0;
unsigned long curr_line = 0;


char get_nextchar(FILE *fp) {
	char c;
	
	c = getc(fp); 
	line_pos++;
	return c;	
}

int single_symbol_op(char op) {
	switch(op) {
		case '(':
			return LP;
		case ')':
			return RP;
		case ';':
			return SC;	
		case '*':
			return MULTIPLICATIVE;
		case '+':
			return ADDITIVE;
		case '-':
			return ADDITIVE;
		case ':':
			return ASGN;
		case '=':
		case '>':
		case '<':
		case '!':
			return COMPARE;
		default:
			return ERR_INVALID_SYM;
		
	}
}

void error_msg(unsigned err_code, char *token) {
	switch(err_code) {
		case ERR_INVALID_OP:
			printf("\n>>> Error: Token = '%s' *** Invalid Operator or Symbol --", token);
			break;
		case ERR_INVALID_KEY:
			printf("\n>>> Error: Token = '%s' *** Invalid Keyword --", token);
			break;
		case ERR_OUT_RANGE:
			printf("\n>>> Error: Token = '%ld' *** Number Out of Range --", atol(token));
			break;
		case LEX_ERROR:
			printf("\n>>> Error: Token = '%s' *** Lexical Error --", token);							break;
		case ERR_INVALID_IDENT:
			printf("\n>>> Error: Token = '%s' *** Invalid Identifier --", token);
			break;
		case ERR_INVALID_NUM:
			printf("\n>>> Error: Token = '%s' *** Invalid Number --", token);
			break;
		case ERR_NO_MEM:
			printf("\n>>> Error: Unable to allocate Memory\n");
			break;
		default:
			break;
	}
	return;
}

void print_error(struct error_info *info) {
	print_scanner_list();
	error_msg(info->err_code, info->token);
	printf("Line = %03ld, Pos = %2ld\n",info->info->line_info, info->info->pos_info);
	fclose(info->fp);
	free_list();
	exit(1); 	
}

void generate_error_info(unsigned long line_no, unsigned long pos, \
			     unsigned char *token, unsigned code, FILE *fp) 
{

	struct error_info e_info;
	struct file_info info;
	
	info.line_info  = line_no;
	info.pos_info   = pos;
	
	e_info.info     = &info;
	e_info.token    = token;
	e_info.err_code = code;
	e_info.fp       = fp;

	print_error(&e_info);
}

int check_keyword_validity(unsigned char *token) {
	switch(token[0]) {	
		case 'i':
			if((!strcmp(token, "if")) || (!strcmp(token, "int"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}		
		case 't':
			if(!strcmp(token, "then")){
				return KEYWORD;	
			}else if(!strcmp(token, "true")) {
				return BOOL_LIT;
			}else {
				return ERR_INVALID_KEY;
			}			
		case 'e':
			if((!strcmp(token, "else")) || (!strcmp(token, "end"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'b':
			if((!strcmp(token, "begin")) || (!strcmp(token, "bool"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'w':
			if((!strcmp(token, "while"))){
				return KEYWORD;	
			} else if(!strcmp(token, "writeint")) {
				return BUILTFUNC_VALID;
			} else {
				return ERR_INVALID_KEY;
			}
		case 'd':
			if((!strcmp(token, "do")))  {
				return KEYWORD;	
			}else if((!strcmp(token, "div"))) {  
				return MULTIPLICATIVE;
			}else {
				return ERR_INVALID_KEY;
			}
		case 'p':
			if((!strcmp(token, "program"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'v':	
			if((!strcmp(token, "var"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'a':
			if((!strcmp(token, "as"))) {
				return KEYWORD;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'r':
			if((!strcmp(token, "readint"))) {
				return BUILTFUNC_VALID;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'f':
			if((!strcmp(token, "false"))) {
				return BOOL_LIT;	
			} else {
				return ERR_INVALID_KEY;
			}
		case 'm':	
			if((!strcmp(token, "mod"))) {
				return MULTIPLICATIVE;	
			} else {
				return ERR_INVALID_KEY;
			}
		default:
			return ERR_INVALID_KEY;
	}
}

int fill_token_info(unsigned char *token, unsigned long line_no, \
					unsigned long pos, unsigned type) {
	struct token_info *info = NULL;
	
	info = malloc(sizeof(struct token_info));
	if(info == NULL) {
		return ERR_NO_MEM;		
	}
	strcpy(info->text, token);
	info->pos         = pos;
	info->line_no     = line_no;
	info->token_type  = type;
	
	insert_token(info);
	return SUCCESS;	
}

int check_double_op(unsigned char *token) {
	switch(token[0]) {	
		case ':':
			if(!strcmp(token, ":="))
				return ASGN;
			break;
		case '!':
			if(!strcmp(token, "!="))
				return COMPARE;
			break;
		case '<':
			if(!strcmp(token, "<="))
				return COMPARE;
			break;
		case '>':
			if(!strcmp(token, ">="))
				return COMPARE;
			break;
		default :
			return ERR_INVALID_SYM;
	}

}

void extract_token(FILE *fp) {
	char ch;
	int i = 0;
	int ret_code;
	unsigned long num;
	unsigned long token_pos = 0;
	unsigned char token[SIZE];
	unsigned err_flag = 0;
	
	curr_line++;

	while((ch = get_nextchar(fp)) != EOF) {
check:		i = 0;
		token_pos = 0;
		memset(token, 0, sizeof(token));

		if(islower(ch)) { //Keywords Scanner
			token_pos = line_pos;	
			token[i++] = ch;
			while(ch = get_nextchar(fp)) {
				if(ch == NEW_LINE || ch == SPACE) {
					break;	
				} else {	
					if(!islower(ch)) {
						err_flag = 1;
					}
					token[i++] = ch;
				}		
			}
			token[i] = '\0';	
			if(err_flag) {
				generate_error_info(curr_line, token_pos, \
							token,ERR_INVALID_KEY, fp);
			}

			ret_code = check_keyword_validity(token);

			if(ret_code == ERR_INVALID_KEY) {
				generate_error_info(curr_line, token_pos, token,ERR_INVALID_KEY,fp);
			}
			
			ret_code = fill_token_info(token, curr_line, token_pos, ret_code);
			if(ret_code == ERR_NO_MEM) {	
				generate_error_info(curr_line, token_pos, token,ERR_NO_MEM,fp);
			}
			goto check;
		} else if(ch == '%') { //Comment Scanner
			while((ch = get_nextchar(fp)) != '\n');
			goto check;
		} else if(isupper(ch)) { //Indentifier Scanner
			token[i++] = ch;
			token_pos  = line_pos;
			while(ch = get_nextchar(fp)) {
				if(ch == NEW_LINE || ch == SPACE) {
					break;								
				} else {	
					if(!isupper(ch) && !isdigit(ch)){
						err_flag = 1;		
					}
					token[i++] = ch;
				}
			}

			token[i] = '\0';
			if(err_flag) {				
				generate_error_info(curr_line, token_pos, token, ERR_INVALID_IDENT,fp);
			}

			ret_code = fill_token_info(token, curr_line, token_pos, IDENTIFIER);
			if(ret_code == ERR_NO_MEM) {	
				generate_error_info(curr_line, token_pos, token,ERR_NO_MEM,fp);
			}			
			goto check;
		} else if(ch == '\n') { //Newline Detector
	 		curr_line++;	
			line_pos = 0;
			continue;
		} else if((ret_code = single_symbol_op(ch)) > 0) {
			token[i++] = ch;
			token_pos  = line_pos;
			while(ch = get_nextchar(fp)) {					
				if(ch == NEW_LINE || ch == SPACE) {
					break;
				} else {
					token[i++] = ch;
				}
			}

			token[i] = '\0';
			num = strlen(token);
			
			if(num == 2) {
				ret_code = check_double_op(token);			
				if(ret_code < 0) {			
					generate_error_info(curr_line, token_pos, \
							  token, ERR_INVALID_OP,fp);				
				}										
                        } else if(num > 2) {	
				generate_error_info(curr_line, token_pos,  \
						   token, ERR_INVALID_OP,fp);				
			}
			
			ret_code = fill_token_info(token, curr_line, token_pos, ret_code);
			if(ret_code == ERR_NO_MEM) {	
				generate_error_info(curr_line, token_pos, token,ERR_NO_MEM,fp);
			}			
			goto check;									
		} else if(isdigit(ch)) {
			token[i++] = ch;
			token_pos  = line_pos;

			while(ch = get_nextchar(fp)) {
				if(ch == NEW_LINE || ch == SPACE) {
					break;
				} else {
					if(!isdigit(ch)){
						err_flag = 1;		
					}
					token[i++] = ch;
				}
			}	
			token[i] = '\0';
	
			if(err_flag) {				
				generate_error_info(curr_line, token_pos, token, \
								ERR_INVALID_NUM,fp);
			}
			num = atol(token);
				
			if(num > (((unsigned long)1 << 31) - 1)) {	
				generate_error_info(curr_line, token_pos, token,  \
								ERR_OUT_RANGE,fp);
			}

			ret_code = fill_token_info(token, curr_line, token_pos, NUMBER);
			if(ret_code == ERR_NO_MEM) {	
				generate_error_info(curr_line, token_pos, token,ERR_NO_MEM,fp);
			}
			goto check;			
		} else {
			if(ch != SPACE && ch != TAB) {				
				token_pos  = line_pos;
				token[i++] = ch;
				while(ch = get_nextchar(fp)) {
					if(ch != NEW_LINE || ch != SPACE) {
						break;
					} else {
						token[i++] = ch;
					}
				}	
				token[i] = '\0';	
				generate_error_info(curr_line, token_pos, token, \
					                          LEX_ERROR,fp);
			}
		}	
	}				
}

int main(int argc, char *argv[]) {
	FILE *fp;
	
	if(argc != 2) {
		printf("Error: Please input the TL source code\n");
		exit(1);
	}
	
	fp = fopen(argv[1], "r");
	if(fp == NULL) {
		printf("Error: Unable to open the input TL file\n");
		exit(1);
	}

	init_symbol_table();
	//calling scanner section of the code	
	list_init();
	extract_token(fp);
	print_scanner_list();

	//calling parser section of the code
	check_syntax();
	free_list();
	fclose(fp);
	return 0;
}
