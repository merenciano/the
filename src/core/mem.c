#include "mem.h"

#include "io.h"
#include <stdlib.h>

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) ((ADDRESS) & (MEM_ALIGN - 1))

static struct the_mem *persistent = NULL;

void *
the_palloc(ptrdiff_t size)
{
	void *ptr = &persistent->buff[persistent->tail];
	persistent->tail += size;
	return (persistent->tail + size) <= persistent->cap ? persistent->tail += size, ptr : NULL;
}

int
the_mem_init(void *buffer, ptrdiff_t size)
{
	if (!buffer) {
		THE_LOG_ERR("Invalid buffer");
		return THE_ERR_INVALID_PTR;
	}
	persistent = buffer;
	persistent->cap = size - sizeof(*persistent);
	persistent->tail = 0;
	return THE_OK;
}

void *
the_alloc(size_t size)
{
	return malloc(size);
}

void *
the_calloc(size_t elem_count, size_t elem_size)
{
	return calloc(elem_count, elem_size);
}

void *
the_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void
the_free(void *ptr)
{
	free(ptr);
}
