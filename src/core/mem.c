#include "mem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) (ADDRESS & (MEM_ALIGN - 1))

static uint8_t *mem;
static size_t offset;
static size_t capacity;

void *THE_PersistentAlloc(size_t size)
{
	offset += MEM_ALIGN_MOD(MEM_ALIGN - MEM_ALIGN_MOD(offset));
	offset += size;
	THE_ASSERT(offset <= capacity, "Allocator out of memory");
	return mem + offset - size; 
}

void *THE_Alloc(size_t size)
{
	return malloc(size);
}

void *THE_Calloc(size_t elem_count, size_t elem_size)
{
	return calloc(elem_count, elem_size);
}

void *THE_Realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void THE_Free(void *ptr)
{
	free(ptr);
}

void THE_MemInit(size_t size)
{
	mem = malloc(size);
	THE_ASSERT(!((size_t)mem % MEM_ALIGN), "WTF");
	if (!mem) {
		THE_SLOG_ERROR("Couldn't allocate THE memory.");
		exit(1);
	}

	offset = 0;
	capacity = size;
}

void THE_MemFreeAll()
{
	free(mem);
	mem = NULL;
}

float THE_MemUsedMB()
{
	return THE_MemUsedBytes() / (1024.0f * 1024.0f);
}

size_t THE_MemUsedBytes()
{
	return offset;
}

size_t THE_MemCapacity()
{
	return capacity;
}
