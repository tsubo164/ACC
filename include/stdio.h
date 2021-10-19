#ifndef __STDIO_H
#define __STDIO_H

#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)

typedef struct _FILE FILE;

extern FILE *__stdinp;
extern FILE *__stdoutp;
extern FILE *__stderrp;
#define	stdin	__stdinp
#define	stdout	__stdoutp
#define	stderr	__stderrp

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);

int fgetc(FILE *stream);

int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int puts(const char *s);

int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

#endif /* __STDIO_H */
