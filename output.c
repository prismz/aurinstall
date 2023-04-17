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

#include "output.h"
#include "util.h"
#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>

void indent_print(char *str, int indent)
{
        if (!isatty(STDOUT_FILENO)) {
                for (int i = 0; i < indent; i++)
                        printf(" ");
                printf("%s\n", str);
                
                return;
        }

        size_t words_capacity = 128;
        size_t words_idx = 0;
        char **words = safe_malloc(sizeof(char *) * words_capacity);
        
        size_t current_word_capacity = 256;
        size_t current_word_idx = 0;
        char *current_word = safe_malloc(sizeof(char) * current_word_capacity);

        size_t len = strlen(str);

        for (size_t i = 0; i < len; i++) {
                char c = str[i];
                if (isspace(c))
                        goto space;

                if (current_word_idx + 2 > current_word_capacity) {
                        current_word_capacity += 32;
                        current_word = realloc(current_word, 
                                        sizeof(char) * current_word_capacity);
                }

                current_word[current_word_idx++] = c;
                current_word[current_word_idx] = '\0';

space:
                if ((isspace(c) || (i == len - 1)) && current_word_idx != 0) {
                        if (words_idx + 1 > words_capacity) {
                                words_capacity += 32;
                                words = safe_realloc(words, 
                                               sizeof(char *) * words_capacity);
                        }

                        words[words_idx++] = safe_strdup(current_word);
                        current_word_capacity = 256;
                        current_word_idx = 0;
                        current_word = safe_realloc(current_word, 
                                        sizeof(char) * current_word_capacity);
                        bzero(current_word, current_word_capacity);

                        continue;
                }
        }

        /* find terminal size to wrap words */
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        int termwidth = w.ws_col - indent;
        size_t printed = indent;

        for (int i = 0; i < indent; i++)
                printf(" ");

        for (size_t i = 0; i < words_idx; i++) {
                char *word = words[i];

                if ((int)(printed + strlen(word) + 1) > termwidth) {
                        printf("\n");
                        for (int j = 0; j < indent; j++)
                                printf(" ");
                        printed = indent;
                }

                printf("%s ", word);
                printed += strlen(word);
                printed++;  /* the space */

        }

        printf("\n");

        for (size_t i = 0; i < words_idx; i++)
                free(words[i]);
        free(words);
        free(current_word);
}

bool stdout_is_tty(void)
{
        return isatty(STDOUT_FILENO);
}

void print_diff(char *a, char *b)
{
        size_t l1 = strlen(a);
        size_t l2 = strlen(b);

        size_t longest = (l1 > l2) ? l1 : l2;
        char *longest_str = (l1 > l2) ? a : b;

        printf("%s%s%s -> ", GREEN, a, ENDC);
        printf("%s", GREEN);

        bool switched = false;
        
        for (size_t i = 0; i < longest; i++) {
                if (a[i] != b[i] && !switched) {
                        switched = true;
                        printf("%s", ENDC);
                        printf("%s", RED);
                }
                printf("%c", longest_str[i]);
        }
        
        printf("%s", ENDC);
}
