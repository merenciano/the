#ifndef NYAS_PIXELS_EXTRA_H
#define NYAS_PIXELS_EXTRA_H

typedef struct nyas_pbr_desc_unit {
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
} nyas_pbr_desc_unit;

typedef struct nyas_pbr_desc_scene {
	float view_projection[16];
	float camera_position[3];
	float padding;
	float sunlight[4];
} nyas_pbr_desc_scene;

#endif // NYAS_PIXELS_EXTRA_H
