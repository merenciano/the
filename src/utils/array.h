#ifndef NYAS_ARRAY_H
#define NYAS_ARRAY_H

#include <string.h>

#if !defined(NYAS_ARRAY_MALLOC) || !defined(NYAS_ARRAY_REALLOC) || !defined(NYAS_ARRAY_FREE)
#include <stdlib.h>
#define NYAS_ARRAY_MALLOC malloc
#define NYAS_ARRAY_REALLOC realloc
#define NYAS_ARRAY_FREE free
#endif

#ifndef NYAS_ARRAY_ASSERT
#include <assert.h>
#define NYAS_ARRAY_ASSERT assert
#endif

#define GET_MACRO(_1, _2, NAME, ...) NAME

#define nyas__internal_arr_pushv(arr, count) \
	nyas__internal_arr_push((void **)&(arr), (count), sizeof(*(arr)))
#define nyas__internal_arr_push_(arr) nyas__internal_arr_pushv((arr), 1)
#define nyas__internal_arr_popv(arr, count) nyas__internal_arr_pop(arr, count)
#define nyas__internal_arr_pop_(arr) nyas__internal_arr_pop(arr, 1)

// Creation/Deletion
#define nyas_arr_create(T, capacity) nyas__internal_arr_create(sizeof(T), (capacity))
#define nyas_arr_release(arr) NYAS_ARRAY_FREE((struct nyas__internal_arr_header *)arr - 1)
#define nyas_arr_reserve(arr, new_capacity) \
	nyas__internal_arr_reserve((void **)&(arr), (new_capacity), sizeof(*(arr)))

// Modifiers
#define nyas_arr_push(...) \
	GET_MACRO(__VA_ARGS__, nyas__internal_arr_pushv, nyas__internal_arr_push_, UNUSED)(__VA_ARGS__)
#define nyas_arr_pop(...) \
	GET_MACRO(__VA_ARGS__, nyas__internal_arr_popv, nyas__internal_arr_pop_, UNUSED)(__VA_ARGS__)
#define nyas_arr_rm(arr, pos) nyas__internal_arr_rm((arr), &(arr[pos]), sizeof(*(arr)))

// Getters
#define nyas_arr_count(arr) (*((int *)(arr)-1))
#define nyas_arr_capacity(arr) (*((int *)(arr)-2))
#define nyas_arr_last(arr) ((arr) + (nyas_arr_count(arr) - 1))

struct nyas__internal_arr_header {
	int cap;
	int count;
	char buf[];
};

static inline void *
nyas__internal_arr_create(int elem_size, int capacity)
{
	struct nyas__internal_arr_header *h =
	  NYAS_ARRAY_MALLOC(sizeof(struct nyas__internal_arr_header) + capacity * elem_size);
	h->cap = capacity;
	h->count = 0;
	return h->buf;
}

// Doubles arr capacity until it is greater than 'min_cap'.
static inline void
nyas__internal_arr_reserve(void **arr, int min_cap, int elem_size)
{
	struct nyas__internal_arr_header *h = (struct nyas__internal_arr_header *)*arr - 1;
	if (min_cap > h->cap) {
		while ((h->cap *= 2) < min_cap) {}
		*arr = NYAS_ARRAY_REALLOC(h, elem_size * h->cap);
		NYAS_ARRAY_ASSERT(*arr);
		h = *arr;
		*arr = h + 1;
	}
}

static inline void *
nyas__internal_arr_push(void **arr, int count, int elem_size)
{
	nyas__internal_arr_reserve(arr, count + nyas_arr_count(*arr), elem_size);
	struct nyas__internal_arr_header *h = (struct nyas__internal_arr_header *)*arr - 1;
	h->count += count;
	return &h->buf[(h->count - count) * elem_size];
}

static inline void
nyas__internal_arr_pop(void *arr, int count)
{
	struct nyas__internal_arr_header *h = (struct nyas__internal_arr_header *)arr - 1;
	h->count = (h->count - count) * ((h->count - count) > 0);
}

static inline void
nyas__internal_arr_rm(void *arr, void *elem, int elem_size)
{
	struct nyas__internal_arr_header *h = (struct nyas__internal_arr_header *)arr - 1;
	memcpy(elem, &h->buf[(--h->count) * elem_size], elem_size);
}


#ifdef NYAS_ARRAY_TEST
#include <stdio.h>

int
main(void)
{
	float *arrf = nyas_arr_create(float, 5);
	NYAS_ARRAY_ASSERT(nyas_arr_capacity(arrf) == 5);
	NYAS_ARRAY_ASSERT(nyas_arr_count(arrf) == 0);
	float *new_value = nyas_arr_push(arrf);
	*new_value = 13.0f;
	printf("This should be 13.0 -> %.1f \n", *new_value);
	printf("Capacity: %d\n", nyas_arr_capacity(arrf));
	float *new_batch = nyas_arr_push(arrf, 45);
	printf("Push 45 elements...\n Capacity: %d\n", nyas_arr_capacity(arrf));
	for (int i = 0; i < 45; ++i) {
		new_batch[i] = (float)i + 100.0f;
	}
	NYAS_ARRAY_ASSERT(nyas_arr_count(arrf) == 46);
	NYAS_ARRAY_ASSERT(nyas_arr_capacity(arrf) >= 46);
	for (int i = 0; i < nyas_arr_count(arrf); ++i) {
		printf("%.1ff ", arrf[i]);
	}
	printf("\n");

	nyas_arr_rm(arrf, 3);
	printf("Removed 3th element\n");
	for (int i = 0; i < nyas_arr_count(arrf); ++i) {
		printf("%.1ff ", arrf[i]);
	}
	printf("\n");
	NYAS_ARRAY_ASSERT(nyas_arr_count(arrf) == 45);
	nyas_arr_pop(arrf, 15);
	printf("Pop last 15\n");
	for (int i = 0; i < nyas_arr_count(arrf); ++i) {
		printf("%.1ff ", arrf[i]);
	}
	printf("\n");
	NYAS_ARRAY_ASSERT(nyas_arr_count(arrf) == 30);
	NYAS_ARRAY_ASSERT(nyas_arr_capacity(arrf) >= 46);

	nyas_arr_pop(arrf);
	printf("Pop last\n");
	for (int i = 0; i < nyas_arr_count(arrf); ++i) {
		printf("%.1ff ", arrf[i]);
	}
	printf("\n");
	NYAS_ARRAY_ASSERT(nyas_arr_count(arrf) == 29);
	printf("This should be 29 -> %d \n", nyas_arr_count(arrf));

	int index;
	for (index = 0; (arrf[index] != 125.0f) && (index < nyas_arr_count(arrf)); ++index) {}
	printf("Find %.1ff = %d\n", 125.0f, index);
	printf("array[%d] = %.1ff\n", index, arrf[index]);
	NYAS_ARRAY_ASSERT(index == 26);
	NYAS_ARRAY_ASSERT(arrf[index] == 125.0f);

	nyas_arr_release(arrf);
	printf("Array released.\n");

	return 0;
}
#endif

#endif // NYAS_ARRAY_H
