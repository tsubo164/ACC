#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);

char *strcpy(char * dst, const char *src);
char *strncpy(char * dst, const char *src, size_t len);

char *strchr(const char *s, int c);

#endif /* __STRING_H */
