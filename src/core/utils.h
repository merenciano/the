#ifndef NYAS_UTILS_H
#define NYAS_UTILS_H

#include "core/nyas_core.h"
#include "core/sched.h"
#include "render/pixels.h"

// Default values
static inline struct nyas_texture_desc
nyutil_tex_defined_desc(nyas_texture_type type, nyas_texture_format fmt, int w, int h)
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

static inline void
nyutil_draw_state_default(struct nyas_render_state *rs)
{
	rs->target.fb = NYAS_NOOP;
	rs->target.bgcolor = (struct nyas_color){0.0f, 0.0f, 0.0f, 1.0f};
	rs->pipeline.shader = NYAS_NOOP;
	rs->pipeline.shared_data.ptr = NULL;
	rs->ops.viewport = (struct nyas_rect){0, 0, 0, 0};
	rs->ops.scissor = (struct nyas_rect){0, 0, 0, 0};
	rs->ops.enable = 0;
	rs->ops.disable = 0;
	rs->ops.depth_fun = NYAS_DEPTH_CURRENT;
	rs->ops.blend_src = NYAS_BLEND_CURRENT;
	rs->ops.blend_dst = NYAS_BLEND_CURRENT;
	rs->ops.cull_face = NYAS_CULL_CURRENT;
}

// Asset loader
struct nyutil_tex_ldargs {
	struct nyas_texture_desc desc;
	const char *path;
	nyas_tex tex;
};

struct nyutil_mesh_ldargs {
	const char *path;
	nyas_mesh *mesh;
};

struct nyutil_shad_ldargs {
	struct nyas_shader_desc desc;
	nyas_shader *shader;
};

struct nyutil_env_ldargs {
	const char *path;
	nyas_tex *sky;
	nyas_tex *irr;
	nyas_tex *pref;
	nyas_tex *lut;
};

typedef struct nyutil_asset_loader nyut_assetldr;
nyut_assetldr *nyutil_assets_create(void);
void nyutil_assets_add_mesh(nyut_assetldr *l, struct nyutil_mesh_ldargs *args);
void nyutil_assets_add_tex(nyut_assetldr *l, struct nyutil_tex_ldargs *args);
void nyutil_assets_add_shader(nyut_assetldr *l, struct nyutil_shad_ldargs *args);
void nyutil_assets_add_env(nyut_assetldr *l, struct nyutil_env_ldargs *args);
void nyutil_assets_add_job(nyut_assetldr *l, struct nyas_job j, bool async);
void nyutil_assets_load(nyut_assetldr *l, int threads);

// Geometry
enum nyutil_geometry {
	NYAS_QUAD,
	NYAS_CUBE,
	NYAS_SPHERE
};

extern nyas_mesh SPHERE_MESH;
extern nyas_mesh CUBE_MESH;
extern nyas_mesh QUAD_MESH;

void nyutil_mesh_init_geometry(void);
void nyutil_mesh_set_geometry(nyas_mesh mesh, enum nyutil_geometry geo);

// Environment maps
void
nyutil_env_load(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref);

#endif // NYAS_UTILS_H
