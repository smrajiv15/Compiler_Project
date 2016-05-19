
struct stack {
        int top;
        struct token_info *info[100];
};

struct tree_node {
	struct tree_node *left, *right;
	struct token_info tok;
	int node_id;
	int type;
	int offset;
	int visited;
	int reg;
	int error;
};

void init_stack(struct stack *info_stack);
int is_empty(struct stack *info_stack);
struct token_info *peek(struct stack *info_stack);
struct token_info *pop(struct stack *info_stack);
void push(struct token_info *info, struct stack *info_stack);
