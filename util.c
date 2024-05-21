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
#include <dirent.h>
#include <pwd.h>

bool dir_is_empty(char *path)
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

bool yesno_prompt(char *prompt, bool default_answer)
{
        char yc = 'y';
        char nc = 'N';
        if (default_answer) {
                yc = 'Y';
                nc = 'n';
        }

        char *buf = safe_malloc(sizeof(char) * 1024);
        for (;;) {
                printf("%s [%c/%c] ", prompt, yc, nc);
                fgets(buf, 1024, stdin);
                if (strcmp(buf, "y\n") == 0 || strcmp(buf, "yes\n") == 0
                                || strcmp(buf, "Y\n") == 0) {
                        free(buf);
                        return true;
                }

                if (strcmp(buf, "n\n") == 0 || strcmp(buf, "no\n") == 0
                                || strcmp(buf, "N\n") == 0) {
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

int snsystem(char *fmt, size_t size, ...)
{
        char *buf = safe_malloc(sizeof(char) * size);

        va_list ap;
        va_start(ap, size);

        vsnprintf(buf, size, fmt, ap);

        va_end(ap);

        int rc = system(buf);
        free(buf);

        return rc;
}

char *get_user_home(void)
{
        char *path = safe_malloc(sizeof(char) * PATH_MAX);
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL) {
                strcpy(path, "/");
                return path;
        }

        strcpy(path, pw->pw_dir);

        return path;
}

bool dir_exists(char *path)
{
        struct stat s;
        if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
                return true;
        return false;
}
