#ifndef THE_RENDER_MATERIAL_H
#define THE_RENDER_MATERIAL_H

// TODO: Quedarme math internamente y exponer arrays
#include <mathc.h>

struct THE_PbrData {
	float model[16];
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
};

struct THE_PbrSceneData {
	struct mat4 view_projection;
	struct vec3 camera_position;
	float padding;
	struct vec4 light_direction_intensity;
};

struct THE_EquirecToCubeData {
	struct mat4 vp;
};

struct THE_PrefilterEnvData {
	struct mat4 vp;
	float roughness;
	float padding[3];
};


#endif
