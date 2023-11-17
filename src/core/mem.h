#ifndef NYAS_CORE_MEM_H
#define NYAS_CORE_MEM_H

#include "nyas_defs.h"
#include <stdlib.h>

void *nyas_mem_reserve(size_t size);
void *nyas_alloc(size_t size);
void *nyas_calloc(size_t elem_count, size_t elem_size);
void *nyas_realloc(void *ptr, size_t size);
void nyas_free(void *ptr);

void nyas_mem_init(size_t size);
void nyas_mem_freeall(void);
float nyas_mem_mega_used(void);
size_t nyas_mem_bytes_used(void);
size_t nyas_mem_capacity(void);

#endif // NYAS_CORE_MEM_H
