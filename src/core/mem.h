#ifndef THE_CORE_MEM_H
#define THE_CORE_MEM_H

#include <stddef.h>

#define THE_ALLOC the_alloc
#define THE_CALLOC the_calloc
#define THE_REALLOC the_realloc
#define THE_FREE the_free

#define THE_KB(X) ((X)*1024)
#define THE_MB(X) (THE_KB(X) * 1024)
#define THE_GB(X) (THE_MB((size_t)X) * 1024)

struct the_mem {
	ptrdiff_t cap;
	ptrdiff_t tail;
	char buff[];
};

/*
 * Sets the memory buffer that will be used by this module for it's allocations.
 * - General allocation functions (i.e. the_alloc, the_calloc, the_realloc and the_free)
 *     may call stdlib.h implementation.
 * - Every other allocator will use this buffer.
 */
int the_mem_init(void *buffer, ptrdiff_t size);

/*
 * Persistent alloc
 * - No realloc
 * - No free
 */
void *the_palloc(ptrdiff_t size);

/*
 * Circular alloc
 * - Volatile, allocated memory will become invalid when posterior allocations
 *     reach the size of the internal buffer.
 * - No realloc (circalloc + memcpy if needed).
 * - No explicit free.
 */
static inline void *the_circalloc(struct the_mem *mem, ptrdiff_t bytes)
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
void *the_alloc(size_t size);
void *the_calloc(size_t elem_count, size_t elem_size);
void *the_realloc(void *ptr, size_t size);
void the_free(void *ptr);

#endif // THE_CORE_MEM_H
