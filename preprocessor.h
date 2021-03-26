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

struct macro_entry {
    char *name;
    char *repl;
    struct macro_entry *next;
};

struct macro_table {
    struct macro_entry *entries[PP_HASH_SIZE];
};

struct preprocessor {
    FILE *fp;
    struct strbuf *text;
    struct macro_table *mactab;
    const char *filename;
    int y;
    int x;
    int prevx;

    int skip_depth;
};

extern struct preprocessor *new_preprocessor(void);
extern void free_preprocessor(struct preprocessor *pp);

extern int preprocess_text(struct preprocessor *pp, const char *filename);

#endif /* _H */
