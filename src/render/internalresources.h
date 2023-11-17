#ifndef NYAS_PIXEL_INTERNAL_RESOURCES_H
#define NYAS_PIXEL_INTERNAL_RESOURCES_H

#include "renderer.h"

#ifdef NYAS_PIXEL_CHECKS
#define NYAS__CHECK_HANDLE(TYPE, HANDLE)                            \
	({                                                              \
		NYAS_ASSERT(HANDLE >= 0 && "Bad resource state");             \
		NYAS_ASSERT(HANDLE < TYPE##_count && "Out of bounds handle"); \
	})
#endif // NYAS_PIXEL_CHECKS

enum nypx_vtxattr {
	A_POSITION = 0,
	A_NORMAL,
	A_TANGENT,
	A_BITANGENT,
	A_UV,
	VERTEX_ATTRIBUTE_COUNT
};

typedef enum nypx_rsrc_flags {
	RF_DIRTY = 1 << 0,
	RF_FREE_AFTER_LOAD = 1 << 1
} nypx_rsrc_flags;

typedef struct nypx_irsrc {
	int id;
	int flags;
} nypx_irsrc;

typedef struct {
	nypx_irsrc res;
	float *vtx;
	IDX_T *idx;
	unsigned int vtx_size;
	unsigned int elements;
	int attr_flags;
	unsigned int internal_buffers_id[2];
} nypx_imesh;

typedef struct {
	nypx_irsrc res;
	void *pix[6];
	int width;
	int height;
	int type;
} nypx_itex;

typedef struct {
	nypx_irsrc res;
	int width;
	int height;
	nyas_tex color_tex;
	nyas_tex depth_tex;
} nypx_ifb;

enum nypx_shdata_type {
	NYAS_SHADER_COMMON_DATA = 0,
	NYAS_SHADER_DATA = 1
};

typedef struct {
	int data;
	int tex;
	int cubemap;
} nypx_shdata_loc;

typedef struct {
	nypx_irsrc res;
	const char *shader_name;
	nypx_shdata_loc data_loc[2];
} nypx_ishd;

bool nypx_resource_check(void *rsrc);

extern nypx_imesh *meshes;
extern nypx_itex *textures;
extern nypx_ifb *framebuffers;
extern nypx_ishd *shaders;
extern int mesh_count;
extern int texture_count;
extern int framebuffer_count;
extern int shader_count;

#endif // NYAS_PIXEL_INTERNAL_RESOURCES_H
