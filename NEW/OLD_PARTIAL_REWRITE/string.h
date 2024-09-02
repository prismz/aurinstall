#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdbool.h>

struct stringlist {
        size_t cap;
        size_t n;
        char **list;
};

struct stringlist *stringlist_new(size_t cap);
void stringlist_free(struct stringlist *list);
int stringlist_append(struct stringlist *list, const char *str);
int stringlist_concat(struct stringlist *dest, const struct stringlist *src);

bool string_endswith(const char *str, const char *with);
bool string_startswith(const char *str, const char *with);
int first_char_idx(const char *str, char c);
char *path_remove_extension(const char *path);
char *trim_whitespace(const char *str);

#endif  /* STRING_H */
