#include "parser.h"
#include "scanner_codes.h"
#include <string.h>


void init_stack(struct stack *info_stack) {
	memset(info_stack, 0, sizeof(struct stack));
	info_stack->top = -1; 
}

int is_empty(struct stack *info_stack) {
	return info_stack->top == -1;
}

struct token_info *peek(struct stack *info_stack) {
	return info_stack->info[info_stack->top];
}

struct token_info *pop(struct stack *info_stack) {
	if(!is_empty(info_stack))
		return info_stack->info[info_stack->top--];
	return NULL;
}

void push(struct token_info *info, struct stack *info_stack) {
	info_stack->info[++info_stack->top] = info;
}

int precedence(int opr) {
	switch(opr) {
		case COMPARE:
			return 1;
		case ADDITIVE:
			return 2;
		case MULTIPLICATIVE:
			return 3;
	}
	return -1;
}

struct token_info *make_token() {
        struct token_info *info;

        info = (struct token_info *)malloc(sizeof(struct token_info));
        if(info == NULL) {
                printf("Error>> Memory Creation Failed\n");
		free_post_tree(1);
        }

        return info;
}

void insert_postfix(struct token_info *info, struct token_head *post) {
	struct token_info *temp = make_token();
	fill_token(info, temp);
	ins_token(post, temp);
}

void infix_to_postfix(struct token_info *info, struct token_head *post, struct stack *info_stack) {

	if(info->token_type == IDENTIFIER || info->token_type == NUMBER) {
		insert_postfix(info, post);
	} else if(!strcmp(info->text, "(")) {
		push(info, info_stack);
	} else if(!strcmp(info->text, ")")) {
		while(!is_empty(info_stack) && (strcmp((peek(info_stack)->text), "("))) {
			insert_postfix(pop(info_stack), post);	
		}
		pop(info_stack);
	} else {
		while(!is_empty(info_stack) && \
				precedence(info->token_type) <= precedence(peek(info_stack)->token_type)) {	
			insert_postfix(pop(info_stack), post);	
		}
		push(info, info_stack);
	}	
}

void print_post(struct token_head *post) {
	struct token_info *node = post->tail;

        while(node != NULL) {
		printf("%s ", node->text);
                node = node->prev;
        }
	printf("\n");
}

void clear_stack(struct token_head *post, struct stack *info_stack) {
	while(!is_empty(info_stack)) {	
		insert_postfix(pop(info_stack), post);	
	}
//	print_post(post);	
}
