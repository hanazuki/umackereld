#include <string.h>

int begin_with(char const *str, char const *prefix) {
  size_t prefix_len = strlen(prefix);
  return strlen(str) >= prefix_len && strncasecmp(str, prefix, prefix_len) == 0;
}

char const *after_colon(char const *str) {
  while (*str && *str != ':')
    ++str;
  while (*str && *str == ':')
    ++str;
  while (*str && *str == ' ')
    ++str;
  return str;
}

char *chomp(char *str) {
  size_t len = strlen(str);
  if (len > 0 && str[len - 1] == '\n') {
    str[len - 1] = '\0';
  }
  return str;
}
