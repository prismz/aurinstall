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

#ifndef UTIL_H
#define UTIL_H

#define BOLD "\033[1m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define BLUE "\033[94m"
#define ENDC "\033[0m"

#include <stdio.h>

void die(FILE* buff, char* str, int ret);
void pretty_print(char* str_, int indent);
int dir_is_empty(char* path);
char* get_homedir(void);
void right_pad_print_str(char* str, int max_len, int extra);
int snsystem(size_t size, char* format, ...);

#endif