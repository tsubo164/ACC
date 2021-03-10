#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>

struct strbuf {
	size_t len;
	size_t alloc;
	char *buf;
};

struct preprocessor {
    FILE *fp;
    struct strbuf *text;
    const char *filename;
    int row;
    int col;
    int colprev;
};

extern struct preprocessor *new_preprocessor();
extern void free_preprocessor(struct preprocessor *pp);

extern int preprocess_text(struct preprocessor *pp, const char *filename);

#endif /* _H */
