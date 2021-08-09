#ifndef MEM_H
#define MEM_H

#include <stdio.h>

void* smalloc(size_t size, char* desc);
void* srealloc(void* ptr, size_t size);
void sfree(void* ptr);

#endif