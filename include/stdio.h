#ifndef __STDIO_H
#define __STDIO_H

#include <stddef.h>
#include <stdarg.h>

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int puts(const char *s);

int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

#endif /* __STDIO_H */
