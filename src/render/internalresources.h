#ifndef __THE_RENDER_INTERNAL_RESOURCES_H__
#define __THE_RENDER_INTERNAL_RESOURCES_H__

#include "core/definitions.h"
#include "renderertypes.h"

typedef enum {
	A_POSITION = 0,
	A_NORMAL,
	A_TANGENT,
	A_BITANGENT,
	A_UV,
	VERTEX_ATTRIBUTE_COUNT
} THE_VertexAttributes;

typedef struct {
	union {
		float *vertices;
		uint32_t *indices;
	};
	int32_t count;
	uint32_t internal_id;
	int32_t cpu_version;
	int32_t gpu_version;
	THE_BufferType type;
} THE_InternalBuffer;

typedef struct {
	float *vtx;
	uint32_t *idx;
	size_t vtx_size;
	size_t elements;
	int32_t attr_flags;
	int32_t internal_id;
	uint32_t internal_buffers_id[2];
} THE_InternalMesh;

typedef struct {
	char path[64];
	void *pix;
	int32_t internal_id;
	int32_t cpu_version;
	int32_t gpu_version;
	uint32_t texture_unit;
	int32_t width;
	int32_t height;
	THE_TexType type;
} THE_InternalTexture;

typedef struct {
	int32_t internal_id;
	int32_t cpu_version;
	int32_t gpu_version;
	int32_t width;
	int32_t height;
	THE_Texture color_tex;
	THE_Texture depth_tex;
} THE_InternalFramebuffer;

typedef struct {
	int32_t data;
	int32_t tex;
	int32_t cubemap;
} THE_DataLocations;

typedef struct {
	const char *shader_name;
	int32_t program_id;
	THE_ShaderData common_data;
	THE_DataLocations data_loc[2];
} THE_InternalShader;

extern THE_InternalBuffer *buffers;
extern THE_InternalMesh *meshes;
extern THE_InternalTexture *textures;
extern THE_InternalFramebuffer *framebuffers;
extern THE_InternalShader *shaders;
extern size_t buffer_count;
extern size_t mesh_count;
extern size_t texture_count;
extern size_t framebuffer_count;
extern size_t shader_count;

bool IsValidBuffer(THE_Buffer buff);
bool IsValidTexture(THE_Texture tex);
bool IsValidFramebuffer(THE_Framebuffer fb);

#endif
