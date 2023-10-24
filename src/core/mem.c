#include "mem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t *mem;
static uint8_t *offset;
static size_t capacity;

void *THE_PersistentAlloc(size_t size, size_t align)
{
	THE_ASSERT((size_t)(offset + size - mem) <= capacity, "Out of memory");
	if ((size_t)(offset + size - mem) > capacity) {
		THE_SLOG_ERROR("Persistent allocation failed, Out of memory!");
		return NULL;
	}

	void *ret_mem = (void*)offset;
	offset += size;
	if (align != 0 && (size_t)offset % align) //TODO: think about a fancier align method
	{
		offset += (align - ((size_t)offset % align));
	}
	return ret_mem; 
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
	mem = (uint8_t*)malloc(size);
	if (!mem) {
		THE_SLOG_ERROR("Couldn't allocate THE memory.");
		exit(1);
	}

	offset = mem;
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
	return offset - mem;
}

size_t THE_MemCapacity()
{
	return capacity;
}
