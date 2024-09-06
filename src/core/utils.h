#ifndef NYAS_UTILS_H
#define NYAS_UTILS_H

#include "core/common.h"
#include "core/sched.h"
#include "render/pixels.h"

// Default values
static inline struct nyas_texture_desc
nyut_texture_desc_default(nyas_texture_type type, nyas_texture_format fmt, int w, int h)
{
	return (struct nyas_texture_desc){
		.flags = NYAS_TEX_FLAG_DEFAULT,
		.type = type,
		.width = w,
		.height = h,
		.fmt = fmt,
		.min_filter = NYAS_TEX_FLTR_LINEAR,
		.mag_filter = NYAS_TEX_FLTR_LINEAR,
		.wrap_s = NYAS_TEX_WRAP_REPEAT,
		.wrap_t = NYAS_TEX_WRAP_REPEAT,
		.wrap_r = NYAS_TEX_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

static inline struct nyas_render_state
nyut_draw_state_default(void)
{
	return (struct nyas_render_state){
		.target = { .fb = NYAS_NOOP, .bgcolor = (struct nyas_color){ 0.0f, 0.0f, 0.0f, 1.0f } },
		.pipeline = { .shader_mat = { .shader = NYAS_NOOP, .ptr = NULL } },
		.ops = { .viewport = (struct nyas_rect){ 0, 0, 0, 0 },
		         .scissor = (struct nyas_rect){ 0, 0, 0, 0 },
		         .enable = 0,
		         .disable = 0,
		         .depth_fun = NYAS_DEPTH_CURRENT,
		         .blend_src = NYAS_BLEND_CURRENT,
		         .blend_dst = NYAS_BLEND_CURRENT,
		         .cull_face = NYAS_CULL_CURRENT }
	};
}

static inline struct nyas_draw
nyut_draw_default(void)
{
	return (struct nyas_draw){ .state = nyut_draw_state_default(), .cmds = NULL };
}

// Asset loader
struct nyut_tex_ldargs {
	struct nyas_texture_desc desc;
	const char *path;
	nyas_tex tex;
};

struct nyut_mesh_ldargs {
	const char *path;
	nyas_mesh *mesh;
};

struct nyut_shad_ldargs {
	struct nyas_shader_desc desc;
	nyas_shader *shader;
};

struct nyut_env_ldargs {
	const char *path;
	nyas_tex *sky;
	nyas_tex *irr;
	nyas_tex *pref;
	nyas_tex *lut;
};

typedef struct nyut_asset_loader nyut_assetldr;
nyut_assetldr *nyut_assets_create(void);
void nyut_assets_add_mesh(nyut_assetldr *l, struct nyut_mesh_ldargs *args);
void nyut_assets_add_tex(nyut_assetldr *l, struct nyut_tex_ldargs *args);
void nyut_assets_add_shader(nyut_assetldr *l, struct nyut_shad_ldargs *args);
void nyut_assets_add_env(nyut_assetldr *l, struct nyut_env_ldargs *args);
void nyut_assets_add_job(nyut_assetldr *l, struct nyas_job j, bool async);
void nyut_assets_load(nyut_assetldr *l, int threads);

// Geometry
enum nyut_geometry { NYAS_QUAD, NYAS_CUBE, NYAS_SPHERE };

extern nyas_mesh NYAS_UTILS_SPHERE;
extern nyas_mesh NYAS_UTILS_CUBE;
extern nyas_mesh NYAS_UTILS_QUAD;

void nyut_mesh_init_geometry(void);
void nyut_mesh_set_geometry(nyas_mesh mesh, enum nyut_geometry geo);

// Environment maps
void nyut_env_load(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref);

#endif // NYAS_UTILS_H
