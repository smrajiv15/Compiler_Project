/* This file creates the ASM code from Abstract sysntax tree on the fly
 */

#include "parser.h"
#include "scanner_codes.h"
#include <string.h>

long frame_ptr = 4;
long block_counter = 0;
int asgn_flg = 1;
int register_next = -1;
int label_counter = 0;
int exit_jmp = 0;

int inc_blk_no() {
	return ++block_counter;
}

int get_blk_no() {
	return block_counter;
}

int next_frame_ptr() {
	return frame_ptr -= 4;
}

int get_frame_ptr() {
	return frame_ptr;
}

int next_reg() {
	register_next += 1;

	if (register_next > 9) {
		register_next = 0;
		return register_next;
	} else {
		return register_next;
	}
}

int get_reg() {
	return register_next;
}

void set_reg_counter() {
	register_next = -1;
}

int next_label() {
	return ++label_counter;
}

int get_label() {
	return label_counter;
}

void init_asgn(FILE *fp, int offset, int num) {
	fprintf(fp, "\tli $t%d, %d\n", next_reg(), num);
	fprintf(fp, "\tsw $t%d, %d($fp)\n\n", get_reg(), offset);
	set_reg_counter();
}

void set_frame(char *ident, FILE *fp) {
	
	struct sym_entry *entry = NULL;
	entry 			= find_symbol(ident);
	entry->valid 		= 1;
	entry->frame_offset 	= next_frame_ptr();
}

void asm_error(FILE *fp) {
	fclose(fp);
	free_post_tree(1);
}

int is_operator(int type) {
	if (type >= 10) 
		return 1;
	return 0;
}

void load_identifier(struct sym_entry *entry, FILE *fp) {
	fprintf(fp, "\tlw $t%d, %d($fp)\n" ,next_reg(), entry->frame_offset);
	return;
}

void load_number(int num, FILE *fp) {
	fprintf(fp, "\tli $t%d, %d\n", next_reg(), num);
}

void emit_comp_inst(char *comp_op, FILE *fp) {
	
	if(!strcmp(comp_op, "<=")) {
		fprintf(fp, "\tsle $t%d, ", next_reg());
	} else if(!strcmp(comp_op, ">=")) {
		fprintf(fp, "\tsge $t%d, ", next_reg());
	} else if(!strcmp(comp_op, ">")) {
		fprintf(fp, "\tsgt $t%d, ", next_reg());
	} else if(!strcmp(comp_op, "<")) {
		fprintf(fp, "\tslt $t%d, ", next_reg());
	} else if(!strcmp(comp_op, "!=")) {
		fprintf(fp, "\tsne $t%d, ", next_reg());
	} else if(!strcmp(comp_op, "=")) {	
		fprintf(fp, "\tseq $t%d, ", next_reg());
	}
}

void emit_oprerator_inst(struct tree_node *root, FILE *fp) {

	switch(root->tok.token_type) {
		case MULTIPLICATIVE:	
			if(!strcmp(root->tok.text, "*")) { 
				fprintf(fp, "\tmul $t%d, ", next_reg()); 
				return;
			} else if(!strcmp(root->tok.text, "div") || !strcmp(root->tok.text, "mod")) {
				fprintf(fp, "\tdiv ");
				return;
			}
			break;
		case ADDITIVE:
			if(!strcmp(root->tok.text, "+")) {
				fprintf(fp, "\tadd $t%d, ", next_reg());
			} else { //Sub Operator	
				fprintf(fp, "\tsub $t%d, ", next_reg());
			}
			break;
		case COMPARE:	
			emit_comp_inst(root->tok.text, fp);
	}
}

void in_order(struct tree_node *root, FILE *fp) {
	
	int num       		= 0;	
	struct sym_entry *entry = NULL;

	if(root) {
		in_order(root->left, fp);
		if(is_operator(root->tok.token_type)) {
			if(root->left->tok.token_type == NUMBER) {
				entry = find_symbol(root->left->tok.text);
				num = atoi(root->left->tok.text);
				load_number(num, fp);
				root->left->reg = get_reg();
			} else if(root->left->tok.token_type == IDENTIFIER) {
				entry = find_symbol(root->left->tok.text);
				load_identifier(entry, fp);
				root->left->reg = get_reg();
			} else { //opertor node
				fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), root->left->offset);				
				root->left->reg = get_reg();
			}

			if(root->right->tok.token_type == NUMBER) {
				entry = find_symbol(root->right->tok.text);
				num = atoi(root->right->tok.text);
				load_number(num, fp);
				root->right->reg = get_reg();
			} else if(root->right->tok.token_type == IDENTIFIER) {
				entry = find_symbol(root->right->tok.text);
				load_identifier(entry, fp);
				root->right->reg = get_reg();
			} else { //opertor node
				fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), root->right->offset);				
				root->right->reg = get_reg();
			}

			//addressing the root operator node
			emit_oprerator_inst(root, fp);
			fprintf(fp, "$t%d, $t%d\n", root->left->reg, root->right->reg);
	
			if(root->tok.token_type == MULTIPLICATIVE) {
				if(!strcmp(root->tok.text, "div")) {
					fprintf(fp, "\tmflo $t%d\n", next_reg());
				} else if(!strcmp(root->tok.text, "mod")) {
					fprintf(fp, "\tmfhi $t%d\n", next_reg());
				}
			}
			root->reg     = get_reg();
			fprintf(fp, "\tsw $t%d, %d($fp)\n\n", root->reg, next_frame_ptr());
			root->offset  = get_frame_ptr();
			root->visited = 1;
			set_reg_counter();    					 
		}

		in_order(root->right,fp);
	}
}

void emit_expr(struct tree_node *root, FILE *fp) {	
	in_order(root, fp);
}

void single_asgn_exp(struct tree_node *root, FILE *fp, struct sym_entry *entry) {

	struct sym_entry *mem_var = NULL;
	int number = 0;

	if(root->tok.token_type == BUILTFUNC_VALID) {
			init_asgn(fp, entry->frame_offset, 0);
			if(strcpy(root->tok.text, "readint")) {
				fprintf(fp, "\tli $v0, 5\n\tsyscall\n");
				fprintf(fp, "\tadd $t%d, $v0, $zero\n", next_reg());
				fprintf(fp, "\tsw $t%d, %d($fp)\n\n", get_reg(), entry->frame_offset);
				set_reg_counter();						
			}
			return;
		} else if(root->tok.token_type == NUMBER) {
			number = atoi(root->tok.text);
			init_asgn(fp, entry->frame_offset, number);
			return;
		} else if(root->tok.token_type == BOOL_LIT) {
			if(!strcmp(root->tok.text, "true")) {
				number = 1;
				init_asgn(fp, entry->frame_offset, number);
				return;
			} 
			init_asgn(fp, entry->frame_offset, number);
			return;
		} else if(root->tok.token_type == IDENTIFIER) {
			mem_var = find_symbol(root->tok.text);
			fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), mem_var->frame_offset);
			fprintf(fp, "\tsw $t%d, %d($fp)\n", get_reg(), entry->frame_offset);
			return;
		}

}

void emit_asgn_expr(struct statement *state, FILE *fp, struct sym_entry *entry) {

	struct tree_node *root = state->ident.asgn.exp_tree;
	struct sym_entry *mem_var = NULL;

	if(root->left == NULL && root->right == NULL)  {
		single_asgn_exp(root, fp, entry);
		set_reg_counter();
		return;		
	}

	emit_expr(root, fp);
	mem_var = find_symbol(state->ident.asgn.var);
	fprintf(fp, "\tsw $t%d, %d($fp)\n", root->reg, mem_var->frame_offset);
	fprintf(fp, "\n");
	set_reg_counter();
}

void single_loop_expr(struct tree_node *root, FILE *fp) {
	
	struct sym_entry *entry = NULL;
	entry = find_symbol(root->tok.text);
	int number = 0;

	if(root->tok.token_type == IDENTIFIER) {
		fprintf(fp, "\t lw $t%d, %d($fp)\n", next_reg(), entry->frame_offset);
		root->reg = get_reg();
		return;
	} else if(root->tok.token_type == NUMBER) {
		number = atoi(root->tok.text);
		fprintf(fp, "\tli $t%d, %d\n", next_reg(), atoi(root->tok.text));
		root->reg = get_reg();
		return;
	} else if (root->tok.token_type == BOOL_LIT) {
		if(!strcmp(root->tok.text, "true")) {
			fprintf(fp, "\tli $t%d, %d\n", next_reg(), 1);
			root->reg = get_reg();
			return; 
		} else {
			fprintf(fp, "\tli $t%d, %d\n", next_reg(), 0);
			root->reg = get_reg();
			return; 
		}
	}
}

int emit_wh_expr(struct statement *state, FILE *fp) {
	struct tree_node *root = state->ident.wh.exp_tree;
	
	if(root->left == NULL && root->right == NULL) {	
		single_loop_expr(root, fp);
		return 1;
	}
	emit_expr(root, fp);
	return 0;	
}

void emit_printint_syscall(int reg, FILE *fp) {
	fprintf(fp, "\tli $v0, 1\n");
	fprintf(fp, "\tadd $a0, $t%d, $zero\n\tsyscall\n", get_reg());
	
	fprintf(fp, "\tli $v0, 4\n");
	fprintf(fp, "\tla $a0, newline\n\tsyscall\n\n");	
}

void single_wint_expr(struct tree_node *root, FILE *fp) {

	struct sym_entry *entry = NULL;
	
	if(root->tok.token_type == IDENTIFIER) {
		entry = find_symbol(root->tok.text);
		fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), entry->frame_offset); 
		emit_printint_syscall(get_reg(), fp);	
	} else if(root->tok.token_type == NUMBER) {
		fprintf(fp, "\tli $t%d, %d\n", next_reg(), atoi(root->tok.text)); 
		emit_printint_syscall(get_reg(), fp);	
	} else { //Expression
		fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), root->offset);
		emit_printint_syscall(get_reg(), fp);	
	}
}

void emit_wint_expr(struct tree_node *root, FILE *fp) {

	if(root->left == NULL && root->right == NULL) {
		single_wint_expr(root, fp);
		set_reg_counter();
		return;
	}
	emit_expr(root, fp);
	single_wint_expr(root, fp);
	set_reg_counter();
}

int emit_if_expr(struct tree_node *root, FILE *fp) {

	if(root->left == NULL && root->right == NULL) {	
		single_loop_expr(root, fp);
		return 1;
	}
	emit_expr(root, fp);	
	return 0;
}

void set_jump_pos(FILE *fp, long pos, int blk_no) {

	long cur_pos = ftell(fp);
	
	fseek(fp, pos, SEEK_SET);
	fprintf(fp, "B%d\n", blk_no);
	fseek(fp, cur_pos, SEEK_SET);
	return;
}

void check_prev_state(struct statement *state, FILE *fp) {
	struct statement *bef_state = state->next;
	if(bef_state != NULL) {
		if(bef_state->type == END) {
			fprintf(fp, "B%d:\n", inc_blk_no());		
		}
	}
}

void check_if_prev(struct statement *state, FILE *fp) {
	struct statement *bef_state = state->next;
	if(bef_state->type != WHILE_STATEMENT) {
		fprintf(fp, "B%d:\n", inc_blk_no()); 
	}
}

void check_next_state(struct statement *state, FILE *fp) {
	struct statement *nxt_state = state->prev;
	if(nxt_state->type == IF_STATEMENT) {
		fprintf(fp, "\tj B%d\n", get_blk_no() + 1); 
	}
}

struct statement *gen_asm(struct statement *state, FILE *fp, int in_recur) {

	struct sym_entry *entry = NULL;
	long pos = 0, pos1 = 0;
	int s_block = 0;

	while(state) {
		if(state->type == ASSIGNMENT) {
			if(asgn_flg == 1) {
				set_frame(state->ident.asgn.var, fp);
			} else { 
				check_prev_state(state, fp);
			}
			
			entry = find_symbol(state->ident.asgn.var);
			if(entry->valid == 0) {
				printf("Error: >> Variable '%s' should be defined before use\n", state->ident.asgn.var);
				asm_error(fp);			
			}

			emit_asgn_expr(state, fp, entry);
			check_next_state(state, fp);
		} else if(state->type == WHILE_STATEMENT) {
			asgn_flg = 0;
			fprintf(fp, "\tj B%d\n", inc_blk_no());
			fprintf(fp, "B%d:\n", get_blk_no());
			s_block = get_blk_no();
			if(!emit_wh_expr(state, fp)) {
				fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), state->ident.wh.exp_tree->offset);
			}
			fprintf(fp, "\tbne $t%d, $zero, B%d\n\n", get_reg(), get_blk_no() + 1);
			fprintf(fp, "L%d:\n\t j ", next_label());
			set_reg_counter();
			pos = ftell(fp);
			fprintf(fp, "      \n");
			state = state->prev;
			if(state->type != END) {
				fprintf(fp, "\nB%d:\n", inc_blk_no());
				state = gen_asm(state, fp, 1);
				fprintf(fp, "\tj B%d\n", s_block);
				set_jump_pos(fp, pos, get_blk_no() + 1);
			}

		} else if(state->type == WRITEINT) {
			asgn_flg = 0;
			check_prev_state(state, fp);
			emit_wint_expr(state->ident.wint.exp_tree, fp);
			check_next_state(state, fp);
		} else if(state->type == END || state->type ==ELSE) {
			if(state->prev == NULL) {
				if(state->next->type == END) {
					exit_jmp = 1;
				}
			}
			return state;
		} else if(state->type == IF_STATEMENT) {
			asgn_flg = 0;
			check_if_prev(state, fp);

			if(!emit_if_expr(state->ident.fi.exp_tree, fp)) {
				fprintf(fp, "\tlw $t%d, %d($fp)\n", next_reg(), state->ident.fi.exp_tree->offset);
			}

                        fprintf(fp, "\tbne $t%d, $zero, B%d\n\n", get_reg(), get_blk_no() + 1);	//if true then part
			set_reg_counter();
			fprintf(fp, "L%d:\n\t j ", next_label()); //if false else part
			pos = ftell(fp);
			fprintf(fp, "\n\n\n\n");
			state = state->prev;
			//start of then block
			if(state->prev->type != IF_STATEMENT)
				fprintf(fp, "\nB%d:\n", inc_blk_no()); 
			state = gen_asm(state, fp, 1);	
			fprintf(fp, "L%d:\n\t j ", next_label()); //if false else part
			pos1 = ftell(fp);
			fprintf(fp, "\n\n\n\n");
			
			if(state->type == ELSE) {
				fprintf(fp, "B%d:\n", inc_blk_no());
				set_jump_pos(fp, pos, get_blk_no());
				state = state->prev;
				state = gen_asm(state, fp, 1);
				fprintf(fp, "\tj B%d\n", get_blk_no() + 1);	
				set_jump_pos(fp, pos1, get_blk_no() + 1);

			} else { // no else block	
				set_jump_pos(fp, pos, get_blk_no() + 1);
				set_jump_pos(fp, pos1, get_blk_no() + 1);

			}
		}
		state = state->prev;
	}	

}

void form_asm(struct statement *state) {
	FILE *fp;

	fp = fopen("Assembly_Code.s", "w+");
	
	fputs("\t.data\nnewline:\t.asciiz \"\\n\" \n\t.text\n\t.globl main\nmain:\n", fp); 
	fprintf(fp, "\tli $fp, 0x7ffffffc\n\n");
	fprintf(fp, "B%d:\n", inc_blk_no());
		
	gen_asm(state, fp, 0);
	if(!exit_jmp) {
		fprintf(fp, "\tj B%d\n", get_blk_no() + 1);
	} 
	fprintf(fp, "B%d:\n", inc_blk_no());
	fprintf(fp, "\t#Exit Routine\n\tli $v0, 10\n\tsyscall\n");
	fclose(fp);
	create_cfg();	
}
