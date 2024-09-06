#ifndef NYAS_CORE_COMMON_H
#define NYAS_CORE_COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define NYAS_ASSERT(X) (assert(X))

#define NYAS_DECL_ARR(T)                         \
	struct nyarr_##T {                           \
		ptrdiff_t count;                         \
		T at[];                                  \
	};                                           \
	T *nyarr_##T##_push(struct nyarr_##T **arr); \
	void nyarr_##T##_push_value(struct nyarr_##T **arr, T value); \
	void nyarr_##T##_release(void *arr)
#define NYAS_IMPL_ARR_MA(T, MALLOC, FREE)                                                \
	T *nyarr_##T##_push(struct nyarr_##T **arr)                                          \
	{                                                                                    \
		ptrdiff_t count = *arr ? (*arr)->count : 0;                                      \
		if (!(count & (count + 1))) {                                                    \
			struct nyarr_##T *mem = MALLOC(sizeof(**arr) + (count + 1) * 2 * sizeof(T)); \
			if (!mem) {                                                                  \
				return NULL;                                                             \
			}                                                                            \
			for (int i = 0; i < count; ++i) {                                            \
				mem->at[i] = (*arr)->at[i];                                              \
			}                                                                            \
			*arr = mem;                                                                  \
		}                                                                                \
		(*arr)->count = count + 1;                                                       \
		return &(*arr)->at[count];                                                       \
	}                                                                                    \
                                                                                         \
	void nyarr_##T##_push_value(struct nyarr_##T **arr, T value)                         \
	{                                                                                    \
		*(nyarr_##T##_push(arr)) = value;                                                \
	}                                                                                       \
                                                                                         \
	void nyarr_##T##_release(void *arr)                                                     \
	{                                                   \
		FREE(arr);\
	}\
	int main(int argc, char **argv)

#define NYAS_IMPL_ARR(TYPE) NYAS_IMPL_ARR_MA(TYPE, malloc, free)

#define NYAS_DECL_POOL(TYPE)                             \
	struct nypool_##TYPE {                               \
		struct nyarr_##TYPE *buf;                        \
		int next;                                        \
		int count;                                       \
	};                                                   \
	int nypool_##TYPE##_add(struct nypool_##TYPE *pool); \
	void nypool_##TYPE##_rm(struct nypool_##TYPE *pool, int i)

#define NYAS_IMPL_POOL(TYPE)                                   \
	int nypool_##TYPE##_add(struct nypool_##TYPE *pool)        \
	{                                                          \
		if (!pool) {                                           \
			return -1;                                         \
		}                                                      \
                                                               \
		int cap = pool->buf ? pool->buf->count : 0;            \
		if (pool->next == cap) {                               \
			nyarr_##TYPE##_push(&pool->buf);                   \
			pool->next = pool->buf->count;                     \
			pool->count++;                                     \
			return pool->buf->count - 1;                       \
		}                                                      \
                                                               \
		pool->count++;                                         \
		int ret = pool->next;                                  \
		pool->next = *(int *)&pool->buf->at[pool->next];       \
		return ret;                                            \
	}                                                          \
                                                               \
	void nypool_##TYPE##_rm(struct nypool_##TYPE *pool, int i) \
	{                                                          \
		if (!pool) {                                           \
			return;                                            \
		}                                                      \
                                                               \
		if (i < pool->buf->count) {                            \
			*(int *)&pool->buf->at[i] = pool->next;            \
			pool->next = i;                                    \
			pool->count--;                                     \
		}                                                      \
	}                                                          \
	int main(int argc, char **argv)

#define NYAS_DECL_STACK(TYPE)                                  \
	struct nystack_##TYPE {                                    \
		struct nyarr_##TYPE *buf;                              \
		ptrdiff_t tail;                                        \
	};                                                         \
                                                               \
	TYPE *nystack_##TYPE##_push(struct nystack_##TYPE *stack); \
	TYPE *nystack_##TYPE##_pop(struct nystack_##TYPE *stack)

#define NYAS_IMPL_STACK(TYPE)                                   \
	TYPE *nystack_##TYPE##_push(struct nystack_##TYPE *stack)   \
	{                                                           \
		if (!stack) {                                           \
			return NULL;                                        \
		}                                                       \
                                                                \
		int cap = stack->buf ? stack->buf->count : 0;           \
		if (stack->tail == cap) {                               \
			nyarr_##TYPE##_push(&stack->buf);                   \
		}                                                       \
		stack->tail++;                                          \
		return &stack->buf->at[stack->tail - 1];                \
	}                                                           \
                                                                \
	TYPE *nystack_##TYPE##_pop(struct nystack_##TYPE *stack)    \
	{                                                           \
		if (!stack || !(stack->tail) || !(stack->buf->count)) { \
			return NULL;                                        \
		}                                                       \
		return &stack->buf->at[--(stack->tail)];                \
	}                                                           \
	int main(int argc, char **argv)

enum nyas_defs {
	NYAS_ERR = 0,
	NYAS_OK = 1,
	NYAS_END = -2,
	NYAS_NULL = -3,
	NYAS_ERR_SWITCH_DEFAULT = -4,
	NYAS_ERR_INVALID_PTR = -5,
	NYAS_ERR_ALLOC = -10,
	NYAS_ERR_FILE = -20,
	NYAS_ERR_THREAD = -30,
	NYAS_DEFAULT = -50,
	NYAS_NOOP = -51,
	NYAS_INVALID = -52,
	NYAS_NONE = -53,
};

struct nyas_point {
	int x, y;
};

struct nyas_rect {
	int x, y, w, h;
};

struct nyas_vec2 {
	float x, y;
};

struct nyas_vec3 {
	float x, y, z;
};

struct nyas_vec4 {
	float x, y, z, w;
};

struct nyas_color {
	float r, g, b, a;
};

union nyas4f {
	struct nyas_vec4 p;
	struct nyas_color c;
	float v[4];
};

typedef float nyas_mat4[16];

union nyas_mat4 {
	struct nyas_vec4 r[4];
	struct nyas_4x4 {
		struct nyas_vec4 x;
		struct nyas_vec4 y;
		struct nyas_vec4 z;
		struct nyas_vec4 w;
	} row;
	float v[16];
	float p[4][4];
	struct nyas_mat4x4 {
		float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33;
	} m;
};

#endif // NYAS_CORE_COMMON_H
