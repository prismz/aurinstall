#include "alloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkmem(void *ptr)
{
        if (ptr == NULL) {
                fprintf(stderr, "fatal error: failed to allocate memory.\n");
                exit(1);
        }
}

void *safe_malloc(size_t s)
{
        void *mem = malloc(s);
        checkmem(mem);
        return mem;
}

void *safe_calloc(int n, size_t s)
{
        void *mem = calloc(n, s);
        checkmem(mem);
        return mem;
}

void *safe_realloc(void *ptr, size_t s)
{
        void *mem = realloc(ptr, s);
        checkmem(mem);
        return mem;
}

char *safe_strdup(const char *str)
{
        if (str == NULL)
                return NULL;

        size_t len = strlen(str);
        char *mem = safe_malloc(sizeof(char) * (len + 1));
        strcpy(mem, str);

        return mem;
}
