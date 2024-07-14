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

char *trim_whitespace(const char *str)
{
        if (str == NULL)
                return 0;

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

static bool is_integer(char *str)
{
        if (str == NULL)
                return false;

        size_t len = strlen(str);
        for (size_t i = 0; i < len; i++) {
                if (!isdigit(str[i]))
                        return false;
        }

        return true;
}

/* TODO: test this function. I added out of bounds check but with indexing
 * I may have made an off by one error. */
void integer_exclude_prompt(char *prompt, int *flags_array, int size)
{
        printf("%s (\"1 2 3\", \"1-3\") ", prompt);
        fflush(stdout);

        char *input_raw = safe_calloc(32, sizeof(char));
        ssize_t n = read(0, input_raw, 32);
        if (n < 0)
                return;

        char *input = trim_whitespace(input_raw);

        size_t len = strlen(input);
        if (len == 0)
                return;

        char *ptr = strtok(input, " ");
        while (ptr != NULL) {
                /* find the slash, used to check valid token later */
                int ptr_slash_idx = -1;
                size_t ptr_len = strlen(ptr);
                for (size_t i = 0; i < ptr_len; i++) {
                        if (ptr[i] == '-') {
                                ptr_slash_idx = i;
                                break;
                        }
                }

                if (is_integer(ptr)) {
                        int idx = atoi(ptr) - 1;
                        if (idx + 1 > size)
                                continue;

                        flags_array[idx] = 0;
                } else if (ptr_slash_idx != -1) {
                        int start, end;
                        sscanf(ptr, "%d-%d", &start, &end);

                        if (start > end) {
                                int temp = end;
                                end = start;
                                start = temp;
                        }

                        if (end > size)
                                end = size;

                        start--;
                        end--;

                        int i = start;
                        while (i <= end && (i + 1) < size) {
                                flags_array[i] = 0;
                                i++;
                        }
                } else {
                        fatal_err("invalid exclude number %s", ptr);
                }

                ptr = strtok(NULL, " ");
        }

        free(input);
        free(input_raw);
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
