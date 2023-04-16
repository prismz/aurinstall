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

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdbool.h>

bool dir_is_empty(char *dir);
bool yesno_prompt(char *prompt, bool default_answer);
int snsystem(char *fmt, size_t size, ...);
char *get_user_home(void);
bool dir_exists(char *path);

#endif  /* UTIL_H */
