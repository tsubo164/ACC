#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>

#define PP_HASH_SIZE 1237 /* a prime number */
#define PP_MULTIPLIER 31

struct strbuf {
	size_t len;
	size_t alloc;
	char *buf;
};

struct macro_param {
    char *name;
    struct strbuf arg;
    struct macro_param *next;
};

struct macro_entry {
    char *name;
    char *repl;
    int is_func;
    struct macro_param *params;
    struct macro_entry *next;
};

struct macro_table {
    struct macro_entry *entries[PP_HASH_SIZE];
};

#define MAX_HIDESET 32
struct preprocessor {
    FILE *fp;
    struct strbuf *text;
    struct macro_table *mactab;
    const struct macro_entry *hideset[MAX_HIDESET];

    const char *filename;
    int y;
    int x;
    int prevx;

    int skip_depth;
};

extern struct preprocessor *new_preprocessor(void);
extern void free_preprocessor(struct preprocessor *pp);

extern int preprocess_file(struct preprocessor *pp, const char *filename);
extern const char *get_text(const struct preprocessor *pp);

#endif /* _H */
