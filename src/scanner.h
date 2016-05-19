#include <stdio.h>

#define SIZE 25

struct token_info {
	unsigned char text[SIZE];
	unsigned pos;
	unsigned line_no;
	unsigned token_type;
        struct token_info *next;
	struct token_info *prev;
};

struct error_info {
        struct file_info *info;
        unsigned char *token;
        unsigned err_code;
        FILE *fp;
};


struct file_info {
	unsigned long line_info;
	unsigned long pos_info;
};
