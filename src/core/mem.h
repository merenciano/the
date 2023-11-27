#ifndef NYAS_CORE_MEM_H
#define NYAS_CORE_MEM_H

#include <stddef.h>

#define NYAS_ALLOC nyas_alloc
#define NYAS_CALLOC nyas_calloc
#define NYAS_REALLOC nyas_realloc
#define NYAS_FREE nyas_free

#define NYAS_KB(X) ((X)*1024)
#define NYAS_MB(X) (NYAS_KB(X) * 1024)
#define NYAS_GB(X) (NYAS_MB((size_t)X) * 1024)

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
