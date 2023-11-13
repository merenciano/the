#include "array.h"

#include <string.h>

#if !defined(NYAS_ALLOC) && !defined(NYAS_FREE)
#include <stdlib.h>
#define NYAS_ALLOC malloc
#define NYAS_FREE free
#endif

#define BUFSZ 1024
#define ELEM(A, I) (&(hdr(A)->buf[I * hdr(A)->esz]))
#define ERR_IF(X) \
	if (X)        \
	return NULL

struct hdr {
	size_t esz;
	size_t len;
	size_t cap;
	unsigned char buf[];
};

static struct hdr *
hdr(struct hdr *arr)
{
	return arr - 1;
}

static size_t
new_cap(size_t min)
{
	min = min - 1;
	min = min | (min >> 1);
	min = min | (min >> 2);
	min = min | (min >> 4);
	min = min | (min >> 8);
	min = min | (min >> 16);
	return min + 1;
}

static void *
check_resize(void **arr, size_t needed_cap)
{
	if (hdr(*arr)->cap > needed_cap) {
		return *arr;
	}

	size_t sz = new_cap(needed_cap * hdr(*arr)->esz + sizeof(struct hdr));
	struct hdr *tmp = NYAS_ALLOC(sz);
	ERR_IF(!tmp);
	tmp->len = hdr(*arr)->len;
	tmp->esz = hdr(*arr)->esz;
	tmp->cap = (sz - sizeof(struct hdr)) / hdr(*arr)->esz;
	memcpy(tmp->buf, *arr, hdr(*arr)->len * hdr(*arr)->esz);
	NYAS_FREE(hdr(*arr));
	*arr = tmp->buf;
	return tmp->buf;
}

void *
nyas_arr_create(size_t n, size_t sz)
{
	ERR_IF(!sz);
	struct hdr *this = NYAS_ALLOC(sizeof *this + n * sz);
	ERR_IF(!this);
	this->esz = sz;
	this->len = 0;
	this->cap = n;
	return this->buf;
}

void
nyas_arr_destroy(void *arr)
{
	NYAS_FREE((struct hdr *)arr - 1);
}

void *
nyas_arr_npush(void **arr, size_t n)
{
	ERR_IF(!arr);
	ERR_IF(!*arr);
	if (!check_resize(arr, hdr(*arr)->len + n)) {
		return NULL;
	}

	void *first_elem = ELEM(*arr, hdr(*arr)->len);
	hdr(*arr)->len += n;
	return first_elem;
}

void *
nyas_arr_npop(void *arr, size_t n)
{
	ERR_IF(!arr);
	ERR_IF(hdr(arr)->len == 0);
	hdr(arr)->len = hdr(arr)->len > n ? hdr(arr)->len - n : 0;
	return ELEM(arr, hdr(arr)->len);
}

void *
nyas_arr_at(void *arr, size_t idx)
{
	ERR_IF(!arr);
	ERR_IF(idx >= hdr(arr)->len);
	return ELEM(arr, idx);
}

size_t
nyas_arr_len(void *arr)
{
	return hdr(arr)->len;
}

ptrdiff_t
nyas_arr_cmp(void *a, void *b)
{
	if ((a == b) || !a || !b) {
		return a - b;
	}

	if (hdr(a)->len != hdr(b)->len) {
		return (ptrdiff_t)(hdr(a)->len - hdr(b)->len);
	}

	if (hdr(a)->esz != hdr(b)->esz) {
		return (ptrdiff_t)(hdr(a)->esz - hdr(b)->esz);
	}

	return !memcmp(a, b, hdr(a)->len * hdr(a)->esz);
}

void *
nyas_arr_swap(void *arr, size_t a, size_t b)
{
	ERR_IF(!arr);
	struct hdr *this = hdr(arr);
	ERR_IF(a >= this->len || b >= this->len);
	if (a == b) {
		return ELEM(arr, a);
	}

	char buffer[BUFSZ];
	void *tmp = BUFSZ < this->esz ? NYAS_ALLOC(this->esz) : buffer;
	ERR_IF(!tmp);
	void *pa = ELEM(arr, a);
	void *pb = ELEM(arr, b);
	memcpy(tmp, pa, this->esz);
	memcpy(pa, pb, this->esz);
	memcpy(pb, tmp, this->esz);
	if (BUFSZ < this->esz) {
		NYAS_FREE(tmp);
	}
	return pa;
}

void *
nyas_arr_rm(void *arr, size_t idx)
{
	ERR_IF(!arr);
	return memcpy(ELEM(arr, idx), nyas_arr_npop(arr, 1), hdr(arr)->esz);
}

void *
nyas_arr_extract(void *arr, size_t idx)
{
	ERR_IF(!arr);
	ERR_IF(hdr(arr)->len <= idx);
	nyas_arr_swap(arr, idx, hdr(arr)->len - 1);
	return nyas_arr_npop(arr, 1);
}

void *
nyas_arr_cpyfta(void **dst, void *src, size_t from, size_t to, size_t at)
{
	ERR_IF(!dst);
	ERR_IF(!*dst);
	ERR_IF(!src);
	ERR_IF(hdr(src)->len == 0);
	to = to ? to : hdr(src)->len;
	at = (at == (size_t)(-1)) ? hdr(*dst)->len : at;
	ERR_IF(hdr(*dst)->len < at);
	ERR_IF(to <= from);
	ERR_IF(hdr(*dst)->esz != hdr(src)->esz);

	size_t new_len = at + to - from;
	ERR_IF(!check_resize(dst, new_len));
	hdr(*dst)->len = new_len;
	size_t bytes_to_cpy = (to - from) * hdr(src)->esz;
	return memcpy(ELEM(*dst, at), ELEM(src, from), bytes_to_cpy);
}

void
nyas_arr_reset(void *arr, size_t elem_size)
{
	if (arr) {
		hdr(arr)->len = 0;
		if (elem_size) {
			hdr(arr)->cap = (hdr(arr)->cap * hdr(arr)->esz) / elem_size;
			hdr(arr)->esz = elem_size;
		}
	}
}
