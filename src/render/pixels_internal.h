#ifndef NYAS_PIXELS_INTERNAL_H
#define NYAS_PIXELS_INTERNAL_H

#include "pixels.h"
#include "utils/array.h"

#ifdef NYAS_PIXEL_CHECKS
#define CHECK_HANDLE(TYPE, HANDLE)                  \
	({                                                    \
		NYAS_ASSERT(HANDLE >= 0 && "Bad resource state"); \
		NYAS_ASSERT(HANDLE < nyas_arr_len(TYPE##_pool) && \
		            "Out of bounds handle");              \
	})
#else
#define CHECK_HANDLE(TYPE, HANDLE) (void)
#endif // NYAS_PIXEL_CHECKS

enum vtxattr {
	A_POSITION = 0,
	A_NORMAL,
	A_TANGENT,
	A_BITANGENT,
	A_UV,
	VERTEX_ATTRIBUTE_COUNT
};

typedef enum resource_flags {
	RF_DIRTY = 1 << 0,
	RF_FREE_AFTER_LOAD = 1 << 1
} resource_flags;

typedef struct resource {
	int id;
	int flags;
} r_resource;

typedef struct {
	r_resource res;
	float *vtx;
	IDX_T *idx;
	unsigned int vtx_size;
	unsigned int elements;
	int attr_flags;
	unsigned int internal_buffers_id[2];
} r_mesh;

typedef struct {
	r_resource res;
	void *pix[6];
	int width;
	int height;
	int type;
} r_tex;

typedef struct {
	r_resource res;
	int width;
	int height;
	nyas_tex color_tex;
	nyas_tex depth_tex;
} r_fb;

enum shadata_type { SHADATA_COMMON = 0, SHADATA_UNIT = 1 };

typedef struct {
	int data;
	int tex;
	int cubemap;
} shadata_loc;

typedef struct {
	r_resource res;
	const char *shader_name;
	shadata_loc data_loc[2];
	unsigned int vert;
	unsigned int frag;
} r_shader;

extern nyas_arr mesh_pool;
extern nyas_arr tex_pool;
extern nyas_arr shader_pool;
extern nyas_arr framebuffer_pool;

typedef struct nyas_cmd_queue {
	nyas_cmd *curr;
	nyas_cmd *curr_last;
	nyas_cmd *next;
	nyas_cmd *next_last;
} cmd_queue;

extern cmd_queue render_queue;

#endif // NYAS_PIXELS_INTERNAL_H
