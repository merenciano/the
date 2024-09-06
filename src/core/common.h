#ifndef THE_CORE_COMMON_H
#define THE_CORE_COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define THE_ASSERT(X) (assert(X))

#define THE_DECL_ARR(T)                         \
	struct thearr_##T {                           \
		ptrdiff_t count;                         \
		T at[];                                  \
	};                                           \
	T *thearr_##T##_push(struct thearr_##T **arr); \
	void thearr_##T##_push_value(struct thearr_##T **arr, T value); \
	void thearr_##T##_release(void *arr)
#define THE_IMPL_ARR_MA(T, MALLOC, FREE)                                                \
	T *thearr_##T##_push(struct thearr_##T **arr)                                          \
	{                                                                                    \
		ptrdiff_t count = *arr ? (*arr)->count : 0;                                      \
		if (!(count & (count + 1))) {                                                    \
			struct thearr_##T *mem = MALLOC(sizeof(**arr) + (count + 1) * 2 * sizeof(T)); \
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
	void thearr_##T##_push_value(struct thearr_##T **arr, T value)                         \
	{                                                                                    \
		*(thearr_##T##_push(arr)) = value;                                                \
	}                                                                                       \
                                                                                         \
	void thearr_##T##_release(void *arr)                                                     \
	{                                                   \
		FREE(arr);\
	}\
	int main(int argc, char **argv)

#define THE_IMPL_ARR(TYPE) THE_IMPL_ARR_MA(TYPE, malloc, free)

#define THE_DECL_POOL(TYPE)                             \
	struct thepool_##TYPE {                               \
		struct thearr_##TYPE *buf;                        \
		int next;                                        \
		int count;                                       \
	};                                                   \
	int thepool_##TYPE##_add(struct thepool_##TYPE *pool); \
	void thepool_##TYPE##_rm(struct thepool_##TYPE *pool, int i)

#define THE_IMPL_POOL(TYPE)                                   \
	int thepool_##TYPE##_add(struct thepool_##TYPE *pool)        \
	{                                                          \
		if (!pool) {                                           \
			return -1;                                         \
		}                                                      \
                                                               \
		int cap = pool->buf ? pool->buf->count : 0;            \
		if (pool->next == cap) {                               \
			thearr_##TYPE##_push(&pool->buf);                   \
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
	void thepool_##TYPE##_rm(struct thepool_##TYPE *pool, int i) \
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

#define THE_DECL_STACK(TYPE)                                  \
	struct thestack_##TYPE {                                    \
		struct thearr_##TYPE *buf;                              \
		ptrdiff_t tail;                                        \
	};                                                         \
                                                               \
	TYPE *thestack_##TYPE##_push(struct thestack_##TYPE *stack); \
	TYPE *thestack_##TYPE##_pop(struct thestack_##TYPE *stack)

#define THE_IMPL_STACK(TYPE)                                   \
	TYPE *thestack_##TYPE##_push(struct thestack_##TYPE *stack)   \
	{                                                           \
		if (!stack) {                                           \
			return NULL;                                        \
		}                                                       \
                                                                \
		int cap = stack->buf ? stack->buf->count : 0;           \
		if (stack->tail == cap) {                               \
			thearr_##TYPE##_push(&stack->buf);                   \
		}                                                       \
		stack->tail++;                                          \
		return &stack->buf->at[stack->tail - 1];                \
	}                                                           \
                                                                \
	TYPE *thestack_##TYPE##_pop(struct thestack_##TYPE *stack)    \
	{                                                           \
		if (!stack || !(stack->tail) || !(stack->buf->count)) { \
			return NULL;                                        \
		}                                                       \
		return &stack->buf->at[--(stack->tail)];                \
	}                                                           \
	int main(int argc, char **argv)

enum the_defs {
	THE_ERR = 0,
	THE_OK = 1,
	THE_END = -2,
	THE_NULL = -3,
	THE_ERR_SWITCH_DEFAULT = -4,
	THE_ERR_INVALID_PTR = -5,
	THE_ERR_ALLOC = -10,
	THE_ERR_FILE = -20,
	THE_ERR_THREAD = -30,
	THE_DEFAULT = -50,
	THE_NOOP = -51,
	THE_INVALID = -52,
	THE_NONE = -53,
};

struct the_point {
	int x, y;
};

struct the_rect {
	int x, y, w, h;
};

struct the_vec2 {
	float x, y;
};

struct the_vec3 {
	float x, y, z;
};

struct the_vec4 {
	float x, y, z, w;
};

struct the_color {
	float r, g, b, a;
};

union the4f {
	struct the_vec4 p;
	struct the_color c;
	float v[4];
};

typedef float the_mat4[16];

union the_mat4 {
	struct the_vec4 r[4];
	struct the_4x4 {
		struct the_vec4 x;
		struct the_vec4 y;
		struct the_vec4 z;
		struct the_vec4 w;
	} row;
	float v[16];
	float p[4][4];
	struct the_mat4x4 {
		float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33;
	} m;
};

#endif // THE_CORE_COMMON_H
