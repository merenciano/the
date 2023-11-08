#ifndef NYAS_TYPES_H
#define NYAS_TYPES_H

#include <stddef.h>

#define NYAS_FXD_STR(SZ)                                                      \
	struct {                                                                  \
		unsigned char str[SZ - 1];                                            \
		unsigned char rem;                                                    \
	}

typedef NYAS_FXD_STR(64) nyas_str_64;

/**
 * @brief Header struct for ny_arr (arrays).
 *
 * @var esz Size in bytes of each element.
 * @var len Length (element count).
 * @var cap Capacity of elements without expanding (current storage).
 */
struct nyas_arr_hdr {
	size_t esz;
	size_t len;
	size_t cap;
	unsigned char buf[];
};
typedef void *nt_arr;
#define nyas_arr_elem_size(ARR) ((((struct nyas_arr_hdr *)ARR) - 1)->esz)
#define nyas_arr_len(ARR) ((((struct nyas_arr_hdr *)ARR) - 1)->len)
#define nyas_arr_cap(ARR) ((((struct nyas_arr_hdr *)ARR) - 1)->cap)
#define nyas_arr_push(REF_ARR) nyas_arr_pushv(REF_ARR, 1)
#define nyas_arr_pop(ARR) nyas_arr_popv(ARR, 1)
#define nyas_arr_clear(ARR) (nyas_arr_len(ARR) = 0)
#define nyas_arr_cpy(REF_ARR, SRC)                                            \
	nyas_arr_cpyft(REF_ARR, SRC, 0, nyas_arr_len(SRC))
nt_arr nyas_arr_create(size_t n, size_t sz);
void nyas_arr_destroy(nt_arr arr);
void *nyas_arr_pushv(nt_arr *arr, size_t n);
void *nyas_arr_popv(nt_arr arr, size_t n);
void *nyas_arr_at(nt_arr arr, size_t idx);
bool nyas_arr_eq(nt_arr a, nt_arr b);
void *nyas_arr_swap(nt_arr arr, size_t a, size_t b);
void *nyas_arr_rm(nt_arr arr, size_t idx);
void *nyas_arr_extract(nt_arr arr, size_t idx);
void *nyas_arr_cpyft(nt_arr *arr, nt_arr other, size_t from, size_t to);
void *nyas_arr_reset(nt_arr arr, size_t elem_size);

#ifdef NYAS_TYPES_IMPL

#include <string.h> // memcpy, memcmp

#ifndef NYAS_TYPES_ALLOC
#include <stdlib.h>
#define NYAS_TYPES_ALLOC malloc
#define NYAS_TYPES_FREE free
#endif

#ifndef NYAS_TYPES_ASSERT
#include <assert.h>
#define NYAS_TYPES_ASSERT assert
#endif

#ifndef NYAS_TYPES_LOG
#include <stdio.h>
#define NYAS_TYPES_LOG printf
#endif

#ifndef NYAS_TYPES_TMP_BUFSIZE
#define NYAS_TYPES_TMP_BUFSIZE 1024
#endif

#define nyas__hdr(ARR) (((struct nyas_arr_hdr *)ARR) - 1)
#define nyas__elem_ptr(POS) (&(this->buf[POS * this->esz]))
#define nyas__check_null(X)                                                   \
	if ((!X))                                                                 \
	return (void *)(X)

#define nyas__err_ret_if(X)                                                   \
	if ((X))                                                                  \
	return NULL

static inline size_t
nyas__new_cap(size_t min)
{
	min = min - 1;
	min = min | (min >> 1);
	min = min | (min >> 2);
	min = min | (min >> 4);
	min = min | (min >> 8);
	min = min | (min >> 16);
	return min + 1;
}

nt_arr
nyas_arr_create(size_t n, size_t sz)
{
	nyas__check_null(sz);
	struct nyas_arr_hdr *this = NYAS_TYPES_ALLOC(sizeof *this + n * sz);
	nyas__check_null(this);
	this->esz = sz;
	this->len = 0;
	this->cap = n;
	return this->buf;
}

void
nyas_arr_destroy(nt_arr arr)
{
	NYAS_TYPES_FREE((struct nyas_arr_hdr *)arr - 1);
}

void *
nyas_arr_pushv(nt_arr *arr, size_t n)
{
	nyas__check_null(arr);
	nyas__check_null(*arr);
	struct nyas_arr_hdr *this = nyas__hdr(*arr);
	if (this->len + n > this->cap) {
		return NULL;
	}
	void *first_elem = nyas__elem_ptr(this->len);
	this->len += n;
	return first_elem;
}

void *
nyas_arr_popv(nt_arr arr, size_t n)
{
	nyas__check_null(arr);
	struct nyas_arr_hdr *this = nyas__hdr(arr);
	if (this->len - n < 0) {
		return NULL;
	}
	this->len -= n;
	return nyas__elem_ptr(this->len);
}

void *
nyas_arr_at(nt_arr arr, size_t idx)
{
	nyas__check_null(arr);
	nyas__err_ret_if(idx >= nyas_arr_len(arr));
	return ((unsigned char *)arr) + idx * nyas_arr_elem_size(arr);
}

bool
nyas_arr_eq(nt_arr a, nt_arr b)
{
	nyas__check_null(a);
	nyas__check_null(b);

	if (nyas_arr_len(a) != nyas_arr_len(b)) {
		return false;
	}

	if (nyas_arr_elem_size(a) != nyas_arr_elem_size(b)) {
		return false;
	}

	return !memcmp(a, b, nyas_arr_len(a) * nyas_arr_elem_size(a));
}

void *
nyas_arr_swap(nt_arr arr, size_t a, size_t b)
{
	nyas__check_null(arr);
	nyas__err_ret_if(a >= nyas_arr_len(arr) || b >= nyas_arr_len(arr));
	struct nyas_arr_hdr *this = nyas__hdr(arr);

	unsigned char buffer[NYAS_TYPES_TMP_BUFSIZE];
	void *tmp = NYAS_TYPES_TMP_BUFSIZE < this->esz ?
	  NYAS_TYPES_ALLOC(this->esz) :
	  buffer;
	nyas__check_null(tmp);
	void *pa = nyas__elem_ptr(a);
	void *pb = nyas__elem_ptr(b);
	memcpy(tmp, pa, this->esz);
	memcpy(pa, pb, this->esz);
	memcpy(pb, tmp, this->esz);
	if (NYAS_TYPES_TMP_BUFSIZE < this->esz) {
		NYAS_TYPES_FREE(tmp);
	}
	return pa;
}

void *
nyas_arr_rm(nt_arr arr, size_t idx)
{
	nyas__check_null(arr);
	struct nyas_arr_hdr *this = nyas__hdr(arr);
	return memcpy(nyas__elem_ptr(idx), nyas_arr_pop(arr), this->esz);
}

void *
nyas_arr_extract(nt_arr arr, size_t idx)
{
	nyas_arr_swap(arr, idx, nyas_arr_len(arr) - 1);
	return nyas_arr_pop(arr);
}

void *
nyas_arr_cpyft(nt_arr *arr, nt_arr other, size_t from, size_t to)
{
	nyas__check_null(arr);
	nyas__check_null(*arr);
	nyas__check_null(other);
	nyas__err_ret_if(to <= from);
	nyas__err_ret_if(nyas_arr_elem_size(*arr) != nyas_arr_elem_size(other));

	size_t elem_to_cpy = to - from;
	size_t elem_size = nyas_arr_elem_size(other);
	if (nyas_arr_cap(*arr) < elem_to_cpy) {
		size_t new_size = nyas__new_cap(elem_to_cpy * elem_size);
		nt_arr tmp = nyas_arr_create(new_size / elem_size, elem_size);
		nyas__check_null(tmp);
		NYAS_TYPES_FREE(*arr);
		*arr = tmp;
	}
	struct nyas_arr_hdr *this = nyas__hdr(*arr);
	this->len = elem_to_cpy;
	return memcpy(*arr, nyas_arr_at(other, from), this->len * this->esz);
}

void *
nyas_arr_reset(nt_arr arr, size_t elem_size)
{
	nyas__check_null(arr);
	nyas__check_null(elem_size);
	nyas_arr_cap(arr) = (nyas_arr_cap(arr) * nyas_arr_elem_size(arr)) /
	  elem_size;
	nyas_arr_clear(arr);
	nyas_arr_elem_size(arr) = elem_size;
	return arr;
}

#ifdef NYAS_TYPES_TEST

void
nyas_arr_test(void)
{
	const char *world = "mon";
	NYAS_TYPES_LOG("Hola %s\n", world);
	nt_arr pix = nyas_arr_create(8, 4);
	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 0);
	NYAS_TYPES_ASSERT(nyas_arr_cap(pix) == 8);
	NYAS_TYPES_ASSERT(nyas_arr_elem_size(pix) == 4);
	float *val = nyas_arr_push(&pix);
	*val = 4.0f;
	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 1);
	NYAS_TYPES_ASSERT(nyas_arr_cap(pix) == 8);
	NYAS_TYPES_ASSERT(nyas_arr_elem_size(pix) == 4);
	NYAS_TYPES_ASSERT(*(float *)nyas_arr_at(pix, 0) == 4.0f);
	val = nyas_arr_pushv(&pix, 4);
	val[0] = 10.0f;
	val[1] = -4.0f;
	uint64_t tmp = (1UL << 63) | (1UL << 31);
	*(((double *)val) + 2) = *(double *)&tmp;

	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 5);
	NYAS_TYPES_ASSERT(nyas_arr_cap(pix) == 8);
	NYAS_TYPES_ASSERT(nyas_arr_elem_size(pix) == 4);
	NYAS_TYPES_ASSERT(*(float *)nyas_arr_at(pix, 2) == -4.0f);
	NYAS_TYPES_LOG("Both should be -0.0f : %f, %f.\n", val[2], val[3]);

	nt_arr arr_cpy = nyas_arr_create(16, 4);
	nyas_arr_cpy(&arr_cpy, pix);
	assert(nyas_arr_eq(pix, arr_cpy) == true);

	NYAS_TYPES_ASSERT(*(float *)nyas_arr_rm(pix, 0) == 0.0f);
	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 4);
	NYAS_TYPES_ASSERT(*(float *)nyas_arr_extract(pix, 1) == 10.0f);
	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 3);
	nyas_arr_clear(pix);
	NYAS_TYPES_ASSERT(nyas_arr_len(pix) == 0);
	nyas_arr_destroy(pix);
	pix = NULL;
}

#endif // NYAS_TYPES_TEST
#endif // NYAS_TYPES_IMPL
#endif // NYAS_TYPES_H
