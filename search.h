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

#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : \
                ((b) < (c) ? (b) : (c)))

int levenshtein(char *s1, char *s2);
int package_qsort_levenshtein(const void *one, const void *two);
void print_search_result(bool istty, char *name, 
                char *desc, char *ver, int ood, bool installed);
int search_aur(int n, char **terms);

#endif  /* SEARCH_H */
