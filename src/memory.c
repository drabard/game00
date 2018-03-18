#include "memory.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void* mem_alloc(size_t size)
{
    return malloc(size);
}

void mem_free(void* ptr)
{
    return free(ptr);
}

void* mem_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void mem_memcpy(void* dst, const void* src, size_t size)
{
    memcpy(dst, src, size);
}
