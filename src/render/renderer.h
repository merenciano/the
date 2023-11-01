#ifndef THE_RENDER_RENDERER_H
#define THE_RENDER_RENDERER_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

struct THE_PbrData {
	float model[16];
	float color[3];
	float use_albedo_map;
	float use_pbr_maps;
	float tiling_x;
	float tiling_y;
	float padding;
	float roughness;
	float metallic;
	float normal_map_intensity;
	float paddingg;
};

struct THE_PbrSceneData {
	float view_projection[16];
	float camera_position[3];
	float padding;
	float sunlight[4];
};

struct THE_EquirecToCubeData {
	float vp[16];
};

struct THE_PrefilterEnvData {
	float vp[16];
	float roughness;
	float padding[3];
};

enum THE_TexType {
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
};

typedef int32_t THE_Mesh;
typedef int32_t THE_Texture;
typedef int32_t THE_Framebuffer;
typedef int32_t THE_Shader;

typedef struct THE_Mat {
	void *ptr;
	int data_count;
	int tex_count;
	int cube_count;
	THE_Shader shader;
} THE_Mat;

extern THE_Mesh SPHERE_MESH;
extern THE_Mesh CUBE_MESH;
extern THE_Mesh QUAD_MESH;

void THE_InitRender(void);
void THE_RenderFrame(void);
void THE_RenderEndFrame(void);
void *THE_AllocateFrameResource(uint32_t size);

THE_Texture THE_CreateTextureFromFile(const char *path, enum THE_TexType t);
THE_Texture THE_CreateEmptyTexture(int32_t width, int32_t height, enum THE_TexType t);
void THE_FreeTextureData(THE_Texture tex); // Frees the texture from RAM (not the VRAM)

THE_Mesh THE_CreateCubeMesh(void);
THE_Mesh THE_CreateSphereMesh(int32_t x_segments, int32_t y_segments);
THE_Mesh THE_CreateQuadMesh(void);
THE_Mesh THE_CreateMeshFromFile_OBJ(const char *path); // TODO tinyObjLoader C verison

THE_Shader THE_CreateShader(const char *shader);
THE_Framebuffer THE_CreateFramebuffer(int32_t width, int32_t height, bool color, bool depth);
THE_Texture THE_GetFrameColor(THE_Framebuffer fb);
void THE_FbDimensions(THE_Framebuffer, int *w, int *h);

THE_Mat THE_MatDefault(void);
/* MatAlloc does not initialize the shader value. */
void *THE_MatAlloc(THE_Mat *);

/* MatAllocFrame does not initialize the shader value. */
void *THE_MatAllocFrame(THE_Mat *);

#endif

