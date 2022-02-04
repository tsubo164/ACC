#ifndef __STDLIB_H
#define __STDLIB_H

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

void abort(void);
void exit(int status);

long strtol(const char *str, char **endptr, int base);
float strtof(const char *str, char **endptr);
double strtod(const char *str, char **endptr);

#endif /* __STDLIB_H */
