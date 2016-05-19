#include "parser.h"
#include "scanner_codes.h"
#include <string.h>

int node_counter = -1;
int present_blk = 0;

int inc_node_counter() {
	return ++node_counter;
}

int get_node_counter() {
	return node_counter;
}

struct dep_stack {
	int top;
	int stack[SIZE];
};

int dis_empty(struct dep_stack *st) {
	return st->top == -1;
}

void d_push(struct dep_stack *st, int num) {
	st->stack[++st->top] = num;
}

int d_pop(struct dep_stack *st) {
	return st->stack[st->top--];
}

int d_peek(struct dep_stack *st) {
	return st->stack[st->top];
}

void print_dep(int blk, FILE *fp, struct dep_stack *st) {
	while(!dis_empty(st)) {
		fprintf(fp, "n%d->n%d\n", blk, d_pop(st));		
	}
}

void get_text(FILE *asm_handle, FILE *cfg_handle, struct dep_stack *st) {
	char line[SIZE];
	char block[SIZE];
	int  dep_blk = 0;
	int blk_entered = 0;
	int block_no = 0;
	
	while(fgets(line, SIZE, asm_handle) != NULL) {
		if(sscanf(line, "%[B0-9^':']", block)) {
			sscanf(block, "B%d:", &block_no);
			if(present_blk != block_no && blk_entered == 1) {
				fprintf(cfg_handle, "</table>>, fillcolor=\"/x11/white\", shape=box]\n");
				print_dep(get_node_counter(), cfg_handle, st);
				dep_blk = 0;
			}
			blk_entered = 1;
			present_blk = block_no;
			fprintf(cfg_handle, " \nn%d [label = <<table border=\"0\"><tr><td border=\"1\" colspan=\"5\">%s</td></tr>", \
										inc_node_counter(), block);
		} else {
			if(blk_entered) {
				if(sscanf(line, "%*[^B]B%d", &dep_blk) == 1) {	
					if(st->top != -1) {
						if(d_peek(st) != dep_blk - 1) {
							d_push(st, dep_blk - 1);
						}
					} else {
						if(d_peek(st) != dep_blk - 1) {
							d_push(st, dep_blk - 1);
						}
					}					
				}
				fprintf(cfg_handle, "<tr><td align=\"left\">%s</td></tr>", line);
			}
		}
	}

	fprintf(cfg_handle, "</table>>, fillcolor=\"/x11/white\", shape=box]\n");
	fprintf(cfg_handle, " }\n entry->n0\n n%d->exit\n", get_node_counter());
}

void set_stack(struct dep_stack *st) {
	st->top = -1;
	memset(st->stack, 0, SIZE);
}

void create_cfg() {
	struct dep_stack st;
	FILE *cfg_handle = NULL;
	FILE *asm_hadle  = NULL;
	
	set_stack(&st);

	cfg_handle = fopen("Control_Flow.dot", "w+");
	asm_hadle  = fopen("Assembly_Code.s", "r");
	
	fputs("digraph cfg {\n node [shape=none];\n edge [tailport = s];\n entry\n", cfg_handle);
	fputs(" subgraph cluster {\n color=\"/x11/white\"\n", cfg_handle);
	get_text(asm_hadle, cfg_handle, &st);
	fprintf(cfg_handle, "}");
	fclose(asm_hadle);
	fclose(cfg_handle);
}
