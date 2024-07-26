#include "string.h"
#include "alloc.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* all functions have undefined behavior if argument is NULL */

bool string_endswith(const char *str, const char *with)
{
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
        if (strncmp(str, with, strlen(with)) == 0)
                return true;
        return false;
}

int first_char_idx(const char *str, char c)
{
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


