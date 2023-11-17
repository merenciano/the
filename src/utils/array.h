#ifndef NYAS_UTILS_ARRAY_H
#define NYAS_UTILS_ARRAY_H

#include <stddef.h>

#ifndef NYAS_ARR_NO_HELPERS
#define NYAS_arr_push(...) nyas_arr_npush(__VA_ARGS__, 1)
#define NYAS_arr_pop(...) nyas_arr_npop(__VA_ARGS__, 1)
#define NYAS_arr_cpy(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, 0)
#define NYAS_arr_concat(...) nyas_arr_cpyfta(__VA_ARGS__, 0, 0, -1)
#define NYAS_arr_clear(...) nyas_arr_reset(__VA_ARGS__, 0)
#define NYAS_arr_eq(...) (nyas_arr_cmp(__VA_ARGS__) == 0)
#define NYAS_arr_empty(...) (nyas_arr_len(__VA_ARGS__) == 0)
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

#endif // NYAS_UTILS_ARRAY_H
