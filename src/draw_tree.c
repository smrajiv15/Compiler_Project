#include "parser.h"
#include "scanner_codes.h"
#include <string.h>


#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


#define container_of(ptr, type, member) ({            \
 const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
 (type *)( (char *)__mptr - offsetof(type,member) );})

int node_num =  4;
struct stack stack_expr;
struct program *free_pg;


void print_declarations(struct decl_list *list, FILE *fp, int parent) {
	struct declarations *dec = list->tail;

	while(dec != NULL) {
		fprintf(fp, "n%d [label=\"decl:'%s':", node_num, dec->var);

		if(dec->type == INT) {
			fprintf(fp, "%s\"];\n", "int");
		} else {	
			fprintf(fp, "%s\"];\n", "bool");
		}

		fprintf(fp, "n%d->n%d;\n", parent, node_num);
		node_num++;

		dec = dec->prev;
	}
}

void free_tree(struct tree_node *leaf) {

	if(leaf != NULL) {
		free_tree(leaf->left);
      		free_tree(leaf->right);
      		free(leaf);
  	}	
}

void free_post_tree(int flag) {
	struct statement *state = free_pg->list.tail;
	struct token_info *info;
	
	while(state != NULL) {
		if(state->type == WHILE_STATEMENT) {
			info = state->ident.wh.post.tail;
			free_token_info(info); //freeing the post order tokens
			free_tree(state->ident.wh.exp_tree);
		} else if(state->type == ASSIGNMENT) {
			info = state->ident.asgn.post.tail;
			free_token_info(info); //freeing the post order tokens
			free_tree(state->ident.asgn.exp_tree);
		} else if(state->type == IF_STATEMENT) {
			info = state->ident.fi.post.tail;
			free_token_info(info); //freeing the post order tokens
			free_tree(state->ident.fi.exp_tree);
		} else if(state->type == WRITEINT) {
			info = state->ident.wint.post.tail;
			free_token_info(info); //freeing the post order tokens
			free_tree(state->ident.wint.exp_tree);
		}
		state = state->prev;
	}

	if(flag) {
		parser_error();
	}
}

struct tree_node *create_tree_node() {
	struct tree_node *node = (struct tree_node *)malloc(sizeof(struct tree_node));

	if(node == NULL) {
		printf("Error: Memory Allocation Failed\n");
		free_post_tree(1);
	}

	memset(node, 0, sizeof(struct tree_node));
	return node;
}

char *print_string(int type, char *var_type) {
	
	if(type == BOOL) {
		strcpy(var_type, "bool");
	} else {	
		strcpy(var_type, "int");
	}
	return var_type;
}

int get_first_token(struct statement *state) {

	if(state->type == ASSIGNMENT) {
		return state->ident.asgn.expr.tail->line_no; 
	}else if(state->type == WRITEINT) {
		return state->ident.wint.expr.tail->line_no; 
	}

}	

void inorder(struct tree_node *root, FILE *fp)
{
    char var_type[SIZE]; 

    if(root)
    {
        inorder(root->left, fp);
	if(root->tok.token_type >= ADDITIVE) {
		//printing left tree
		if(root->left->error == 0) {
        		fprintf(fp, "n%d [label=\"%s:", root->left->node_id, root->left->tok.text);	
        		fprintf(fp, "%s\"];\n", print_string(root->left->type, var_type));
		} else {
			fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
						root->left->node_id, root->left->tok.text);
		}
		fprintf(fp, "n%d->n%d;\n", root->node_id, root->left->node_id);

		if(root->right->error == 0) {
			fprintf(fp, "n%d [label=\"%s:", root->right->node_id, root->right->tok.text);
        		fprintf(fp, "%s\"];\n", print_string(root->right->type, var_type));
		} else {
			fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
						root->right->node_id, root->right->tok.text);

		}
		fprintf(fp, "n%d->n%d;\n", root->node_id, root->right->node_id);
	
		if(root->error == 1) {
        		fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
						root->node_id, root->tok.text);
		} else {
        		fprintf(fp, "n%d [label=\"%s:", root->node_id, root->tok.text);
        		fprintf(fp, "%s\"];\n", print_string(root->type, var_type));
		}
	
	} else if(root->tok.token_type == BOOL_LIT || root->tok.token_type == BUILTFUNC_VALID) {

		if(!strcmp(root->tok.text, "readint")) {	
			fprintf(fp, "n%d [label=\"%s:", root->node_id, root->tok.text);
        		fprintf(fp, "%s\"];\n", print_string(root->type, var_type));			
		} else {
			fprintf(fp, "n%d [label=\"%s\"];\n", root->node_id, root->tok.text);
		}
	} else if(root->left == NULL || root->right == NULL) {
		if(root->type == INT || root->type == BOOL) {	
        		fprintf(fp, "n%d [label=\"%s:", root->node_id, root->tok.text);
        		fprintf(fp, "%s\"];\n", print_string(root->type, var_type));
		} else {
			printf("ELSE : %d\n", root->type);
		}	
	}
        inorder(root->right, fp);
    }
}

int is_ints(int left, int right) {
	if((left & right) == INT) {
		return 1;
	} else {
		return 0;
	}
}

void check_type_rules(struct tree_node *node, int right_type, int left_type) {
	
	int ret_code;

	switch(node->tok.token_type) {
		case MULTIPLICATIVE:
				ret_code = is_ints(right_type, left_type);
				if(ret_code) {
					node->error = 0;
				} else {
					printf("Error >> Invalid operand type for the operator");
					printf(": '%s' : line : %d : pos : %d \n", node->tok.text,\
								node->tok.line_no, node->tok.pos);		
					node->error = 1;
				}

				node->type  = INT;
				break;
		case COMPARE:
				ret_code = is_ints(right_type, left_type);

				if(ret_code) {
					node->error = 0;
				} else {
					printf("Error >> Invalid operand type for the operator");
					printf(": '%s' : line : %d : pos : %d \n", node->tok.text,\
								node->tok.line_no, node->tok.pos);	
					node->error = 1;
				}

				node->type  = BOOL;
				break;
		case ADDITIVE:
				ret_code = is_ints(right_type, left_type);
				if(ret_code) {
					node->error = 0;
				} else {
					printf("Error >> Invalid operand type for the operator");
					printf(": '%s' : line : %d : pos : %d \n", node->tok.text,\
								node->tok.line_no, node->tok.pos);	
					node->error = 1;
				}

				node->type  = INT;
				break;
	}
}

struct tree_node *construct_tree(struct token_head *post, FILE *fp) {
	struct token_info *info = post->tail;
	struct token_info *info1, *info2;
	struct stack exp_tree;
	struct sym_entry *entry;
	struct tree_node *node;
	
	init_stack(&exp_tree);
	
	while(info != NULL) {
		if(info->token_type == IDENTIFIER || info->token_type == NUMBER) {
			node = create_tree_node();
			node->node_id = node_num++;
			memcpy(&node->tok, info, sizeof(struct token_info));

			if(info->token_type == IDENTIFIER) {
				entry = find_symbol(node->tok.text);
				if(entry != NULL) {
					node->type = entry->type;
				} else {
					printf("Error >> Undefined Variable : %s", node->tok.text);
					printf(": line: %d : pos: %d\n", node->tok.line_no, node->tok.pos);
					free_post_tree(1);
				}
			} else {
				node->type = INT;
			}

			push(&node->tok, &exp_tree);			
		} else if (info->token_type == BUILTFUNC_VALID || (info->token_type == BOOL_LIT)) {
			node = create_tree_node();
			node->node_id = node_num++;
			memcpy(&node->tok, info, sizeof(struct token_info));

			if(!strcmp(node->tok.text, "readint")) {
				node->type = INT;
			}

			if(info->token_type == BOOL_LIT) {
				node->type = BOOL;
			}
 
			push(&node->tok, &exp_tree);	

		} else { //operators will fall in this case
			node = create_tree_node();
			node->node_id = node_num++;
			memcpy(&node->tok, info, sizeof(struct token_info));
		
			info1 = pop(&exp_tree);	
			info2 = pop(&exp_tree);	
			
			node->right = container_of(info1 ,struct tree_node, tok);	
			node->left  = container_of(info2 ,struct tree_node, tok);
			check_type_rules(node, node->right->type, node->left->type);
			push(&node->tok, &exp_tree);	
			
		}
		info = info->prev;
	}
	
	node = container_of(pop(&exp_tree), struct tree_node, tok);
	inorder(node, fp);
	return node;	
}

struct tree_node *print_expr(struct token_head *list, struct token_head *post, FILE *fp) {
	struct token_info *info = list->tail;
	struct stack info_stack;

	init_stack(&info_stack);
	while(info != NULL) {
		infix_to_postfix(info, post, &info_stack);
		info = info->prev; 
		
	}
	clear_stack(post,&info_stack);
	return construct_tree(post, fp);
}

struct tree_node *print_asgn(struct statement *state, FILE *fp) {
	struct token_info *info =  state->ident.asgn.expr.tail; 
	return print_expr(&state->ident.asgn.expr, &state->ident.asgn.post, fp);
}

struct statement *print_statement(struct statement  *state, FILE *fp, int parent) {
	int temp, temp2;
	struct sym_entry *entry = NULL;
	char type_buffer[SIZE];

	while(state) {
		if(state->type == ASSIGNMENT) {

			entry = find_symbol(state->ident.asgn.var);
			
			if(entry == NULL) {
				printf("Error >> Undefined variable : %s", state->ident.asgn.var);
				printf(": line: %d\n", get_first_token(state));
				free_post_tree(1);
			}

		 	fprintf(fp, "n%d [label=\"%s", node_num, state->ident.asgn.var);
			fprintf(fp, ": %s\"];\n", print_string(entry->type, type_buffer));	
			temp = node_num;
			node_num++;

			state->ident.asgn.exp_tree = print_asgn(state, fp);

			if(state->ident.asgn.exp_tree->error == 1) {						
				
				fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
						node_num, ":=");
				printf("Error >> Unmatched Operand types on either side of ':='");
				printf(": line: %d\n", get_first_token(state));
				fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
										node_num, ":=");					
			} else {
				if(entry->type == state->ident.asgn.exp_tree->type) {
		 			fprintf(fp, "n%d [label=\"%s\"];\n", node_num, ":=");
				} else {	

				    	printf("Error >> Unmatched Operand types on either side of ':='");
					printf(": line: %d\n", get_first_token(state));
					fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
										node_num, ":=");		
				}
			}
	
		 	fprintf(fp, "n%d->n%d;\n", node_num, temp);
		 	fprintf(fp, "n%d->n%d;\n", node_num, node_num - 1);
		 	fprintf(fp, "n%d->n%d;\n", parent, node_num);
			node_num++;
		} else if(state->type == WHILE_STATEMENT) {	
		 	fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "while");	
		 	fprintf(fp, "n%d->n%d;\n", parent, node_num);
			temp = node_num;
			node_num++;
			state->ident.wh.exp_tree = print_expr(&state->ident.wh.expr,\
								&state->ident.wh.post, fp);
		 	fprintf(fp, "n%d->n%d;\n", temp, node_num - 1);
			fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "stmt list");	
		 	fprintf(fp, "n%d->n%d;\n", temp, node_num);
			temp = node_num++;
			state = state->prev;
			if(state->type != END) {
				state = print_statement(state, fp, temp);	
			}

		} else if(state->type == WRITEINT) {
		 	state->ident.wint.exp_tree = print_expr(&state->ident.wint.expr,\
								 &state->ident.wint.post,fp);
			if(state->ident.wint.exp_tree->type == INT) {
				fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "writeint");
			} else {	
				printf("Error >> Expected integer operand for 'writeint'");
				printf(": line: %d\n", get_first_token(state));

				fprintf(fp, "n%d [label=\"%s\",fillcolor=\"/pastel13/1\"];\n",\
										node_num, "writeint");	
			}

		 	fprintf(fp, "n%d->n%d;\n", parent, node_num);
			fprintf(fp, "n%d->n%d;\n", node_num, node_num - 1);
			node_num++;
		} else if (state->type == END || state->type == ELSE) {
			return state;
		} else if(state->type == IF_STATEMENT) {
			fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "if");
		 	fprintf(fp, "n%d->n%d;\n", parent, node_num);
			temp2 = node_num;
			temp = node_num++;
			state->ident.fi.exp_tree = print_expr(&state->ident.fi.expr,\
								&state->ident.fi.post, fp);
			fprintf(fp, "n%d->n%d;\n", temp, node_num - 1);
			fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "then");
			fprintf(fp, "n%d->n%d;\n", temp, node_num);
			temp = node_num++;
			state = state->prev;
			state = print_statement(state, fp, temp);
			if(state->type == ELSE) {
				state = state->prev;
				fprintf(fp, "n%d [label=\"%s\"];\n", node_num, "else");
				fprintf(fp, "n%d->n%d;\n", temp2, node_num);
				temp = node_num++;
				state = print_statement(state, fp, temp);
			}	
		}

		state = state->prev;	
	}
}

void print_tree(struct program *prog) {
	FILE *fp;

	free_pg = prog;
        fp = fopen("AST.dot", "w+");

        if(prog) {
        fputs("digraph AST {\n ordering=out;\nnode[shape=box,style=filled,fillcolor=white];\n", fp);
			fputs("n1 [label=\"program\"];\n", fp);
                        fputs("n2 [label=\"decl list\"];\n", fp);
                        fputs("n1->n2;\n", fp);
                        fputs("n3 [label=\"stmt list\"];\n", fp);
                        fputs("n1->n3;\n", fp);
               
        }

       	print_declarations(&prog->dec, fp, 2);
	print_statement(prog->list.tail, fp, 3);
	form_asm(prog->list.tail);

	fputs("\n}", fp);
        fclose(fp);
	free_post_tree(0);
}
