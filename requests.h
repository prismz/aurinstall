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

#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdlib.h>

struct curl_str {
        char *ptr;
        size_t len;
};

void init_string(struct curl_str *s);
size_t write_func(void *ptr, size_t size, size_t nmemb, struct curl_str *s);
char *requests_get(char *url);

#endif  /* REQUESTS_H */
