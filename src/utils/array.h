#ifndef NYAS_ARRAY_H
#define NYAS_ARRAY_H

#include <string.h>

#if !defined(NYAS_ARRAY_MALLOC) || !defined(NYAS_ARRAY_FREE)
#include <stdlib.h>
#define NYAS_ARRAY_MALLOC malloc
#define NYAS_ARRAY_REALLOC realloc
#define NYAS_ARRAY_FREE free
#endif

#ifndef NYAS_ARRAY_ASSERT
#include <assert.h>
#define NYAS_ARRAY_ASSERT assert
#endif

struct internal_arr_head {
	int cap;
	int count;
	char buf[];
};

static inline void *
internal_arr_create(int elem_size, int capacity)
{
	struct internal_arr_head *h = NYAS_ARRAY_MALLOC(
	  sizeof(struct internal_arr_head) + capacity * elem_size);
	h->cap = capacity;
	h->count = 0;
	return h->buf;
}

static inline void *
internal_arr_push(void **arr, int count, int elem_size)
{
	struct internal_arr_head *h = (*(struct internal_arr_head **)arr) - 1;
	if ((h->count += count) > h->cap) {
		while ((h->cap *= 2) < h->count)
			;
		*arr = NYAS_ARRAY_REALLOC(h, elem_size * h->cap);
		NYAS_ARRAY_ASSERT(*arr);
		h = *arr;
		*arr = h + 1;
	}
	return &h->buf[(h->count - count) * elem_size];
}

static inline void
internal_arr_pop(void *arr, int count)
{
	struct internal_arr_head *h = (struct internal_arr_head *)arr - 1;
	h->count = (h->count - count) * ((h->count - count) > 0);
}

static inline void
internal_arr_rm(void *arr, void *elem, int elem_size)
{
	struct internal_arr_head *h = (struct internal_arr_head *)arr - 1;
	memcpy(elem, &h->buf[(--h->count) * elem_size], elem_size);
}

typedef int (*callback)(void *elem, void *ctx);

static inline int
internal_arr_find(void *arr, int elem_size, callback fn, void *ctx)
{
	struct internal_arr_head *h = (struct internal_arr_head *)arr - 1;
	int i = 0;
	while ((i < h->count) && !(fn(&h->buf[i * elem_size], ctx))) {
		++i;
	}
	return i;
}

#define nyas_arr_create(T, capacity) internal_arr_create(sizeof(T), (capacity))
#define nyas_arr_release(arr) NYAS_ARRAY_FREE((struct arr_head *)arr - 1)
#define nyas_arr_npush(arr, count) \
	internal_arr_push((void **)&(arr), (count), sizeof(*(arr)))
#define nyas_arr_push(arr) nyas_arr_npush((arr), 1)
#define nyas_arr_npop(arr, count) internal_arr_pop(arr, count)
#define nyas_arr_pop(arr) internal_arr_pop(arr, 1)
#define nyas_arr_rm(arr, pos) \
	internal_arr_rm((arr), &(arr[pos]), sizeof(*(arr)))
#define nyas_arr_find(arr, cmp_fn, ctx) \
	(internal_arr_find((arr), sizeof(*(arr)), cmp_fn, (ctx)))
#define nyas_arr_count(arr) (*((int *)(arr)-1))
#define nyas_arr_capacity(arr) (*((int *)(arr)-2))

#ifdef NYAS_ARRAY_TEST
#include <stdio.h>

static int
is_eq(void *elem, void *ctx)
{
	float *e = elem;
	float *other = ctx;
	return *e == *other;
}

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
	float *new_batch = nyas_arr_npush(arrf, 45);
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
	nyas_arr_npop(arrf, 15);
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

	float to_find = 125.0f;
	int index = nyas_arr_find(arrf, is_eq, &to_find);
	printf("Find %.1ff = %d\n", to_find, index);
	printf("array[%d] = %.1ff\n", index, arrf[index]);
	NYAS_ARRAY_ASSERT(index == 26);
	NYAS_ARRAY_ASSERT(arrf[index] == to_find);

	nyas_arr_release(arrf);
	printf("Array released.\n");

	return 0;
}
#endif

#endif // NYAS_ARRAY_H
