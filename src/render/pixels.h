#ifndef NYAS_PIXELS_H
#define NYAS_PIXELS_H

#include "core/nyas_core.h"
#include "pixels_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * nyaspix config
 */

#ifdef NYAS_ELEM_SIZE_16
typedef uint16_t nyas_idx;
#else
typedef uint32_t nyas_idx;
#endif

#define NYAS_RENDER_QUEUE_CAPACITY 1024
#define NYAS_FRAME_POOL_SIZE (16 * 1024 * 1024)
#define NYAS_TEX_RESERVE 64
#define NYAS_MESH_RESERVE 64
#define NYAS_FB_RESERVE 32
#define NYAS_SHADER_RESERVE 32

/*
 * nyaspix resources
 */

union nyas_mat_value {
	float data;
	nyas_tex sampler;
};

typedef struct nyas_mat {
	void *ptr; // TODO deprecar
	nyas_shader shader;
	union nyas_material_value *data;
} nyas_mat;

typedef struct nyas_shader_desc {
	const char *name;
	int data_count;
	int tex_count;
	int cubemap_count;
	int common_data_count;
	int common_tex_count;
	int common_cubemap_count;
} nyas_shader_desc;

enum nyas_geometry {
	NYAS_QUAD,
	NYAS_CUBE,
	NYAS_SPHERE
};

void nyas_px_init(void);
void nyas_px_render(void);
void nyas_frame_end(void);
void *nyas_alloc_frame(unsigned int size);

struct nyas_texture_desc nyas_tex_defined_desc(nyas_texture_type type, nyas_texture_format fmt, int w, int h);
struct nyas_texture_desc nyas_tex_desc(nyas_tex tex);
nyas_tex nyas_tex_alloc(int count);
void nyas_tex_set(nyas_tex tex, struct nyas_texture_desc *desc);
void nyas_tex_load(nyas_tex tex, struct nyas_texture_desc *desc, const char *path);
struct nyas_vec2i nyas_tex_size(nyas_tex tex);
void nyas_load_env(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref);

nyas_framebuffer nyas_fb_create(void);

nyas_mesh nyas_mesh_load_file(const char *path);
nyas_mesh nyas_mesh_load_geometry(enum nyas_geometry geo);
void nyas_mesh_reload_file(nyas_mesh mesh, const char *path);
void nyas_mesh_reload_geometry(nyas_mesh mesh, enum nyas_geometry geo);

nyas_shader nyas_shader_create(const nyas_shader_desc *desc);
void *nyas_shader_data(nyas_shader shader);
nyas_tex *nyas_shader_tex(nyas_shader shader);
nyas_tex *nyas_shader_cubemap(nyas_shader shader);
void nyas_shader_reload(nyas_shader shader);

/* Creates a new material and alloc persistent memory for its data */
nyas_mat nyas_mat_pers(nyas_shader shader);
/* Creates a new material and alloc frame-scoped memory for its data */
nyas_mat nyas_mat_tmp(nyas_shader shader);
nyas_mat nyas_mat_copy(nyas_mat mat);
nyas_mat nyas_mat_copy_shader(nyas_shader shader);

nyas_tex *nyas_mat_tex(nyas_mat mat); // Ptr to first texture.
nyas_tex *nyas_mat_cubemap(nyas_mat mat); // Ptr to first cubemap.

extern nyas_mesh SPHERE_MESH;
extern nyas_mesh CUBE_MESH;
extern nyas_mesh QUAD_MESH;

/*
 * nyaspix commands
 */

typedef struct nyas_clear_cmdata {
	float color[4];
	bool color_buffer;
	bool depth_buffer;
	bool stencil_buffer;
} nyas_clear_cmdata;

typedef struct nyas_draw_cmdata {
	nyas_mesh mesh;
	nyas_mat material;
} nyas_draw_cmdata;

typedef enum nyas_rops_opt {
	NYAS_ROPS_NONE = 0,
	NYAS_BLEND = 1 << 0,
	NYAS_CULL_FACE = 1 << 1,
	NYAS_DEPTH_TEST = 1 << 2,
	NYAS_DEPTH_WRITE = 1 << 3,
	NYAS_REND_OPTS_COUNT
} nyas_rops_opt;

typedef struct nyas_rops_cmdata {
	int enable_flags;
	int disable_flags;
	enum nyas_blend_func blend_src;
	enum nyas_blend_func blend_dst;
	enum nyas_cull_face cull_face;
	enum nyas_depth_func depth_func;
} nyas_rops_cmdata;

typedef struct nyas_fb_slot {
	nyas_tex tex;
	int mip_level;
	int type;
	int face;
} nyas_fb_slot;

typedef struct nyas_set_fb_cmdata {
	nyas_framebuffer fb;
	int vp_x;
	int vp_y;
	nyas_fb_slot attach;
} nyas_set_fb_cmdata;

typedef union nyas_cmdata {
	nyas_clear_cmdata clear;
	nyas_draw_cmdata draw;
	nyas_rops_cmdata rend_opts;
	nyas_mat mat;
	nyas_set_fb_cmdata set_fb;
} nyas_cmdata;

typedef struct nyas_cmd {
	struct nyas_cmd *next;
	void (*execute)(nyas_cmdata *data);
	nyas_cmdata data;
} nyas_cmd;

void nyas_cmd_add(nyas_cmd *rc);
nyas_cmd *nyas_cmd_alloc(void);

extern void nyas_clear_fn(nyas_cmdata *data);
extern void nyas_draw_fn(nyas_cmdata *data);
extern void nyas_rops_fn(nyas_cmdata *data);
extern void nyas_setshader_fn(nyas_cmdata *data);
extern void nyas_setfb_fn(nyas_cmdata *data);

typedef int nyas_draw_flags;
enum nyas_draw_flags {
	NYAS_DRAW_CLEAR_COLOR = 0,
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
	float bgcolor[4];
};

struct nyas_draw_pipeline {
	nyas_shader shader;
	int vtx_attrib;
};

struct nyas_draw_ops {
	struct nyas_rect viewport[4];
	struct nyas_rect scissor[4];
	nyas_draw_flags flags;
	uint8_t blend_src;
	uint8_t blend_dst;
	uint8_t cull_face;
	uint8_t depth_fun;
	uint8_t stencil_fun;
};

struct nyas_draw_cmd {
	nyas_mesh mesh;
	nyas_mat material;
};

struct nyas_draw_list {
	struct nyas_draw_target target;
	struct nyas_draw_pipeline pipeline;
	struct nyas_draw_ops ops;
	struct nyas_draw_cmd *cmds;
};

struct nyas_draw_frame {
	struct nyas_draw_list *draw_lists; // nyas_arr
	// post process
	uint8_t *mem_arena;
};

#endif // NYAS_PIXELS_H
