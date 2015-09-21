#include <stdlib.h>
#include "fileutil.h"

FILE *vfopenf(char const *mode, char const *fmt, va_list ap) {
  char *path;
  if (vasprintf(&path, fmt, ap) == -1) {
    return NULL;
  }
  FILE *fp = fopen(path, mode);
  free(path);
  return fp;
}

FILE *fopenf(char const *mode, char const *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  FILE *fp = vfopenf(mode, fmt, ap);
  va_end(ap);
  return fp;
}

char *fgets_close(char *s, size_t size, FILE *fp) {
  if (!fp) {
    return NULL;
  }
  char *p = fgets(s, size, fp);
  fclose(fp);
  return p;
}
