#ifndef NYAS_NYAS_CORE_H
#define NYAS_NYAS_CORE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define NYAS_ASSERT(X) (assert(X))

#define NYAS_DECL_ARR(T)                                          \
	struct nyarr_##T {                                            \
		ptrdiff_t count;                                          \
		T at[];                                                   \
	};                                                            \
	T *nyarr_##T##_push(struct nyarr_##T **arr);                  \
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
	}                                                                                    \
                                                                                         \
	void nyarr_##T##_release(void *arr)                                                  \
	{                                                                                    \
		FREE(arr);                                                                       \
	}                                                                                    \
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

struct nyarr {
	ptrdiff_t count;
	char at[];
};

struct nypool {
	struct nyarr *buf;
	int next;
	int count;
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

#define NYAS_ENUM_INDEX(ENUM_TYPE, ID) (ENUM_TYPE##_##ID - ENUM_TYPE)
#define NYAS_ENUM_FLAG(ENUM_TYPE, ID) (1U << (NYAS_ENUM_INDEX(ENUM_TYPE, ID)))
#define NYAS_ENUM_COUNT(ENUM_TYPE) (NYAS_ENUM_INDEX(ENUM_TYPE, DEFAULT))

typedef int nyas_flags;
typedef int nyas_enum;
typedef int nyas_error;

typedef enum nyas_enum_ {
	NYAS_NULL,

	NYAS_DRAW_RESOURCE,
	NYAS_DRAW_RESOURCE_CREATE,
	NYAS_DRAW_RESOURCE_DIRTY,
	NYAS_DRAW_RESOURCE_DEFAULT,

	NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_TYPE_2D = NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_TYPE_2D_ARRAY,
	NYAS_TEXTURE_TYPE_CUBEMAP,
	NYAS_TEXTURE_TYPE_CUBEMAP_ARRAY,
	NYAS_TEXTURE_TYPE_DEFAULT,

	NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FORMAT_DEPTH = NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FORMAT_STENCIL,
	NYAS_TEXTURE_FORMAT_DEPTH_STENCIL,
	NYAS_TEXTURE_FORMAT_R_8,
	NYAS_TEXTURE_FORMAT_RG_8,
	NYAS_TEXTURE_FORMAT_RGB_8,
	NYAS_TEXTURE_FORMAT_RGBA_8,
	NYAS_TEXTURE_FORMAT_SRGB_8,
	NYAS_TEXTURE_FORMAT_SRGBA_8,
	NYAS_TEXTURE_FORMAT_R_16F,
	NYAS_TEXTURE_FORMAT_RG_16F,
	NYAS_TEXTURE_FORMAT_RGB_16F,
	NYAS_TEXTURE_FORMAT_RGBA_16F,
	NYAS_TEXTURE_FORMAT_R_32F,
	NYAS_TEXTURE_FORMAT_RG_32F,
	NYAS_TEXTURE_FORMAT_RGB_32F,
	NYAS_TEXTURE_FORMAT_RGBA_32F,
	NYAS_TEXTURE_FORMAT_DEFAULT,

	NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_FILTER_LINEAR = NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_DEFAULT,

	NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_WRAP_CLAMP = NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_WRAP_WHITE,
	NYAS_TEXTURE_WRAP_BLACK,
	NYAS_TEXTURE_WRAP_REPEAT,
	NYAS_TEXTURE_WRAP_MIRROR,
	NYAS_TEXTURE_WRAP_DEFAULT,

	NYAS_TEXTURE_FACE,
	NYAS_TEXTURE_FACE_POS_X = NYAS_TEXTURE_FACE,
	NYAS_TEXTURE_FACE_NEG_X,
	NYAS_TEXTURE_FACE_POS_Y,
	NYAS_TEXTURE_FACE_NEG_Y,
	NYAS_TEXTURE_FACE_POS_Z,
	NYAS_TEXTURE_FACE_2D,
	NYAS_TEXTURE_FACE_DEFAULT,

	NYAS_VERTEX_ATTRIBUTE,
	NYAS_VERTEX_ATTRIBUTE_POSITION = NYAS_VERTEX_ATTRIBUTE,
	NYAS_VERTEX_ATTRIBUTE_NORMAL,
	NYAS_VERTEX_ATTRIBUTE_TANGENT,
	NYAS_VERTEX_ATTRIBUTE_BITANGENT,
	NYAS_VERTEX_ATTRIBUTE_UV,
	NYAS_VERTEX_ATTRIBUTE_COLOR,
	NYAS_VERTEX_ATTRIBUTE_DEFAULT,

	NYAS_DRAW_BLEND,
	NYAS_DRAW_BLEND_ONE = NYAS_DRAW_BLEND,
	NYAS_DRAW_BLEND_SRC_ALPHA,
	NYAS_DRAW_BLEND_ONE_MINUS_SRC_ALPHA,
	NYAS_DRAW_BLEND_ZERO,
	NYAS_DRAW_BLEND_DEFAULT,

	NYAS_DRAW_CULL,
	NYAS_DRAW_CULL_FRONT = NYAS_DRAW_CULL,
	NYAS_DRAW_CULL_BACK,
	NYAS_DRAW_CULL_FRONT_AND_BACK,
	NYAS_DRAW_CULL_DEFAULT,

	NYAS_DRAW_DEPTH,
	NYAS_DRAW_DEPTH_LEQUAL = NYAS_DRAW_DEPTH,
	NYAS_DRAW_DEPTH_LESS,
	NYAS_DRAW_DEPTH_DEFAULT,

	NYAS_DRAW_OP,
	NYAS_DRAW_OP_COLOR_CLEAR = NYAS_DRAW_OP,
	NYAS_DRAW_OP_DEPTH_CLEAR,
	NYAS_DRAW_OP_DEPTH_TEST,
	NYAS_DRAW_OP_DEPTH_WRITE,
	NYAS_DRAW_OP_STENCIL_CLEAR,
	NYAS_DRAW_OP_STENCIL_TEST,
	NYAS_DRAW_OP_STENCIL_WRITE,
	NYAS_DRAW_OP_BLEND,
	NYAS_DRAW_OP_CULL,
	NYAS_DRAW_OP_SCISSOR,
	NYAS_DRAW_OP_DEFAULT,

	NYAS_TEXTURE_TYPE_COUNT = NYAS_TEXTURE_TYPE_DEFAULT - NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_FORMAT_COUNT = NYAS_TEXTURE_FORMAT_DEFAULT - NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FILTER_COUNT = NYAS_TEXTURE_FILTER_DEFAULT - NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_WRAP_COUNT = NYAS_TEXTURE_WRAP_DEFAULT - NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_FACE_COUNT = NYAS_TEXTURE_FACE_DEFAULT - NYAS_TEXTURE_FACE,
	NYAS_VERTEX_ATTRIBUTE_COUNT = NYAS_VERTEX_ATTRIBUTE_DEFAULT - NYAS_VERTEX_ATTRIBUTE,
	NYAS_DRAW_BLEND_COUNT = NYAS_DRAW_BLEND_DEFAULT - NYAS_DRAW_BLEND,
	NYAS_DRAW_CULL_COUNT = NYAS_DRAW_CULL_DEFAULT - NYAS_DRAW_CULL,
	NYAS_DRAW_DEPTH_COUNT = NYAS_DRAW_DEPTH_DEFAULT - NYAS_DRAW_DEPTH,
	NYAS_DRAW_OP_COUNT = NYAS_DRAW_OP_DEFAULT - NYAS_DRAW_OP,

	NYAS_FLAG_NONE = 0,

	NYAS_FLAG_DR_CREATE = NYAS_ENUM_FLAG(NYAS_DRAW_RESOURCE, CREATE),

	NYAS_FLAG_VA_POSITION = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, POSITION),
	NYAS_FLAG_VA_NORMAL = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, NORMAL),
	NYAS_FLAG_VA_TANGENT = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, TANGENT),
	NYAS_FLAG_VA_BITANGENT = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, BITANGENT),
	NYAS_FLAG_VA_UV = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, UV),
	NYAS_FLAG_VA_COLOR = NYAS_ENUM_FLAG(NYAS_VERTEX_ATTRIBUTE, COLOR),

	NYAS_FLAG_DO_COLOR_CLEAR = NYAS_ENUM_FLAG(NYAS_DRAW_OP, COLOR_CLEAR),
	NYAS_FLAG_DO_DEPTH_CLEAR = NYAS_ENUM_FLAG(NYAS_DRAW_OP, DEPTH_CLEAR),
	NYAS_FLAG_DO_DEPTH_TEST = NYAS_ENUM_FLAG(NYAS_DRAW_OP, DEPTH_TEST),
	NYAS_FLAG_DO_DEPTH_WRITE = NYAS_ENUM_FLAG(NYAS_DRAW_OP, DEPTH_WRITE),
	NYAS_FLAG_DO_STENCIL_CLEAR = NYAS_ENUM_FLAG(NYAS_DRAW_OP, STENCIL_CLEAR),
	NYAS_FLAG_DO_STENCIL_TEST = NYAS_ENUM_FLAG(NYAS_DRAW_OP, STENCIL_TEST),
	NYAS_FLAG_DO_STENCIL_WRITE = NYAS_ENUM_FLAG(NYAS_DRAW_OP, STENCIL_WRITE),
	NYAS_FLAG_DO_BLEND = NYAS_ENUM_FLAG(NYAS_DRAW_OP, BLEND),
	NYAS_FLAG_DO_CULL = NYAS_ENUM_FLAG(NYAS_DRAW_OP, CULL),
	NYAS_FLAG_DO_SCISSOR = NYAS_ENUM_FLAG(NYAS_DRAW_OP, SCISSOR),

	NYAS_OK = 0,
	NYAS_ERROR = -1,
	NYAS_DEFAULT = -50,
	NYAS_NOOP = -51,
	NYAS_NONE = -53,

	NYAS_ERR_FAIL = -90,
	NYAS_ERR_NULL = -91,
	NYAS_ERR_UNINIT = -92,
	NYAS_ERR_MEM = -100,
	NYAS_ERR_ALLOC = -110,
	NYAS_ERR_RANGE = -120, // Out of bounds access.
	NYAS_ERR_STREAM = -200,
	NYAS_ERR_FILE = -210,
	NYAS_ERR_THREAD = -300,
	NYAS_ERR_ARG = -400,
	NYAS_ERR_NULLARG = -401,
	NYAS_ERR_BADARG = -402,
	NYAS_ERR_SWITCH = -500,
	NYAS_ERR_SWITCH_DEFAULT = -501, // Switch default label.
} nyas_enum_;

#endif // NYAS_NYAS_CORE_H
