#include "scanner.h"
#include "scanner_codes.h"
#include "double_link.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct token_head *list = NULL;

int list_init() {
	list = (struct token_head *)malloc(sizeof(struct token_head));
	if(list == NULL)
		return ERR_NO_MEM; 
	list->head = NULL;
	list->tail = NULL;
}

void insert_token(struct token_info *info) {
	
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

void print_lexical_tokens(struct token_info *info) {
	int i = 0;
	
	switch(info->token_type) {
	
	case KEYWORD:
	case BUILTFUNC_VALID:
		while(info->text[i] != '\0') {
			printf("%c",toupper(info->text[i++]));
		}
		printf("\n");
                break;
	case IDENTIFIER:
		printf("ident(%s)\n", info->text); 
		break;
	case MULTIPLICATIVE:
		printf("MULTIPLICATIVE(%s)\n", info->text);
		break;
	case ADDITIVE:
		printf("ADDITIVE(%s)\n", info->text);
		break;
	case COMPARE:
		printf("COMPARE(%s)\n", info->text);
		break;
	case LP:	
		printf("LP(%s)\n", info->text);
		break;
	case RP:	
		printf("RP(%s)\n", info->text);
		break;
	case SC:	
		printf("SC(%s)\n", info->text);
		break;
	case ASGN:	
		printf("ASGN(%s)\n", info->text);
		break;
	case NUMBER:	
		printf("num(%s)\n", info->text);
		break;
	case BOOL_LIT:
		printf("boollit(%s)\n", info->text);
		break;
	}
}

void print_scanner_list() {
	struct token_info *last = list->tail;

	while(last != NULL) {
		print_lexical_tokens(last);
		last = last->prev;
	}
}

void free_list() {
	struct token_info *node_temp, *node = list->head;

	while(node != NULL) {
		node_temp = node->next;
		free(node);
		node = node_temp;
	}	
	
}


