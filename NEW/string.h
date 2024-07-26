#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

bool string_endswith(const char *str, const char *with);
bool string_startswith(const char *str, const char *with);
int first_char_idx(const char *str, char c);
char *path_remove_extension(const char *path);
char *trim_whitespace(const char *str);

#endif  /* STRING_H */
