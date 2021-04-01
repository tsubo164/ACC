#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strncpy(char * s1, const char *s2, size_t n);

#endif /* __STRING_H */
