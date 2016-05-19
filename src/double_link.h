
struct token_head {
	struct token_info *head;
	struct token_info *tail;
};

int list_init();
void print_scanner_list();
void free_list();

void insert_token(struct token_info *info);

