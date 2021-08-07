#include "mem.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>


void*
smalloc(size_t size, char* desc)
{
    void* mem = malloc(size);
    FILE* fp = fopen("log.txt", "ab+");
    if (mem == NULL)
        die(stderr, "error: out of memory.", EXIT_FAILURE);
    if (!fp)
        return mem;
    fprintf(fp, "malloc()   bytes at %p: %s\n", mem, desc);
    fclose(fp);

    
    return mem;
}

void*
srealloc(void* ptr, size_t size)
{
    void* mem = realloc(ptr, size);
    FILE* fp = fopen("log.txt", "ab+");
    if (mem == NULL)
        die(stderr, "error: could not resize memory block.", EXIT_FAILURE);
    if (!fp)
        return mem;
    fprintf(fp, "realloc()  bytes at %p -> %p\n", ptr, mem);
        
    return mem;
}

void
sfree(void* ptr)
{
    FILE* fp = fopen("log.txt", "ab+");
    if (!fp) {
        free(ptr);
        return;
    }
    fprintf(fp, "free()     bytes at %p\n", ptr);
    fclose(fp);
    free(ptr);
}