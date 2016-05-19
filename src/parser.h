#include "scanner.h"
#include "double_link.h"
#include "tree_ops.h"

#define BUCKET_COUNT 26

#define BOOL 1
#define INT  2

//statement type
#define ASSIGNMENT   	1
#define IF_STATEMENT 	2
#define WHILE_STATEMENT	3
#define WRITEINT	4
#define END		5
#define THEN		6
#define ELSE		7

//ASSIGNMENT TYPE
#define ASGN_1 0 //READINT
#define ASGN_2 1 //EXPR

//Node Type
#define PROGRAM 0

void parser_error();
int get_token_type(char *text);
struct token_info *create_token();
void fill_token(struct token_info *src, struct token_info *dest);
void ins_token(struct token_head *list, struct token_info *info);
void free_token_info(struct token_info *info);
struct sym_entry *find_symbol(char *symbol);

struct sym_entry {
	char token[SIZE];
	unsigned type;
	unsigned reg;
	struct sym_entry *next;
	int frame_offset;
	int valid;
};

struct hash_table {
	struct sym_entry *head;
};

//AST data structures
struct declarations {
	char var[SIZE];
	int type;
	struct declarations *next;
	struct declarations *prev;
};

struct decl_list {
	struct declarations *head;
	struct declarations *tail;
};

struct while_state {
	struct token_head expr;
	struct token_head post;
	struct tree_node  *exp_tree;
};

struct assignment_state {
	char var[SIZE];
	int type;
	struct token_head expr;	
	struct token_head post;	
	struct tree_node  *exp_tree;
};

struct if_statement {
	struct token_head expr;
	struct token_head post;	
	struct tree_node  *exp_tree;
};

struct write_int {
	struct token_head expr;	
	struct token_head post;	
	struct tree_node  *exp_tree;
};

struct end_delimit {
	struct token_info info;
};

//statement sequence
union stat_ident {
	struct while_state wh;
	struct assignment_state asgn;
	struct if_statement fi;
	struct write_int wint;
	struct end_delimit end;	
};

struct statement {
	int type;
	union stat_ident ident;
	struct statement *next;
	struct statement *prev;
};

struct statement_list {
	struct statement *head; 
	struct statement *tail; 
};

struct program {
	struct decl_list dec;
	struct statement_list list;
};
