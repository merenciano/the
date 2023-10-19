#ifndef THE_RENDERER_TYPES_H
#define THE_RENDERER_TYPES_H

#include "core/definitions.h"

typedef enum {
	THE_BUFFER_NONE = 0,
	THE_BUFFER_VERTEX_3P,
	THE_BUFFER_VERTEX_3P_3N,
	THE_BUFFER_VERTEX_3P_2UV,
	THE_BUFFER_VERTEX_3P_3N_2UV,
	THE_BUFFER_VERTEX_3P_3N_3T_3B_2UV,
	THE_BUFFER_INDEX,
} THE_BufferType;

typedef enum {
	THE_TEX_NONE = 0,
	THE_TEX_R,
	THE_TEX_RGB,
	THE_TEX_SRGB,
	THE_TEX_DEPTH,
	THE_TEX_SKYBOX,
	THE_TEX_RGB_F16,
	THE_TEX_RGBA_F16,
	THE_TEX_LUT,
	THE_TEX_ENVIRONMENT,
	THE_TEX_PREFILTER_ENVIRONMENT,
} THE_TexType;

typedef int32_t THE_Texture;
typedef int32_t THE_Buffer;
typedef int32_t THE_Framebuffer;
typedef int32_t THE_Shader;

typedef struct {
	THE_Buffer vertex;
	THE_Buffer index;
} THE_Mesh;

typedef struct {
	float *data;
	THE_Texture *tex;
	int32_t dcount;
	int32_t tcount;
	int32_t cube_start;
} THE_ShaderData;

typedef THE_ShaderData THE_Material;

typedef struct {
	struct mat4 view_mat;
	struct mat4 proj_mat;
	float far_value;
	float fov;
	THE_Framebuffer fb;
} THE_Camera;

#endif
