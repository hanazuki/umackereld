#ifndef UMACKERELD_FILEUTIL_H
#define UMACKERELD_FILEUTIL_H
#include <stdarg.h>
#include <stdio.h>

FILE *fopenf(char const *mode, char const *fmt, ...)
    __attribute__((format(printf, 2, 3)));

FILE *vfopenf(char const *mode, char const *fmt, va_list ap);

char *fgets_close(char *s, size_t size, FILE *fp);

#endif
