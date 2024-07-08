#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

void nonfatal_err(const char *fmt, ...);
void fatal_err(const char *fmt, ...);
void warning(const char *fmt, ...);
bool dir_exists(const char *path);
bool dir_is_empty(const char *path);
char *trim_whitespace(const char *str);

#endif  /* UTIL_H */
