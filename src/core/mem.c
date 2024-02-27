#include "mem.h"

#include "io.h"
#include <stdlib.h>

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) ((ADDRESS) & (MEM_ALIGN - 1))

static struct nyas_mem *persistent = NULL;

void *
nyas_palloc(ptrdiff_t size)
{
	void *ptr = &persistent->buff[persistent->tail];
	persistent->tail += size;
	return (persistent->tail + size) <= persistent->cap ? persistent->tail += size, ptr : NULL;
}

int
nyas_mem_init(void *buffer, ptrdiff_t size)
{
	if (!buffer) {
		NYAS_LOG_ERR("Invalid buffer");
		return NYAS_ERR_INVALID_PTR;
	}
	persistent = buffer;
	persistent->cap = size - sizeof(*persistent);
	persistent->tail = 0;
	return NYAS_OK;
}

void *
nyas_alloc(size_t size)
{
	return malloc(size);
}

void *
nyas_calloc(size_t elem_count, size_t elem_size)
{
	return calloc(elem_count, elem_size);
}

void *
nyas_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void
nyas_free(void *ptr)
{
	free(ptr);
}
