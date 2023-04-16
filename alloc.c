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

#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *safe_malloc(size_t s)
{
        void *mem = malloc(s);
        assert(mem != NULL);
        return mem;
}

void *safe_calloc(int n, size_t s)
{
        void *mem = calloc(n, s);
        assert(mem != NULL);
        return mem;
}

void *safe_realloc(void *ptr, size_t s)
{
        void *mem = realloc(ptr, s);
        assert(mem != NULL);
        return mem;
}

char *safe_strdup(char *str)
{
        if (str == NULL)
                return NULL;

        size_t len = strlen(str);
        char *mem = safe_malloc(sizeof(char) * (len + 1));
        strcpy(mem, str);

        return mem;
}
