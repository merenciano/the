#ifndef THE_UTILS_H
#define THE_UTILS_H

#include "core/common.h"
#include "core/sched.h"
#include "render/pixels.h"

// Default values
static inline struct the_texture_desc
tut_texture_desc_default(the_texture_type type, the_texture_format fmt, int w, int h)
{
	return (struct the_texture_desc){
		.flags = THE_TEX_FLAG_DEFAULT,
		.type = type,
		.width = w,
		.height = h,
		.fmt = fmt,
		.min_filter = THE_TEX_FLTR_LINEAR,
		.mag_filter = THE_TEX_FLTR_LINEAR,
		.wrap_s = THE_TEX_WRAP_REPEAT,
		.wrap_t = THE_TEX_WRAP_REPEAT,
		.wrap_r = THE_TEX_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

static inline struct the_render_state
tut_draw_state_default(void)
{
	return (struct the_render_state){
		.target = { .fb = THE_NOOP, .bgcolor = (struct the_color){ 0.0f, 0.0f, 0.0f, 1.0f } },
		.pipeline = { .shader_mat = { .shader = THE_NOOP, .ptr = NULL } },
		.ops = { .viewport = (struct the_rect){ 0, 0, 0, 0 },
		         .scissor = (struct the_rect){ 0, 0, 0, 0 },
		         .enable = 0,
		         .disable = 0,
		         .depth_fun = THE_DEPTH_CURRENT,
		         .blend_src = THE_BLEND_CURRENT,
		         .blend_dst = THE_BLEND_CURRENT,
		         .cull_face = THE_CULL_CURRENT }
	};
}

static inline struct the_draw
tut_draw_default(void)
{
	return (struct the_draw){ .state = tut_draw_state_default(), .cmds = NULL };
}

// Asset loader
struct tut_tex_ldargs {
	struct the_texture_desc desc;
	const char *path;
	the_tex tex;
};

struct tut_mesh_ldargs {
	const char *path;
	the_mesh *mesh;
};

struct tut_shad_ldargs {
	struct the_shader_desc desc;
	the_shader *shader;
};

struct tut_env_ldargs {
	const char *path;
	the_tex *sky;
	the_tex *irr;
	the_tex *pref;
	the_tex *lut;
};

typedef struct tut_asset_loader tut_assetldr;
tut_assetldr *tut_assets_create(void);
void tut_assets_add_mesh(tut_assetldr *l, struct tut_mesh_ldargs *args);
void tut_assets_add_tex(tut_assetldr *l, struct tut_tex_ldargs *args);
void tut_assets_add_shader(tut_assetldr *l, struct tut_shad_ldargs *args);
void tut_assets_add_env(tut_assetldr *l, struct tut_env_ldargs *args);
void tut_assets_add_job(tut_assetldr *l, struct the_job j, bool async);
void tut_assets_load(tut_assetldr *l, int threads);

// Geometry
enum tut_geometry { THE_QUAD, THE_CUBE, THE_SPHERE };

extern the_mesh THE_UTILS_SPHERE;
extern the_mesh THE_UTILS_CUBE;
extern the_mesh THE_UTILS_QUAD;

void tut_mesh_init_geometry(void);
void tut_mesh_set_geometry(the_mesh mesh, enum tut_geometry geo);

// Environment maps
void tut_env_load(const char *path, the_tex *lut, the_tex *sky, the_tex *irr, the_tex *pref);

#endif // THE_UTILS_H
