#include "string.h"
#include "alloc.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct stringlist *stringlist_new(size_t cap)
{
        struct stringlist *list = safe_calloc(1, sizeof(struct stringlist));

        list->cap = cap;
        list->n = 0;
        list->list = safe_calloc(cap, sizeof(char *));

        return list;
}

void stringlist_free(struct stringlist *list)
{
        if (list == NULL)
                return;

        if (list->list == NULL)
                goto end;
        for (size_t i = 0; i < list->n; i++) {
                free(list->list[i]);
        }
        free(list->list);
end:
        free(list);
}

/*
 * returns index where string was added, in list->list
 * returns -1 on error
 */
int stringlist_append(struct stringlist *list, const char *str)
{
        if (str == NULL)
                return -1;

        if (list->n + 1 > list->cap) {
                list->list = safe_realloc(list->list,
                                (list->cap += 8) * sizeof(char *));
        }

        list->list[list->n++] = safe_strdup(str);

        return list->n - 1;
}

/* adds all of src to dest */
int stringlist_concat(struct stringlist *dest, const struct stringlist *src)
{
        for (size_t i = 0; i < src->n; i++) {
                int rc = stringlist_append(dest, src->list[i]);
                if (rc)
                        return 1;
        }

        return 0;
}

bool string_endswith(const char *str, const char *with)
{
        if (str == NULL || with == NULL)
                return false;

        int len = (int)strlen(str);
        int with_len = (int)strlen(with);

        /* a string cannot end with a string longer than itself */
        if (with_len > len)
                return false;
        if (len == with_len && strcmp(str, with) == 0)
                return true;

        len--;  /* for indexing */
        for (int i = with_len - 1; i >= 0; i--) {
                char wc = with[i];
                char sc = str[len--];
                if (wc != sc)
                        return false;
        }

        return true;
}

bool string_startswith(const char *str, const char *with)
{
        if (str == NULL || with == NULL)
                return false;

        if (strncmp(str, with, strlen(with)) == 0)
                return true;
        return false;
}

int first_char_idx(const char *str, char c)
{
        if (str == NULL)
                return -1;

        size_t len = strlen(str);
        for (size_t i = 0; i < len; i++) {
                if (str[i] == c)
                        return (int)i;
        }

        return -1;
}

/*
 * removes the last filetype extension
 * /home/anon/file.txt.gz -> /home/anon/file.txt
 * doesn't support escaping but all names are hardcoded
 */
char *path_remove_extension(const char *path)
{
        if (path == NULL)
                return NULL;

        int len = (int)strlen(path);
        len--;

        int i;
        for (i = len; i >= 0; i--) {
                char c = path[i];
                if (c == '.')
                        break;
                if (c == '/')  /* hit directory before extension - no file extension */
                        return safe_strdup(path);
        }

        /* got to start without slash or extension
         * -1 because we offset by one after strlen() */
        if (i == -1)
                return safe_strdup(path);

        char *buf = safe_calloc(i + 1, 1);
        strncpy(buf, path, i);
        return buf;
}

char *trim_whitespace(const char *str)
{
        if (str == NULL)
                return NULL;

        size_t trimmed_len;
        char *trimmed;
        int ti = 0;

        int len = (int)strlen(str);
        int start;
        int end = 0;
        for (start = 0; isspace(str[start]); start++);
        if (start == len)
                goto done;

        for (end = len - 1; end >= 0; end--) {
                if (!isspace(str[end]))
                        break;
        }
        end++;

done:
        trimmed_len = abs(end - start) + 1;
        trimmed = safe_calloc(trimmed_len, 1);
        for (int i = start; i < end; i++) {
                trimmed[ti++] = str[i];
        }

        return trimmed;
}
