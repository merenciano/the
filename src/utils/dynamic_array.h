#ifndef NYAS_DYNAMIC_ARRAY_H
#define NYAS_DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

typedef struct arrhead_t {
	int cap;
	int count;
	char buf[];
} arrhead;

void *arr_push(void **arr, int count, int elem_size)
{
	arrhead *h = (*(arrhead**)arr) - 1;
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
	arrhead *h = (*(arrhead**)arr) - 1;
	h->count = MAX(h->count - count, 0);
}

void arr_rm(void *arr, void *elem, int elem_size)
{
	arrhead *h = (arrhead*)arr - 1;
	memmove(elem, &h->buf[h->count-- * elem_size], elem_size);
}

typedef int(*callback)(void *elem, void *ctx);

int arr_find(void *arr, int elem_size, callback fn, void *ctx)
{
	arrhead *h = (arrhead*)arr - 1;
	int i = 0;
	while ((i < h->count) && !(fn(&h->buf[i * elem_size], ctx))) {
		++i;
	}
	return i;
}

#define ARR_NPUSH(arr, count) arr_push(&(arr), (count), sizeof(*(arr)))
#define ARR_PUSH(arr) ARR_NPUSH((arr), 1)
#define ARR_NPOP(arr, count) arr_pop(arr, count, sizeof(*arr))
#define ARR_POP(arr) arr_pop(arr, 1, sizeof(*arr))
#define ARR_COUNT(arr) (*(((int*)(arr)) - 1))
#define ARR_RM(arr, pos) arr_rm((arr), &(arr)[pos], sizeof(*(arr)))
#define ARR_FIND(arr, value, eq_fn, ctx) \
	(arr_find((arr), sizeof(*(arr)), eq_fn, (ctx)))


#endif // NYAS_DYNAMIC_ARRAY_H
