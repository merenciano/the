#ifndef __THE_RENDER_INTERNAL_RESOURCES_H__
#define __THE_RENDER_INTERNAL_RESOURCES_H__

#include "core/definitions.h"
#include "renderertypes.h"

typedef uint32_t THE_InternalMaterial; // OpenGL program (compiled shaders)

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

extern THE_InternalBuffer *buffers;
extern THE_InternalTexture *textures;
extern THE_InternalFramebuffer *framebuffers;
extern THE_InternalMaterial *materials;
extern size_t buffer_count;
extern size_t texture_count;
extern size_t framebuffer_count;
extern size_t material_count;

bool IsValidBuffer(THE_Buffer buff);
bool IsValidTexture(THE_Texture tex);
bool IsValidFramebuffer(THE_Framebuffer fb);

uint32_t InitInternalMaterial(const char *shader_name);

#endif
