#ifndef __STDLIB_H
#define __STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);

long strtol(const char *str, char **endptr, int base);

#endif /* __STDLIB_H */
