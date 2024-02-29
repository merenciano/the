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

struct nyas_mem {
	ptrdiff_t cap;
	ptrdiff_t tail;
	char buff[];
};

/*
 * Sets the memory buffer that will be used by this module for it's allocations.
 * - General allocation functions (i.e. nyas_alloc, nyas_calloc, nyas_realloc and nyas_free)
 *     may call stdlib.h implementation.
 * - Every other allocator will use this buffer.
 */
int nyas_mem_init(void *buffer, ptrdiff_t size);

/*
 * Persistent alloc
 * - No realloc
 * - No free
 */
void *nyas_palloc(ptrdiff_t size);

/*
 * Circular alloc
 * - Volatile, allocated memory will become invalid when posterior allocations
 *     reach the size of the internal buffer.
 * - No realloc (circalloc + memcpy if needed).
 * - No explicit free.
 */
static inline void *nyas_circalloc(struct nyas_mem *mem, ptrdiff_t bytes)
{
	if (mem->tail + bytes > mem->cap) {
		mem->tail = bytes;
		return mem->buff;
	}
	void *ret = mem->buff + mem->tail;
	mem->tail += bytes;
	return ret;
}

/*
 * General allocation functions.
 * - Same behaviour as stdlib.h ones regardless of intenral implementation.
 * - alloc, calloc, realloc and free.
 */
void *nyas_alloc(size_t size);
void *nyas_calloc(size_t elem_count, size_t elem_size);
void *nyas_realloc(void *ptr, size_t size);
void nyas_free(void *ptr);

#endif // NYAS_CORE_MEM_H
