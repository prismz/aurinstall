#include "util.h"
#include "alloc.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

void nonfatal_err(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "error: ");

        if (fmt != NULL)
                vfprintf(stderr, fmt, ap);
        else
                fprintf(stderr, "no message?");
        fprintf(stderr, "\n");
}

void fatal_err(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "fatal error: ");

        if (fmt != NULL)
                vfprintf(stderr, fmt, ap);
        else
                fprintf(stderr, "no message?");
        fprintf(stderr, "\n");

        exit(1);
}

void warning(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "warning: ");

        if (fmt != NULL)
                vfprintf(stderr, fmt, ap);
        else
                fprintf(stderr, "no message?");
        fprintf(stderr, "\n");
}

/* returns false if path doesn't exist, or path is a file */
bool dir_exists(const char *path)
{
        if (path == NULL)
                return false;

        struct stat s;
        if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
                return true;
        return false;
}

bool dir_is_empty(const char *path)
{
        int n = 0;
        struct dirent *d;
        DIR *dir = opendir(path);
        if (dir == NULL)
                return true;

        while ((d = readdir(dir)) != NULL) {
                if (n++ > 2)
                        break;
        }
        closedir(dir);

        if (n <= 2)
                return 1;
        return 0;
}

char *path_join(const char *p1, const char *p2)
{
        if (p1 == NULL || p2 == NULL)
                return NULL;

        size_t p1_len = strlen(p1);
        size_t p2_len = strlen(p2);

        char *combined = safe_calloc(p1_len + p2_len + 128, 1);
        strcat(combined, p1);
        strcat(combined, "/");
        strcat(combined, p2);

        return combined;
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

int gunzip(const char *path)
{
        if (path == NULL)
                return 1;

        char *new_path = path_remove_extension(path);
        printf("gunzip %s to %s\n", path, new_path);

        FILE *fp = fopen(new_path, "wb+");
        if (!fp)
                return 1;

        gzFile file = gzopen(path, "r");
        if (!file)
                return 1;

        for (;;) {
                unsigned char buff[2048];
                int nread = gzread(file, buff, 2047);
                fwrite(buff, nread, 1, fp);
                buff[nread] = 0;

                if (nread > 2047)
                        continue;

                int err;
                const char *err_str = gzerror(file, &err);
                if (err) {
                        fprintf(stderr, "Error: %s\n", err_str);
                        return 1;
                }
                if (gzeof(file))
                        break;
        }

        gzclose(file);
        fclose(fp);
        free(new_path);

        return 0;
}

bool yesno_prompt(char *prompt, bool default_answer)
{
        char yc = 'y';
        char nc = 'N';
        if (default_answer) {
                yc = 'Y';
                nc = 'n';
        }

        char *buf_raw = safe_malloc(sizeof(char) * 1024);
        for (;;) {
                printf("%s [%c/%c] ", prompt, yc, nc);
                fgets(buf_raw, 1024, stdin);
                char *buf = trim_whitespace(buf_raw);

                if (strcmp(buf, "y") == 0 || strcmp(buf, "yes") == 0
                                || strcmp(buf, "Y\n") == 0) {
                        free(buf);
                        return true;
                }

                if (strcmp(buf, "n") == 0 || strcmp(buf, "no") == 0
                                || strcmp(buf, "N") == 0) {
                        free(buf);
                        return false;
                }

                if (strcmp(buf, "") == 0) {
                        free(buf);
                        return default_answer;
                }

                printf("response not understood.\n");
        }
}
