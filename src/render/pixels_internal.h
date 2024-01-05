#ifndef NYAS_PIXELS_INTERNAL_H
#define NYAS_PIXELS_INTERNAL_H

#include "nyaspix.h"
#include "pixels.h"
#include "utils/array.h"

#ifdef NYAS_PIXEL_CHECKS
#define CHECK_HANDLE(TYPE, HANDLE) nypx__check_handle((HANDLE), (TYPE##_pool))
#else
#define CHECK_HANDLE(TYPE, HANDLE) (void)
#endif // NYAS_PIXEL_CHECKS

#include "core/log.h" // assert

#define NYAS_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA (8)

static void
nypx__check_handle(int h, void *arr)
{
	NYAS_ASSERT(h >= 0 && "Bad resource state");
	NYAS_ASSERT(h < (nyas_resource_handle)nyas_arr_len(arr) &&
	            "Out of bounds handle");
}

/*enum vtxattr {
	A_POSITION = 0,
	A_NORMAL,
	A_TANGENT,
	A_BITANGENT,
	A_UV,
	VERTEX_ATTRIBUTE_COUNT
};*/

typedef enum resource_flags {
	RF_DIRTY = 1 << 0,
	RF_FREE_AFTER_LOAD = 1 << 1
} resource_flags;

typedef struct resource {
	int id;
	int flags;
} r_resource;

/*typedef struct {
	r_resource res;
	float *vtx;
	nyas_idx *idx;
	unsigned int vtx_size;
	unsigned int elements;
	int attr_flags;
	unsigned int internal_buffers_id[2];
} r_mesh;*/

enum nyas_internal_resource_flags {
	NYAS_IRF_DIRTY = 1U << 3,
	NYAS_IRF_CREATED = 1U << 4,
	NYAS_IRF_RELEASE_RAM_BUFFER = 1U << 5,
};

struct nyas_internal_resource {
	uint32_t id;
	int flags;
};

struct nyas_internal_mesh {
	struct nyas_internal_resource res;
	struct nyas_internal_resource res_vb; // vertex buffer resource
	struct nyas_internal_resource res_ib; // index buffer resource
	float *vtx;
	nypx_index *idx;
	int64_t elem_count;
	uint32_t vtx_size;
	int attrib;
};

struct nyas_internal_texture {
	struct nyas_internal_resource res;
	void *pix[6];
	int width;
	int height;
	int type;
};

struct nyas_internal_shader {
	struct nyas_internal_resource res;
	const char *name;
	struct {
		int data, tex, cubemap;
	} loc[2], count[2]; // 0: unit, 1: common
	void *common;
};

/*typedef struct {
	r_resource res;
	int width;
	int height;
	nyas_tex color_tex;
	nyas_tex depth_tex;
} r_fb;*/

struct nyas_internal_framebuffer {
	struct nyas_internal_resource res;
};

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
