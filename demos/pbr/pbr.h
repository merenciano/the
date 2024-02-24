#include "nyas.h"

struct pbr_desc_unit {
	float model[16];
	float color[3];
	float use_albedo_map;
	float use_pbr_maps;
	float tiling_x;
	float tiling_y;
	float reflectance;
	float roughness;
	float metallic;
	float normal_map_intensity;
	float paddingg;
};

struct pbr_desc_scene {
	float view_projection[16];
	struct nyas_vec3 camera_position;
	float padding;
	float sunlight[4];
};

struct pbr_maps {
	nyas_tex a, n, r, m;
};

static const struct {
	const nyas_shader_desc pbr;
	const nyas_shader_desc fullscreen_img;
	const nyas_shader_desc sky;
} g_shader_descriptors = {
	.pbr = {
		.name = "pbr",
		.data_count = 7 * 4, // 7 vec4
		.tex_count = 4,
		.cubemap_count = 0,
		.shared_data_count = 6 * 4, // 6 vec4
		.common_tex_count = 1,
		.common_cubemap_count = 2
	},
	.fullscreen_img = {
		.name = "fullscreen-img", // fullscreen quad with texture
		.data_count = 0,
		.tex_count = 0,
		.cubemap_count = 0,
		.shared_data_count = 0,
		.common_tex_count = 1,
		.common_cubemap_count = 0
	},
	.sky = {
		.name = "skybox",
		.data_count = 0,
		.tex_count = 0,
		.cubemap_count = 0,
		.shared_data_count = 4 * 4, // mat4
		.common_tex_count = 0,
		.common_cubemap_count = 1
	}
};
