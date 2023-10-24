#ifndef THE_RENDER_MATERIAL_H
#define THE_RENDER_MATERIAL_H

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

#endif
