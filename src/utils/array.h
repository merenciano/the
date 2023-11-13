#ifndef NYAS_UTILS_ARRAY_H
#define NYAS_UTILS_ARRAY_H

#include <stddef.h>

#if defined(NYAS_ARR_HELPERS)
#define nyas_arr_push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define nyas_arr_pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define nyas_arr_cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define nyas_arr_concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define nyas_arr_clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define nyas_arr_eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define nyas_arr_empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
#endif

typedef void *nyas_arr;  //!< Pointer to array type.
typedef void *nyas_elem; //!< Pointer to array element type.

/**
 * Creates a new instance of an array, allocating the required memory.
 * The initial allocation will be at least of 'n * sz' bytes.
 * @param n Minimum capacity (i.e. number of elements that can hold without
 * resizing its buffer).
 * @param sz Element size.
 * @return Pointer to the instance created.
 */
extern nyas_arr nyas_arr_create(size_t n, size_t sz);

/**
 * Destroys the array structure, releasing all allocations related.
 * @param arr Array instance.
 */
extern void nyas_arr_destroy(nyas_arr arr);

/**
 * Adds N elements at the array's tail.
 * @param arr Reference to an array's instance.
 * @param n Number of elements to add.
 * @return Pointer to the first consecutive pushed element.
 */
extern nyas_elem nyas_arr_npush(nyas_arr *arr, size_t n);

/**
 * Removes N consecutive elements from the array's tail.
 * This operation reduces the length of the array but not its capacity.
 * The returned pointer gets invalidated when the array grows back.
 * @param arr Array instance.
 * @param n Number of elements to pop out.
 * @return Volatile pointer to the first consecutive element removed.
 */
extern nyas_elem nyas_arr_npop(nyas_arr arr, size_t n);

/**
 * Gets the array's element stored at the given index.
 * @param arr Array instance.
 * @param idx Index of the element.
 * @return Pointer to the element.
 */
extern nyas_elem nyas_arr_at(nyas_arr arr, size_t idx);

/**
 * Gets the current length of the array (i.e. number of elements).
 * @param arr Array instance.
 * @return Current number of elements.
 */
extern size_t nyas_arr_len(nyas_arr arr);

/**
 * Compares two arrays.
 * Returns a signed number representing 'a - b' in terms of difference.
 * The value depends on the difference between the first non-matching fields,
 * evaluated following this sequence:
 * 1) Pointer value if any of them is NULL;
 * 2) Length (i.e. number of elements);
 * 3) Element size;
 * 4) Array contents (i.e. element values);
 * 5) If all the above comparisons result in equal values, zero is returned;
 * @param a One array instance.
 * @param b Another array instance.
 * @return Difference between 'a' and 'b' (see description for detailed info).
 */
extern ptrdiff_t nyas_arr_cmp(nyas_arr a, nyas_arr b);

/**
 * Swaps two elements of an array.
 * @param arr Array instance.
 * @param a Element index of one swap param.
 * @param b Element index of other swap param.
 * @return Reference to the element at 'a' or NULL on error.
 */
extern nyas_elem nyas_arr_swap(nyas_arr arr, size_t a, size_t b);

/**
 * Removes one element of an array.
 * Copies the last element of the array at the removed one's index.
 * The previous index of the relocated element becomes invalidated.
 * @param arr Array instance.
 * @param idx Element index.
 * @return Volatile pointer to the element at the invalidated index or NULL
 * on error.
 */
extern nyas_elem nyas_arr_rm(nyas_arr arr, size_t idx);

/**
 * Extracts an element of an array.
 * The extracted element is removed from the array and the returned pointer
 * becomes invalidated when the array grows back.
 * This function is equivalent to swap 'idx' with the last element, then pop.
 * @param arr Array instance.
 * @param idx Element index.
 * @return Volatile reference of the extracted element.
 */
extern nyas_elem nyas_arr_extract(nyas_arr arr, size_t idx);

/**
 * Copy elements at range src[from, to] to dst[at, ...].
 * @param dst Reference to an array's instance to copy elements to.
 * @param src Array instance to copy elements from.
 * @param from Index of the first element to copy from 'src'.
 * @param to Index of the last element to copy from 'src' (including itself).
 * @param at Index where start copying elements at 'dst', the value '-1'
 * is converted to 'arr_len(dst)'.
 * @return Reference to the first copied element from src array.
 */
extern nyas_elem nyas_arr_cpyfta(nyas_arr *dst,
                                 nyas_arr src,
                                 size_t from,
                                 size_t to,
                                 size_t at);

/**
 * Resets the array.
 * The array is left in empty state. The previous allocation remains unchanged.
 * @param arr Array instance.
 * @param elem_size New element size, for keeping the previous use 0.
 */
extern void nyas_arr_reset(nyas_arr arr, size_t elem_size);

/*
 * Short naming definitions (different granularities).
 */
#if defined(NYAS_ARR_NS_NONE)
typedef void *arr;
typedef void *elem;
#define create nyas_arr_create
#define destroy nyas_arr_destroy
#define npush nyas_arr_npush
#define npop nyas_arr_npop
#define at nyas_arr_at
#define len nyas_arr_len
#define cmp nyas_arr_cmp
#define swap nyas_arr_swap
#define rm nyas_arr_rm
#define extract nyas_arr_extract
#define cpyfta nyas_arr_cpyfta
#define reset nyas_arr_reset

#if defined(NYAS_ARR_HELPERS)
#define push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
#endif // NYAS_ARR_HELPERS

#elif defined(NYAS_ARR_NS_ACRONYM)
typedef void *na_arr;
typedef void *na_elem;
#define na_create nyas_arr_create
#define na_destroy nyas_arr_destroy
#define na_npush nyas_arr_npush
#define na_npop nyas_arr_npop
#define na_at nyas_arr_at
#define na_len nyas_arr_len
#define na_cmp nyas_arr_cmp
#define na_swap nyas_arr_swap
#define na_rm nyas_arr_rm
#define na_extract nyas_arr_extract
#define na_cpyfta nyas_arr_cpyfta
#define na_reset nyas_arr_reset

#if defined(NYAS_ARR_HELPERS)
#define na_push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define na_pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define na_cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define na_concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define na_clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define na_eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define na_empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
#endif // NYAS_ARR_HELPERS

#elif defined(NYAS_ARR_NS_COMPACT)
typedef void *nyarr;
typedef void *nyelem;
#define nyarr_create nyas_arr_create
#define nyarr_destroy nyas_arr_destroy
#define nyarr_npush nyas_arr_npush
#define nyarr_npop nyas_arr_npop
#define nyarr_at nyas_arr_at
#define nyarr_len nyas_arr_len
#define nyarr_cmp nyas_arr_cmp
#define nyarr_swap nyas_arr_swap
#define nyarr_rm nyas_arr_rm
#define nyarr_extract nyas_arr_extract
#define nyarr_cpyfta nyas_arr_cpyfta
#define nyarr_reset nyas_arr_reset

#if defined(NYAS_ARR_HELPERS)
#define nyarr_push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define nyarr_pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define nyarr_cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define nyarr_concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define nyarr_clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define nyarr_eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define nyarr_empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
#endif // NYAS_ARR_HELPERS

#elif defined(NYAS_ARR_NS_MODULE_ONLY)
typedef void *arr_arr;
typedef void *arr_elem;
#define arr_create nyas_arr_create
#define arr_destroy nyas_arr_destroy
#define arr_npush nyas_arr_npush
#define arr_npop nyas_arr_npop
#define arr_at nyas_arr_at
#define arr_len nyas_arr_len
#define arr_cmp nyas_arr_cmp
#define arr_swap nyas_arr_swap
#define arr_rm nyas_arr_rm
#define arr_extract nyas_arr_extract
#define arr_cpyfta nyas_arr_cpyfta
#define arr_reset nyas_arr_reset

#if defined(NYAS_ARR_HELPERS)
#define arr_push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define arr_pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define arr_cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define arr_concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define arr_clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define arr_eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define arr_empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
#endif // NYAS_ARR_HELPERS
#endif

/**
 * Debug functions.
 */
#ifdef NYAS_DEBUG
#ifndef NYAS_LOG
#include <stdio.h>
#define NYAS_LOG printf
#endif

/**
 * Prints info and contents of an array.
 * Useful for debug builds.
 */
static void
NYAS_DBG_PRINTARR(void *arr, void (*print_elem)(void *elem))
{
	NYAS_LOG("Array %x -> Length(%ul), Capacity(%ul), ElemSize(%ul):\n", arr,
	         *((size_t *)arr - 2), *((size_t *)arr - 1), *((size_t *)arr - 3));
	for (int i = 0; i < nyas_arr_len(arr); ++i) {
		printf(" [%d]", i);
		print_elem(nyas_arr_at(i));
	}
}
#endif // NYAS_DEBUG
#endif // NYAS_UTILS_ARRAY_H

/**
 * Unit testing functions.
 */
#ifdef NYAS_ARR_TEST
#include <stdint.h>
#include <stdio.h>
#ifndef NYAS_LOG
#define NYAS_LOG printf
#endif

#ifndef NYAS_ASSERT
#include <assert.h>
#define NYAS_ASSERT assert
#endif

void
nyas_arr_test(void)
{
	const char *world = "mon";
	NYAS_LOG("Hola %s\n", world);
	nyas_arr pix = nyas_arr_create(8, 4);
	NYAS_ASSERT(nyas_arr_len(pix) == 0);
	//NYAS_ASSERT(nyas_arr_cap(pix) == 8);
	//NYAS_ASSERT(nyas_arr_elem_size(pix) == 4);
	// float *val = nyas_arr_push(&pix);
	float *val = nyas_arr_npush(&pix, 1);
	*val = 4.0f;
	NYAS_ASSERT(nyas_arr_len(pix) == 1);
	//NYAS_ASSERT(nyas_arr_cap(pix) == 8);
	//NYAS_ASSERT(nyas_arr_elem_size(pix) == 4);
	NYAS_ASSERT(*(float *)nyas_arr_at(pix, 0) == 4.0f);
	val = nyas_arr_npush(&pix, 4);
	val[0] = 10.0f;
	val[1] = -4.0f;
	uint64_t tmp = (1UL << 63) | (1UL << 31);
	*(((double *)val) + 2) = *(double *)&tmp;

	NYAS_ASSERT(nyas_arr_len(pix) == 5);
	//NYAS_ASSERT(nyas_arr_cap(pix) == 8);
	//NYAS_ASSERT(nyas_arr_elem_size(pix) == 4);
	NYAS_ASSERT(*(float *)nyas_arr_at(pix, 2) == -4.0f);
	NYAS_LOG("Both should be -0.0f : %f, %f.\n", val[2], val[3]);

	nyas_arr arr_cpy = nyas_arr_create(16, 4);
	nyas_arr_cpyfta(&arr_cpy, pix, 0, 0, 0);
	assert(nyas_arr_cmp(pix, arr_cpy) == 0);

	NYAS_ASSERT(*(float *)nyas_arr_rm(pix, 0) == 0.0f);
	NYAS_ASSERT(nyas_arr_len(pix) == 4);
	NYAS_ASSERT(*(float *)nyas_arr_extract(pix, 1) == 10.0f);
	NYAS_ASSERT(nyas_arr_len(pix) == 3);
	nyas_arr_reset(pix, 0);
	NYAS_ASSERT(nyas_arr_len(pix) == 0);
	nyas_arr_destroy(pix);
	pix = NULL;
}

#endif // NYAS_ARR_TEST
