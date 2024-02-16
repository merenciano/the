#ifndef NYAS_PIXELS_H
#define NYAS_PIXELS_H

#include "core/nyas_core.h"
#include "pixels_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef NYAS_ELEM_SIZE_16
typedef uint16_t nyas_idx;
#else
typedef uint32_t nyas_idx;
#endif

#define NYAS_TEX_RESERVE 64
#define NYAS_MESH_RESERVE 64
#define NYAS_FB_RESERVE 32
#define NYAS_SHADER_RESERVE 32

typedef int nyas_draw_flags;
enum nyas_draw_flags {
	NYAS_DRAW_CLEAR_COLOR,
	NYAS_DRAW_CLEAR_DEPTH,
	NYAS_DRAW_CLEAR_STENCIL,
	NYAS_DRAW_TEST_DEPTH,
	NYAS_DRAW_TEST_STENCIL,
	NYAS_DRAW_WRITE_DEPTH,
	NYAS_DRAW_WRITE_STENCIL,
	NYAS_DRAW_BLEND,
	NYAS_DRAW_CULL,
	NYAS_DRAW_SCISSOR,
	NYAS_DRAW_FLAGS_COUNT
};

struct nyas_draw_target {
	nyas_framebuffer fb;
	struct nyas_color bgcolor;
};

struct nyas_draw_pipeline {
	nyas_shader shader;
	nyas_mat shared_data;
	nyas_vertex_attrib va;
};

struct nyas_draw_ops {
	struct nyas_rect viewport;
	struct nyas_rect scissor;
	nyas_draw_flags enable;
	nyas_draw_flags disable;
	uint8_t blend_src;
	uint8_t blend_dst;
	uint8_t cull_face;
	uint8_t depth_fun;
};

struct nyas_draw_cmd {
	nyas_mesh mesh;
	nyas_mat material;
};

struct nyas_render_state {
	struct nyas_draw_target target;
	struct nyas_draw_pipeline pipeline;
	struct nyas_draw_ops ops;
};

struct nyas_drawlist {
	struct nyas_render_state state;
	struct nyas_draw_cmd *cmds; // nyas_arr
};

struct nyas_frame_ctx {
	struct nyas_drawlist *draw_lists; // nyas_arr
};

// Helpers
enum nyas_geometry {
	NYAS_QUAD,
	NYAS_CUBE,
	NYAS_SPHERE
};

extern nyas_mesh SPHERE_MESH;
extern nyas_mesh CUBE_MESH;
extern nyas_mesh QUAD_MESH;

struct nyas_texture_desc nyas_tex_defined_desc(nyas_texture_type type, nyas_texture_format fmt, int w, int h);
void nyas_load_env(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref);
// --Helpers

void nyas_px_init(void);

nyas_tex nyas_tex_create(int count);
void nyas_tex_set(nyas_tex tex, struct nyas_texture_desc *desc);
void nyas_tex_load(nyas_tex tex, struct nyas_texture_desc *desc, const char *path);
// TODO(Renderer): Cambiar viejos vec por nyas_core
struct nyas_point nyas_tex_size(nyas_tex tex);

nyas_framebuffer nyas_fb_create(void);
void nyas_fb_set_target(nyas_framebuffer fb, int index, struct nyas_texture_target target);

// TODO(Renderer): Unificar load y reload
nyas_mesh nyas_mesh_load_file(const char *path);
nyas_mesh nyas_mesh_load_geometry(enum nyas_geometry geo);
void nyas_mesh_reload_file(nyas_mesh mesh, const char *path);
void nyas_mesh_reload_geometry(nyas_mesh mesh, enum nyas_geometry geo);

nyas_shader nyas_shader_create(const nyas_shader_desc *desc);
void *nyas_shader_data(nyas_shader shader);
void nyas_shader_reload(nyas_shader shader);
nyas_tex *nyas_shader_tex(nyas_shader shader);
nyas_tex *nyas_shader_cubemap(nyas_shader shader);

/* Creates a new material and alloc persistent memory for its data */
nyas_mat nyas_mat_create(nyas_shader shader);
/* Creates a new material and alloc frame-scoped memory for its data */
nyas_mat nyas_mat_tmp(nyas_shader shader);
nyas_mat nyas_mat_copy(nyas_mat mat);
nyas_mat nyas_mat_copy_shader(nyas_shader shader);
nyas_tex *nyas_mat_tex(nyas_mat mat); // Ptr to first texture.

void nyas_draw_op_enable(struct nyas_draw_ops *ops, nyas_draw_flags op);
void nyas_draw_op_disable(struct nyas_draw_ops *ops, nyas_draw_flags op);
void *nyas_frame_alloc(ptrdiff_t size); // Circular buffer (fixed size, not freeing)
void nyas_frame_render(struct nyas_frame_ctx *frame);

static inline void nyas_draw_state_default(struct nyas_render_state *rs)
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

#endif // NYAS_PIXELS_H
