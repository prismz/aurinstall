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

#define FCHUNKSZ 512 

#include <stdio.h>
#include <stdbool.h>

/* this could be just one function but you can't really
 * conveniently pass infinite args between the two */
void err(const char *fmt, ...);
void fatal_err(const char *fmt, ...);
bool dir_is_empty(const char *dir);
bool yesno_prompt(const char *prompt, bool default_answer);
char *get_user_home(void);
bool dir_exists(const char *path);
bool file_exists(const char *path);
char *read_file(const char *path);

#endif  /* UTIL_H */
