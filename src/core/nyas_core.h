#ifndef NYAS_NYAS_CORE_H
#define NYAS_NYAS_CORE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define NYAS_ASSERT(X) (assert(X))

#define NYAS_DECL_ARR(T)                         \
	struct nyarr_##T {                           \
		ptrdiff_t count;                         \
		T at[];                                  \
	};                                           \
	T *nyarr_##T##_push(struct nyarr_##T **arr); \
	void nyarr_##T##_push_value(struct nyarr_##T **arr, T value)

#define NYAS_IMPL_ARR(T)                                                                       \
	T *nyarr_##T##_push(struct nyarr_##T **arr)                                                \
	{                                                                                          \
		ptrdiff_t count = *arr ? (*arr)->count : 0;                                            \
		if (!(count & (count + 1))) {                                                          \
			void *new =                                                                        \
			  realloc(*arr, sizeof(struct nyarr_##T) + (count + 1) * 2 * sizeof(*(*arr)->at)); \
			if (!new) {                                                                        \
				return NULL;                                                                   \
			}                                                                                  \
			*arr = new;                                                                        \
		}                                                                                      \
		(*arr)->count = count + 1;                                                             \
		return &(*arr)->at[count];                                                             \
	}                                                                                          \
                                                                                               \
	void nyarr_##T##_push_value(struct nyarr_##T **arr, T value)                               \
	{                                                                                          \
		*(nyarr_##T##_push(arr)) = value;                                                      \
	}                                                                                          \
	int main(int argc, char **argv)

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

union nyas4f {
	struct nyas_vec4 {
		float x, y, z, w;
	} p;

	struct nyas_color {
		float r, g, b, a;
	} c;

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

#endif // NYAS_NYAS_CORE_H
