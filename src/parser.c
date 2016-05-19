#include "parser.h"
#include "scanner_codes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void parser_error();

struct program prog;

char token[SIZE];
int type;

extern struct token_head *list;
struct token_info *next = NULL;

struct  hash_table table[BUCKET_COUNT];

void init_hash_buckets() {
	int i = 0;

	for(i = 0; i < BUCKET_COUNT; i++) {
		table[i].head = NULL;
	}
}

void init_symbol_table() {
	init_hash_buckets();
}

int hash_function() {
	unsigned ascii = token[0];
	unsigned index = ascii % BUCKET_COUNT;
	return index;
}

struct sym_entry *allocate_entry() {
	struct sym_entry *entry = NULL;
	entry = (struct sym_entry *)malloc(sizeof(struct sym_entry));
	if(entry != NULL) {
		memset(entry, 0, sizeof(struct sym_entry));
		return entry;
	}
	return NULL;
}

void insert_ident_in_symtab(int type) {

	unsigned index = hash_function();
	struct sym_entry *entry = allocate_entry();
	
	if(entry == NULL) {
                printf("Error: Unable to allocate memory\n");
                parser_error();
        }
	
	strcpy(entry->token, token);
	entry->type       = type;
	
	if(table[index].head == NULL) {
		entry->next       = NULL;
		table[index].head = entry;
	} else {
		entry->next       = table[index].head;
		table[index].head = entry;
	}
}

void print_symbol_table() {
	int i, j;
	struct sym_entry *entry;
	
	printf("\n=============\n");
	printf("Symbol Table\n");	
	printf("=============\n");

	for(i = 0; i < BUCKET_COUNT; i++) {
		if(table[i].head != NULL) {
			entry = table[i].head;
			while(entry != NULL) {
				printf("Identifier : %-10s", entry->token);
				if(entry->type == BOOL) {
					printf("*** Type : %s\n", "bool");
				} else {	
					printf("*** Type : %s\n", "int");
				}
				entry = entry->next;
			}
		}
	}
}

int hash_index(char *symbol) {
	unsigned ascii = symbol[0];
	unsigned index = ascii % BUCKET_COUNT;
	return index;
	
}

struct sym_entry *find_symbol(char *symbol) {
	int index = hash_index(symbol);
	struct sym_entry *entry;

	if(table[index].head != NULL) {
		entry = table[index].head;
		
		while(entry != NULL) {
			if(!strcmp(entry->token, symbol)) {
				return entry;
			}

			entry = entry->next;
		}
	}

	return NULL;
}

void free_symbol_table() {
	int i;
	struct sym_entry *entry, *next;
	
	for(i = 0; i < BUCKET_COUNT; i++) {
		if(table[i].head != NULL) {
			entry = table[i].head;
			while(entry != NULL) {
				next = entry->next;
				free(entry);
				entry = next;
			}
		}
	}
}

void insert_declaration_node(struct declarations *dec) {

	if(prog.dec.head == NULL && prog.dec.tail == NULL) {
                prog.dec.head = dec;
                prog.dec.tail = dec;

                prog.dec.head->next = NULL;
                prog.dec.head->prev = NULL; 

        } else {
                dec->next           = prog.dec.head;
                prog.dec.head->prev = dec;
                dec->prev           = NULL;
                prog.dec.head       = dec; 
        } 
}

struct declarations *create_dec_node() {
	struct declarations *dec;
	
	dec = (struct declarations *)malloc(sizeof(struct declarations));
	if(dec == NULL) {
		printf("Error: Memory Creation Failed\n");
		parser_error();
	}
	return dec;
}

struct token_info *create_token() {
	struct token_info *info;

	info = (struct token_info *)malloc(sizeof(struct token_info));
	if(info == NULL) {
		printf("Error: Memory Creation Failed\n");
		parser_error();
	}
	return info;
}

void fill_token(struct token_info *src, struct token_info *dest) {

	memcpy(dest, src, sizeof(struct token_info));
	dest->next = NULL;
	dest->prev = NULL;
}

void ins_token(struct token_head *list, struct token_info *info) {

	if(list->tail == NULL && list->head == NULL) {
                list->head = info;
                list->tail = info;

                list->head->next = NULL;
                list->head->prev = NULL;

        } else {
                info->next       = list->head;
                list->head->prev = info;
                info->prev       = NULL;
                list->head       = info;
        }

}

struct statement * create_statement() {
	struct statement *state;
	
	state = (struct statement *) malloc(sizeof(struct statement));
	if(state == NULL) {
		printf("Error: Memory Creation Failed\n");
		parser_error();
	}
	return state;
}

void insert_statement(struct statement *state) {


        if(prog.list.head == NULL) {
                prog.list.head = state;
                prog.list.tail = state;

                prog.list.head->next = NULL;
                prog.list.head->prev = NULL;
        } else {
                state->next              = prog.list.head;
                prog.list.head->prev     = state;
                state->prev              = NULL;
                prog.list.head           = state;
        }

}

void free_declarations() {
	struct declarations *temp, *tail = prog.dec.tail;

	while(tail != NULL) {
		temp = tail->prev;
		free(tail);
		tail = temp;
	}
}

void free_token_info(struct token_info *info) {
	struct token_info *temp;
	
	while(info) {
		temp = info->prev;
		free(info);
		info = temp;
	}

}

void free_statements() {
	struct statement *temp, *head = prog.list.tail;
	struct token_info *info;

	while(head != NULL) {
		temp = head->prev;
	
		if(head->type == WHILE_STATEMENT) {
			info = head->ident.wh.expr.tail;
			free_token_info(info);
			free(head);
		} else if(head->type == ASSIGNMENT) {
			info = head->ident.asgn.expr.tail;
			free_token_info(info);
			free(head);
		} else if(head->type == IF_STATEMENT) {
			info = head->ident.fi.expr.tail;
			free_token_info(info);
			free(head);
		} else if(head->type == WRITEINT) {
			info = head->ident.wint.expr.tail;
			free_token_info(info);
			free(head);
		} else { // statement with marker END, THEN, ELSE
			free(head);
		}

		head = temp;
	}
}

void free_prog_structure() {
	free_declarations();
	free_statements();
}

//start of the parsing code

void parser_error() {
        free_list();
        free_symbol_table();
	free_prog_structure();
        exit(1);
}


int term() {
	struct statement *state;
		
	if(!strcmp(next->text, "end")) {
		state = create_statement();
		state->type = END;
		fill_token(next, &state->ident.end.info);
		insert_statement(state);
	} else if(!strcmp(next->text, "then")) {
		state = create_statement();
		state->type = THEN;
		fill_token(next, &state->ident.end.info);
		insert_statement(state);
	}else if(!strcmp(next->text, "else")) {
		state = create_statement();
		state->type = ELSE;
		fill_token(next, &state->ident.end.info);
		insert_statement(state);
	}

	if(next->prev != NULL) {
		next = next->prev; 
	}

	return 1;
}

int ident_store() {
	strcpy(token, next->text);
	return term();
}

int ident_check(struct statement *state) {
	if(state->type == ASSIGNMENT) {
		strcpy(state->ident.asgn.var, next->text);
		state->ident.asgn.type = next->token_type;	  	
	}
	return term();
}

int check_type() {

	if(find_symbol(token)) {
		printf("Error >> Identifier Redeclaration : '%s' : line : %d\n", token, next->line_no);
		parser_error();
	}

	if(!strcmp(next->text, "bool")) {
		insert_ident_in_symtab(BOOL);
		type = BOOL;		
		return term();
	} else if(!strcmp(next->text, "int")) {
		insert_ident_in_symtab(INT);
		type = INT;
		return term();
	} else {
		printf(">> Parser Error: Type Error : Expected \"INT or BOOL\" keyword ");
                printf(" ** line : %d : pos : %d\n", next->line_no, next->pos);
                parser_error();
	}
}

int check_declarations() {
	int status;
	struct declarations *dec_node = create_dec_node();

	if(!strcmp(next->text, "var")) {
		 status = term()&&ident_store();

                if(status) {
			//copy of variable name (token global variable)
			strcpy(dec_node->var, token);
                        if(!strcmp(next->text, "as")) {
                                status = term()&&check_type();
                                if(status) {
					//type global variable
					dec_node->type = type;
                                        if(next->token_type == SC) {
						insert_declaration_node(dec_node);
                                                return term()&&check_declarations();
                                        } else {
                                                printf(">> Parser Error: ");
                                                printf(" ** line : %d\n", next->line_no - 1);
                                                parser_error();
                                        }
                                }
                        } else {
                                printf(">> Parser Error: Expected \"AS\" keyword ");
                                printf(" ** line : %d : pos : %d\n", next->line_no, next->pos);
                                parser_error();
                        }
                } else {
                        return status;
                }
	} else if(!strcmp(next->text, "begin")) {
		return 1;
	} else {
		printf(">> Parser Error: Missing \"BEGIN\" keyword \n");
                parser_error();
	}
}

int identify_valid_statement_start() {
	if(next->token_type == IDENTIFIER || (!strcmp(next->text, "if")) || \
		(!strcmp(next->text, "while")) ||  (!strcmp(next->text, "writeint"))) {
		return 1;
	}
		
	return 0;
}

int identify_valid_expr_start() {
	if(next->token_type == IDENTIFIER || next->token_type == NUMBER || \
		(next->token_type == BOOL_LIT) ||  next->token_type == LP) {
		return 1;
	}

	return 0;
}

int check_expression(struct statement *state) {

	if(identify_valid_expr_start())
		return check_simple_expr(state)&&check_comp(state);
	else 
		return 0;
}

int check_factor(struct statement *state) {
	
	struct token_info *info;
	int status;

	if(next->token_type ==  LP) {
		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}
		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}
		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}
		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}


		status = term()&&check_expression(state);
		if(status) {	
			if(state->type == ASSIGNMENT) {
				info = create_token();
				fill_token(next, info);	
				ins_token(&state->ident.asgn.expr, info);
			}
			if(state->type == WHILE_STATEMENT) {
				info = create_token();
				fill_token(next, info);	
				ins_token(&state->ident.wh.expr, info);
			}
			if(state->type == WRITEINT) {
				info = create_token();
				fill_token(next, info);	
				ins_token(&state->ident.wint.expr, info);
			}
			if(state->type == IF_STATEMENT) {
				info = create_token();
				fill_token(next, info);	
				ins_token(&state->ident.fi.expr, info);
			}


			return term();
		} else {
			return status;
		}
	}else if(identify_valid_expr_start()) {
		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}
		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}
		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}
		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}

		return term();
	} else {
		return 0;
	}

}

int check_mult_epsillon() {

	switch(next->token_type) {
		case COMPARE:
		case ADDITIVE:
		case RP:
		case SC:
			return 1;
	}

	if((!strcmp(next->text, "then")) || (!strcmp(next->text, "do"))) {
		return 1;
	}
	
	return 0;
}

int check_mult(struct statement *state) {
	struct token_info *info;
	if(next->token_type == MULTIPLICATIVE) {

		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}
		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}

		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}

		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}



		return term()&&check_term(state);
	} else if(check_mult_epsillon()) {
		return 1;
	} else {
		return 0;
	}
}

int check_add_epsillon() {

	switch(next->token_type) {
		case COMPARE:
		case RP:
		case SC:
			return 1;
	}

	if((!strcmp(next->text, "then")) || (!strcmp(next->text, "do"))) {
		return 1;
	}
	
	return 0;
}

int check_term(struct statement *state) {
	return check_factor(state)&&check_mult(state);
}

int check_add(struct statement *state) {
	struct token_info *info;

	if(next->token_type == ADDITIVE) {

		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}
		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}
		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}
		
		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}


		return term()&&check_simple_expr(state);
	} else if(check_add_epsillon()) {
		return 1;
	} else {
		return 0;
	}
}

int check_simple_expr(struct statement *state) {
	return check_term(state)&&check_add(state);
}

int check_comp_epsillon() {

	switch(next->token_type) {
		case RP:
		case SC:
			return 1;
	}

	if((!strcmp(next->text, "then")) || (!strcmp(next->text, "do"))) {
		return 1;
	}
	
	return 0;
}

int check_comp(struct statement *state) {
	struct token_info *info;

	if(next->token_type == COMPARE) {
		
		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}
		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}
		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}

		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}

		return term()&&check_expression(state);
	}
}



int check_expr(struct statement *state) {

	struct token_info *info;

	if(identify_valid_expr_start())
		return check_expression(state);
	else if(!strcmp(next->text, "readint")) {

		if(state->type == ASSIGNMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.asgn.expr, info);
		}

		if(state->type == WHILE_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wh.expr, info);
		}
		if(state->type == WRITEINT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.wint.expr, info);
		}
		if(state->type == IF_STATEMENT) {
			info = create_token();
			fill_token(next, info);	
			ins_token(&state->ident.fi.expr, info);
		}


		return term();
	} else {
		return 0;
	}
}

int check_assignment(struct statement *state) {
	int status = 0;
	status = ident_check(state);
	if(status) {
		if(next->token_type == ASGN) {
			return term()&&check_expr(state);
		} else {
			printf("Parser Error : Expected \":=\" *** But found : '%s'", next->text);
			printf(": line :%d\n", next->line_no);
			parser_error();
		}	
	} else {
		return status;
	}
}

int check_else_statement() {
	if(!strcmp(next->text, "else")) {
		return term()&&check_statement_seq();
	} else if(!strcmp(next->text, "end")) {
		return 1;
	} else {
		return 0;
	}
}

int check_if_statement(struct statement *state) {
	int status = 0;

	if(!strcmp(next->text, "if")) {
		status = term()&&check_expression(state);
		if(status) {
			insert_statement(state);
			return term()&&check_statement_seq()&&check_else_statement()&&term();
		} else {
			return status;
		}
	} else {
		return 0;
	}
}

int check_while_statement(struct statement *state) {
	int  status = 0;

	if(!strcmp(next->text, "while")) {
		status = term()&&check_expression(state);
		if(status) {
			insert_statement(state);
			return term()&&check_statement_seq()&&term();
		} else {
			return status;
		}
	}
}

int check_writeint(struct statement *state) {
	if(!strcmp(next->text, "writeint")) {
		return term()&&check_expression(state);
	}		
}

int check_statment() {
	struct statement *state = create_statement();
	int status = 0;

	if(next->token_type == IDENTIFIER) {
		state->type = ASSIGNMENT;
		status = check_assignment(state);
		insert_statement(state);
		return status;
	} else if(!strcmp(next->text, "if")) {
		state->type = IF_STATEMENT;
		return check_if_statement(state);
	} else if(!strcmp(next->text, "while")) {
		state->type = WHILE_STATEMENT;
		return  check_while_statement(state);
	} else if(!strcmp(next->text, "writeint")) {
		state->type = WRITEINT;
		status =  check_writeint(state);	
		insert_statement(state);
		return status;
	} 
}

int check_statement_seq() {

	int status = 0;

	if(identify_valid_statement_start()) { 
		status = check_statment();
                if(status) {
                        if(next->token_type == SC) {
                                return term()&&check_statement_seq();
                        } else {
                                printf(">> Parser Error:");
                                printf(" ** line : %d\n", next->line_no);
                                parser_error();
                        }
                } else {
                        return 0;
                }
	} else if((!strcmp(next->text, "end")) || (!strcmp(next->text, "else"))) {
		return 1;
	} else {
		printf("Parser Error: Invalid Statement ** line : %d\n", next->line_no);
		parser_error();
	}
}

void check_program() {

	int status;

	if(!strcmp(next->text, "program")) {
		status = term()&&check_declarations();
		if(status) {
			if(!strcmp(next->text, "begin")) {
				status = term()&&check_statement_seq();
	
				if(status) {		
					if(!strcmp(next->text, "end")) {
						term();
					} 				
				} else {
					printf(">> *** Parser Error ***\n");	
					parser_error();
				}
			} 		
		} else {
			printf(">> *** Parser Error ***\n");
			parser_error();
		}
	} else {
			printf(">> Parser Error : Program Keyword Missing \n");
			parser_error();
	}
}

void check_syntax() {
	next = list->tail;
	memset(&prog, 0, sizeof(struct program));
	check_program();
	print_tree(&prog);
	free_symbol_table();
	free_prog_structure();	
}



