#ifndef THE_RENDER_RENDERER_H
#define THE_RENDER_RENDERER_H

#include <stdint.h>
#include <stdbool.h>

#define THE_RENDER_QUEUE_CAPACITY 12000
#define THE_FRAME_POOL_SIZE 1048576
#define THE_MAX_TEXTURES 63
#define THE_MAX_MESHES 32
#define THE_MAX_FRAMEBUFFERS 8
#define THE_MAX_SHADERS 32

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

typedef struct {
	float *data;
	THE_Texture *tex;
	int32_t dcount;
	int32_t tcount;
	int32_t cube_start;
} THE_ShaderData;

typedef THE_ShaderData THE_Material;


extern THE_Mesh SPHERE_MESH;
extern THE_Mesh CUBE_MESH;
extern THE_Mesh QUAD_MESH;

extern struct THE_Camera camera;
extern struct vec4 sunlight;

void THE_InitRender(void);
void THE_RenderFrame(void);
void THE_SubmitFrame(void);
void *THE_AllocateFrameResource(uint32_t size);
int32_t THE_IsInsideFramePool(void *address);

THE_Texture THE_CreateTexture(const char *path, enum THE_TexType t);
THE_Texture THE_CreateEmptyTexture(int32_t width, int32_t height, enum THE_TexType t);
void THE_LoadTexture(THE_Texture tex, const char *path);
void THE_FreeTextureData(THE_Texture tex); // Frees the texture from RAM (not the VRAM)

THE_Mesh THE_CreateCubeMesh(void);
THE_Mesh THE_CreateSphereMesh(int32_t x_segments, int32_t y_segments);
THE_Mesh THE_CreateQuadMesh(void);
THE_Mesh THE_CreateMeshFromFile_OBJ(const char *path); // TODO tinyObjLoader C verison

THE_Shader THE_CreateShader(const char *shader);
THE_ShaderData *THE_ShaderCommonData(THE_Shader);

THE_Material THE_MaterialDefault(void);
void THE_MaterialSetModel(THE_Material *mat, float *data); // This funcion copies a mat4 in the first 64 bytes of the already allocated data.
void THE_MaterialSetData(THE_Material *mat, float *data, int32_t count); // General allocator. It will not free itself
void THE_MaterialSetFrameData(THE_Material *mat, float *data, int32_t count); // Like above but with frame allocator
void THE_MaterialSetTexture(THE_Material *mat, THE_Texture *tex, int32_t count, int32_t cube_start); // General allocator. It will not free itself
void THE_MaterialSetFrameTexture(THE_Material *mat, THE_Texture *tex, int32_t count, int32_t cube_start); // Like above but with frame allocator

THE_Framebuffer THE_CreateFramebuffer(int32_t width, int32_t height, bool color, bool depth);

#endif

