#include "mathc.h"
#include "nyas.h"

typedef struct nyas_pbr_desc_unit {
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
} nyas_pbr_desc_unit;

typedef struct nyas_pbr_desc_scene {
	float view_projection[16];
	float camera_position[3];
	float padding;
	float sunlight[4];
} nyas_pbr_desc_scene;

static const nyas_shader_desc pbr_shader_desc = {
	.name = "pbr",
	.data_count = 7 * 4, // 7 vec4
	.tex_count = 4,
	.cubemap_count = 0,
	.common_data_count = 6 * 4, // 6 vec4
	.common_tex_count = 1,
	.common_cubemap_count = 2
};

static const nyas_shader_desc sky_shader_desc = {
	.name = "skybox",
	.data_count = 0,
	.tex_count = 0,
	.cubemap_count = 0,
	.common_data_count = 4 * 4, // mat4
	.common_tex_count = 0,
	.common_cubemap_count = 1
};

static const nyas_shader_desc fs_img_shader_desc = {
	.name = "fullscreen-img", // fullscreen quad with texture
	.data_count = 0,
	.tex_count = 0,
	.cubemap_count = 0,
	.common_data_count = 0,
	.common_tex_count = 1,
	.common_cubemap_count = 0
};

struct ShaderDescriptors {
	const nyas_shader_desc *pbr;
	const nyas_shader_desc *fullscreen_img;
	const nyas_shader_desc *sky;
};

static const struct ShaderDescriptors g_shader_descriptors = {
	.pbr = &pbr_shader_desc,
	.fullscreen_img = &fs_img_shader_desc,
	.sky = &sky_shader_desc,
};

struct pbr_tex_desc {
	struct nyas_texture_desc albedo;
	struct nyas_texture_desc normal;
	struct nyas_texture_desc pbr_map;
};


static inline nyas_tex
InitMainFramebuffer(nyas_framebuffer fb)
{
	struct nyas_vec2i vp = nyas_window_size();
	struct nyas_texture_desc descriptor =
	  nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_RGB32F, vp.x, vp.y);
	nyas_tex fb_tex = nyas_tex_empty(&descriptor);
	struct nyas_texture_desc depthscriptor =
	  nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_DEPTH, vp.x, vp.y);
	nyas_tex fb_depth = nyas_tex_empty(&depthscriptor);

	nyas_cmd *set_fb_tex = nyas_cmd_alloc();
	set_fb_tex->data.set_fb.fb = fb;
	set_fb_tex->data.set_fb.vp_x = vp.x;
	set_fb_tex->data.set_fb.vp_y = vp.y;
	set_fb_tex->data.set_fb.attach.type = NYAS_SLOT_COLOR0;
	set_fb_tex->data.set_fb.attach.tex = fb_tex;
	set_fb_tex->data.set_fb.attach.mip_level = 0;
	set_fb_tex->data.set_fb.attach.face = -1;
	set_fb_tex->execute = nyas_setfb_fn;

	nyas_cmd *set_fb_depth = nyas_cmd_alloc();
	set_fb_depth->data.set_fb.fb = fb;
	set_fb_depth->data.set_fb.vp_x = vp.x;
	set_fb_depth->data.set_fb.vp_y = vp.y;
	set_fb_depth->data.set_fb.attach.type = NYAS_SLOT_DEPTH;
	set_fb_depth->data.set_fb.attach.tex = fb_depth;
	set_fb_depth->data.set_fb.attach.mip_level = 0;
	set_fb_depth->data.set_fb.attach.face = -1;
	set_fb_depth->execute = nyas_setfb_fn;
	set_fb_depth->next = NULL;

	set_fb_tex->next = set_fb_depth;
	nyas_cmd_add(set_fb_tex);

	return fb_tex;
}
