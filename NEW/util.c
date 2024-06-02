/*
 * This file is part of aurinstall.
 *
 * aurinstall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aurinstall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurinstall.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Copyright (C) 2023 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#include "util.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>

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

/* if path == NULL or dir doesn't exist, returns true */
bool dir_is_empty(const char *path)
{
        if (path == NULL)
                return true;

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

/* TODO: make better (remove leading & trailing whitespace) */
bool yesno_prompt(const char *prompt, bool default_answer)
{
        if (prompt == NULL)
                fatal_err("no prompt for yesno_prompt?");
                
        char yc = 'y';
        char nc = 'N';
        if (default_answer) {
                yc = 'Y';
                nc = 'n';
        }

        char *buf = safe_malloc(1024);
        for (;;) {
                printf("%s [%c/%c] ", prompt, yc, nc);
                fgets(buf, 1024, stdin);
                if (strcmp(buf, "y\n") == 0 || strcmp(buf, "yes\n") == 0) {
                        free(buf);
                        return true;
                }

                if (strcmp(buf, "n\n") == 0 || strcmp(buf, "no\n") == 0) {
                        free(buf);
                        return false;
                }

                if (strcmp(buf, "\n") == 0) {
                        free(buf);
                        return default_answer;
                }

                printf("response not understood.\n");
        }
}

char *get_user_home(void)
{
        char *path = safe_malloc(4096);
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL)
                fatal_err("user not found.");

        strncpy(path, pw->pw_dir, 4096);

        return path;
}

bool dir_exists(const char *path)
{
        if (path == NULL)
                return false;

        struct stat s;
        if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
                return true;
        return false;
}

bool file_exists(const char *path)
{
        if (path == NULL)
                return false;
        if (access(path, F_OK) != -1)
                return true;
        return false; 
}

char *read_file(const char *path)
{
        FILE *fp = fopen(path, "rb");
        if (!fp)
                return NULL;
       
        size_t cap = FCHUNKSZ * 2;
        size_t i = 0;
        size_t n = 0;
        char *buf = safe_malloc(cap);
        memset(buf, 0, cap);
        
        char tmp[FCHUNKSZ + 1];
        for (;;) {
                n = fread(tmp, 1, FCHUNKSZ, fp);
                tmp[n] = 0;
                if (i + n >= cap)
                        buf = safe_realloc(buf, cap += (FCHUNKSZ * 2));
                strcat(buf, tmp);
                i += n;

                if (n < FCHUNKSZ) {
                        if (i > 0)
                                buf[i - 1] = 0;
                        break;
                }
        }

        fclose(fp);

        return buf;
}
