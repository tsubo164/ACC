#ifndef ESC_SEQ_H
#define ESC_SEQ_H

extern int char_to_escape_sequence(int ch, char *es);
extern void print_string_literal(FILE *fp, const char *src);
extern void make_string_literal(const char *src, char *dst, size_t n);

#endif /* _H */
