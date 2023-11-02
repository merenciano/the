#ifndef THE_RENDER_INTERNAL_RESOURCES_H
#define THE_RENDER_INTERNAL_RESOURCES_H

#include "renderer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef THE_RENDER_CHECKS
#define THE__CHECK_HANDLE(TYPE, HANDLE)                                       \
	({                                                                        \
		THE_ASSERT(HANDLE >= 0, "Bad resource state");                        \
		THE_ASSERT(HANDLE < TYPE##_count, "Out of bounds handle");            \
	})

#define THE__CHECK_INTERNAL(RESOURCE)                                         \
	({                                                                        \
		THE_ASSERT(RESOURCE && RESOURCE->internal_id >= 0,                    \
		  "Invalid internal resource.");                                      \
	})

#endif // THE_RENDER_CHECKS

enum THE_VertexAttributes {
	A_POSITION = 0,
	A_NORMAL,
	A_TANGENT,
	A_BITANGENT,
	A_UV,
	VERTEX_ATTRIBUTE_COUNT
};

typedef enum THE_ResourceFlags { RF_DIRTY = 1 << 0 } THE_ResourceFlags;

typedef struct THE_InternalResource {
	int id;
	int flags;
} THE_InternalResource;

typedef struct {
	float *vtx;
	IDX_T *idx;
	unsigned vtx_size;
	unsigned elements;
	int attr_flags;
	int internal_id;
	uint32_t internal_buffers_id[2];
} THE_InternalMesh;

typedef struct {
	char path[64];
	void *pix[6];
	int internal_id;
	int cpu_version;
	int gpu_version;
	int texture_unit;
	int width;
	int height;
	int type;
} THE_InternalTexture;

typedef struct {
	THE_InternalResource res;
	int internal_id;
	int cpu_version;
	int gpu_version;
	int width;
	int height;
	THE_Texture color_tex;
	THE_Texture depth_tex;
} THE_InternalFramebuffer;

enum THE_DataGroup { THE_SHADER_COMMON_DATA = 0, THE_SHADER_DATA = 1 };

typedef struct {
	int data;
	int tex;
	int cubemap;
} THE_DataLocations;

typedef struct {
	const char *shader_name;
	int program_id;
	THE_DataLocations data_loc[2];
} THE_InternalShader;

extern THE_InternalMesh *meshes;
extern THE_InternalTexture *textures;
extern THE_InternalFramebuffer *framebuffers;
extern THE_InternalShader *shaders;
extern int mesh_count;
extern int texture_count;
extern int framebuffer_count;
extern int shader_count;

#endif
