#ifndef NYAS_DYNAMIC_ARRAY_H
#define NYAS_DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

struct arr_head {
	int cap;
	int count;
	char buf[];
};

static inline struct arr_head *internal_head(void *arr)
{
	return (struct arr_head*)arr - 1;
}

static void *
arr_create(int elem_size, int capacity)
{
	struct arr_head *h = malloc(sizeof(struct arr_head) + capacity * elem_size);
	h->cap = capacity;
	h->count = 0;
	return h->buf;
}

void
arr_release(void *arr)
{
	free(internal_head(arr));
}

void *arr_push(void **arr, int count, int elem_size)
{
	struct arr_head *h = (*(struct arr_head**)arr) - 1;
	if ((h->count + count) > h->cap) {
		 *arr = realloc(h, elem_size * h->cap * 2);
		 assert(*arr);
		 h = *arr;
		 *arr = h + 1;
		 h->cap *= 2;
	}
	h->count += count;
	return &h->buf[(h->count - count) * elem_size];
}

void arr_pop(void *arr, int count)
{
	struct arr_head *h = (*(struct arr_head**)arr) - 1;
	h->count = MAX(h->count - count, 0);
}

void arr_rm(void *arr, void *elem, int elem_size)
{
	struct arr_head *h = (struct arr_head*)arr - 1;
	memmove(elem, &h->buf[h->count-- * elem_size], elem_size);
}

typedef int(*callback)(void *elem, void *ctx);

int arr_find(void *arr, int elem_size, callback fn, void *ctx)
{
	struct arr_head *h = (struct arr_head*)arr - 1;
	int i = 0;
	while ((i < h->count) && !(fn(&h->buf[i * elem_size], ctx))) {
		++i;
	}
	return i;
}

static inline int arr_count(void *arr) { return *((int*)arr - 1); }
static inline int arr_capacity(void *arr) { return *((int*)arr - 2); }

#define ARR_CREATE(T, capacity) arr_create(sizeof(T), (capacity))
#define ARR_RELEASE(arr) arr_release(arr)
#define ARR_NPUSH(arr, count) arr_push(&(arr), (count), sizeof(*(arr)))
#define ARR_PUSH(arr) ARR_NPUSH((arr), 1)
#define ARR_NPOP(arr, count) arr_pop(arr, count, sizeof(*arr))
#define ARR_POP(arr) arr_pop(arr, 1, sizeof(*arr))
#define ARR_COUNT(arr) (*(((int*)(arr)) - 1))
#define ARR_CAPACITY(arr) (*(((int*)(arr)) - 1))
#define ARR_RM(arr, pos) arr_rm((arr), &(arr)[pos], sizeof(*(arr)))
#define ARR_FIND(arr, value, eq_fn, ctx) \
	(arr_find((arr), sizeof(*(arr)), eq_fn, (ctx)))


#endif // NYAS_DYNAMIC_ARRAY_H
