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

#include "mem.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

void*
smalloc(size_t size)
{
    void* mem = malloc(size);
    if (mem == NULL)
        die(stderr, "error: out of memory.", EXIT_FAILURE);

    return mem;
}

void*
srealloc(void* ptr, size_t size)
{
    void* mem = realloc(ptr, size);
    if (mem == NULL)
        die(stderr, "error: could not resize memory block.", EXIT_FAILURE);

    return mem;
}

/* 
 * old debug function, kept in for consistency 
 * calls free() on ptr.
 */
void
sfree(void* ptr)
{
    if (ptr != NULL)
        free(ptr);
}