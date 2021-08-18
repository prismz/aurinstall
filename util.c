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
 * Copyright (C) 2021 Hasan Zahra
 * https://github.com/prismz/aurinstall
 */

#include "util.h"
#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>

void
die(FILE* buff, char* str, int ret)
{
    fprintf(buff, "%s\n", str);
    exit(ret);
}

void
pretty_print(char* str_, int indent)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int termwidth = w.ws_col - indent;

    char* str = smalloc(sizeof(char) * (strlen(str_) + 1));
    strcpy(str, str_);

    char* words[strlen(str) + 1];
    char* ptr = strtok(str, " ");

    int wordc;
    for (wordc = 0; ptr != NULL; wordc++) {
        words[wordc] = smalloc(sizeof(char) * (strlen(ptr) + 1));
        strcpy(words[wordc], ptr);
        words[wordc + 1] = NULL;
        ptr = strtok(NULL, " ");
    }

    for (int i = 0; i < indent; i++)
        printf(" ");

    int j = 0;
    for (int i = 0; i < wordc; i++) {
        char* word = words[i];
        printf("%s ", word);
        j += strlen(word) + 1;
        sfree(words[i]);

        if (i == wordc - 1)
            break;

        if (j + (strlen(words[i + 1]) + 1) >= (size_t)termwidth) {
            printf("\n");
            for (int k = 0; k < indent; k++)
                printf(" ");
            j = 0;
        }
    }
    sfree(str);

    printf("\n");
}

int
dir_is_empty(char* path)
{
    int n = 0;
    struct dirent* d;
    DIR* dir = opendir(path);
    if (dir == NULL)
        return 1;
    while ((d = readdir(dir)) != NULL) {
        if (n++ > 2)
            break;
    }
    closedir(dir);

    if (n <= 2)
        return 1;
    return 0;
}

char*
get_homedir(void)
{
    char* path = smalloc(sizeof(char) * 256);
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw == NULL) {
        strcpy(path, "/");
        return path;
    }

    strcpy(path, pw->pw_dir);
    return path;
}

void
right_pad_print_str(char* str, int max_len, int extra)
{
    printf("%s", str);
    for (int i = max_len - strlen(str) + extra; i >= 0; i--)
        printf(" ");
}

int
snsystem(size_t size, char* format, ...)
{
    va_list argl;
    va_start(argl, format);

    char* command = smalloc(sizeof(char) * size);

    vsnprintf(command, size, format, argl);

    va_end(argl);

    int ret = system(command);
    sfree(command);

    return ret;
}