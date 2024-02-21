#include "mem.h"

#include "io.h"
#include <stdlib.h>

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) (ADDRESS & (MEM_ALIGN - 1))

static unsigned char *mem;
static size_t offset;
static size_t capacity;

void *
nyas_mem_reserve(size_t size)
{
	offset += MEM_ALIGN_MOD(MEM_ALIGN - MEM_ALIGN_MOD(offset));
	offset += size;
	NYAS_ASSERT(offset <= capacity && "Allocator out of memory");
	return mem + offset - size; 
}

void *nyas_alloc(size_t size)
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

void
nyas_mem_init(size_t size)
{
	mem = malloc(size); // TODO: mem minimal instance in stack memory in order to alloc this chunk for itself
	NYAS_ASSERT(!((size_t)mem % MEM_ALIGN) && "WTF");
	if (!mem) {
		NYAS_LOG_ERR("Error allocating initial mem chunk.");
		exit(1);
	}

	offset = 0;
	capacity = size;
}

void
nyas_mem_freeall(void)
{
	free(mem);
	mem = NULL;
}

float
nyas_mem_mega_used(void)
{
	return nyas_mem_bytes_used() / (1024.0f * 1024.0f);
}

size_t nyas_mem_bytes_used(void)
{
	return offset;
}

size_t
nyas_mem_capacity(void)
{
	return capacity;
}
