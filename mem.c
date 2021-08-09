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
smalloc(size_t size, char* desc)
{
    void* mem = malloc(size);
    if (mem == NULL)
        die(stderr, "error: out of memory.", EXIT_FAILURE);

    /* 
     * FILE* fp = fopen("log.txt", "ab+");
     * if (!fp)
     *    return mem;
     * fprintf(fp, "malloc()   bytes at %p: %s\n", mem, desc);
     * fclose(fp);
     */

    return mem;
}

void*
srealloc(void* ptr, size_t size)
{
    void* mem = realloc(ptr, size);
    if (mem == NULL)
        die(stderr, "error: could not resize memory block.", EXIT_FAILURE);
   
    /*
     * FILE* fp = fopen("log.txt", "ab+");
     * if (!fp)
     *     return mem;
     * fprintf(fp, "realloc()  bytes at %p -> %p\n", ptr, mem);
     */

    return mem;
}

void
sfree(void* ptr)
{
    /* 
     * FILE* fp = fopen("log.txt", "ab+");
     * if (!fp) {
     *     free(ptr);
     *     return;
     * }
     * fprintf(fp, "free()     bytes at %p\n", ptr);
     * fclose(fp);
     */
    
    free(ptr);
}
