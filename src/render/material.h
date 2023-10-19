#ifndef THE_RENDER_MATERIAL_H
#define THE_RENDER_MATERIAL_H

#include "core/definitions.h"
#include "render/renderertypes.h"

typedef struct {
	float model_[16];
	struct vec3 color;
	float use_albedo_map;
	float use_pbr_maps;
	float tiling_x;
	float tiling_y;
	float padding;
	float roughness;
	float metallic;
	float normal_map_intensity;
	float paddingg;
} THE_PbrData;

typedef struct {
	struct mat4 view_projection;
	struct vec3 camera_position;
	float padding;
	struct vec4 light_direction_intensity;
} THE_PbrSceneData;

typedef struct {
	struct mat4 vp;
} THE_EquirecToCubeData;

typedef struct {
	struct mat4 vp;
	float roughness;
	float padding[3];
} THE_PrefilterEnvData;

THE_Shader THE_CreateShader(const char *shader);
THE_Material THE_MaterialDefault(void);

void THE_MaterialSetModel(THE_Material *mat, float *data); // This funcion copies a mat4 in the first 64 bytes of the already allocated data.
void THE_MaterialSetData(THE_Material *mat, float *data, int32_t count); // General allocator. It will not free itself
void THE_MaterialSetFrameData(THE_Material *mat, float *data, int32_t count); // Like above but with frame allocator
void THE_MaterialSetTexture(THE_Material *mat, THE_Texture *tex, int32_t count, int32_t cube_start); // General allocator. It will not free itself
void THE_MaterialSetFrameTexture(THE_Material *mat, THE_Texture *tex, int32_t count, int32_t cube_start); // Like above but with frame allocator

#endif
