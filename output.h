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

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdbool.h>

#define BOLD "\033[1m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define BLUE "\033[94m"
#define ENDC "\033[0m"

void indent_print(char *str, int indent);
bool stdout_is_tty(void);
void print_diff(char *a, char *b);

#endif
