#ifndef NYAS_DEFS_H
#define NYAS_DEFS_H

#include "core/nyas_core.h"

#include <stddef.h>
#include <stdint.h>

#include <stddef.h>

#if 0

typedef struct nyas_resource {
	void *internal;
	nyas_flags flags;
} nyas_resource;

typedef struct nyas_texture_data {
	void *pixel_buffer;
	ptrdiff_t pixel_buffer_size;
	int lod;        /* level of detail */
	int index;      /* texture arrays only */
	nyas_enum face; /* cubemaps only */
} nyas_texture_data;

typedef struct nyas_texture {
	nyas_texture_data *images;
	int img_count;
	int width;
	int height;
	nyas_enum type;       /* NYAS_TEXTURE_TYPE */
	nyas_enum format;     /* NYAS_TEXTURE_FORMAT */
	nyas_enum min_filter; /* NYAS_TEXTURE_FILTER */
	nyas_enum mag_filter; /* NYAS_TEXTURE_FILTER */
	nyas_enum wrap_s;     /* NYAS_TEXTURE_WRAP */
	nyas_enum wrap_t;     /* NYAS_TEXTURE_WRAP */
	nyas_enum wrap_r;     /* NYAS_TEXTURE_WRAP */
} nyas_texture;

typedef struct nyas_mesh {
	float *vertices;
	void *indices;
	ptrdiff_t vtx_count;
	ptrdiff_t idx_count;
	int indices_sizeof; /* 2 or 4 */
	nyas_flags vertex_attributes;
} nyas_mesh;

typedef struct nyas_material_data {
	float *values;
	nyas_texture *textures;
	nyas_texture *cubemaps;
} nyas_material_data;

typedef struct nyas_shader {
	nyas_material_data shared_data; /* Read-only data for material instances. */
	int shared_values_location;
	int shared_tex_location;
	int shared_cubemap_location;
	int shared_values_count;
	int shared_tex_count;
	int shared_cubemap_count;
	int values_location;
	int tex_location;
	int cubemap_location;
	int values_count;
	int tex_count;
	int cubemap_count;
	nyas_flags vertex_attributes; /* ?? NYAS_FLAG_VA (see: struct nyas_mesh) */
} nyas_shader;

typedef struct nyas_material {
	nyas_material_data data;
	nyas_shader	shader;
} nyas_material;

#define NYAS_CONFIG_FRAMEBUFFER_COLOR_TEXTURES 6
typedef struct nyas_framebuffer {
	nyas_texture textures[NYAS_CONFIG_FRAMEBUFFER_COLOR_TEXTURES];
	nyas_texture depth;
	nyas_texture stencil;
} nyas_framebuffer;

typedef struct nyas_draw_state {
	nyas_flags enable_flags;
	nyas_flags disable_flags;
	nyas_enum blend_src_fn;
	nyas_enum blend_dst_fn;
	nyas_enum depth_fn;
	nyas_enum face_culling;
	int viewport_min_x;
	int viewport_min_y;
	int viewport_max_x;
	int viewport_max_y;
	int scissor_min_x;
	int scissor_min_y;
	int scissor_max_x;
	int scissor_max_y;
	float bg_color_r;
	float bg_color_g;
	float bg_color_b;
	float bg_color_a;
} nyas_draw_state;

typedef struct nyas_draw_unit {
	nyas_material material;
	nyas_mesh mesh;
} nyas_draw_unit;

typedef struct nyas_draw_command {
	nyas_draw_state state;
	nyas_draw_unit *units;
	ptrdiff_t unit_count;
	nyas_framebuffer framebuffer;
	nyas_shader shader;
} nyas_draw_command;

typedef struct nyas_draw_list {
	nyas_draw_command *commands;
	ptrdiff_t count;
} nyas_draw_list;

typedef struct nyas_draw_io {
	/*
	 * frame-lived allocate
	 * Allocations made with this funcion are expected to be valid until the frame is drawn.
	 * The allocations should not expect to be released by this module.
	 */
	void *(*falloc)(size_t size);

	/*
	 * error callback
	 * Error reporting function.
	 */
	void (*error)(nyas_enum code, void *context, const char *description);

	void (*log)(const char *text);
	void (*assert)(int expression);
} nyas_draw_io;

typedef struct nyas_draw_context {
	nyas_draw_io io;
} nyas_draw_context;

#endif

#define NYAS_GL3

#ifdef NYAS_ELEM_SIZE_16
typedef uint16_t nyas_idx;
#else
typedef uint32_t nyas_idx;
#endif

typedef int nyas_resource_handle;
typedef nyas_resource_handle nyas_mesh;
typedef nyas_resource_handle nyas_tex;
typedef nyas_resource_handle nyas_framebuffer;
typedef nyas_resource_handle nyas_shader;

typedef int nyas_vertex_attrib;
enum NYAS_VA_ {
	NYAS_VA_POS = 0,
	NYAS_VA_NORMAL,
	NYAS_VA_TAN,
	NYAS_VA_BITAN,
	NYAS_VA_UV,
	NYAS_VA_COUNT
};

typedef int nyas_framebuffer_attach;
enum NYAS_ATTACH_ {
	NYAS_ATTACH_DEPTH_STENCIL = -3,
	NYAS_ATTACH_STENCIL = -2,
	NYAS_ATTACH_DEPTH = -1,
	NYAS_ATTACH_COLOR = 0,
	NYAS_ATTACH_COLOR1,
	NYAS_ATTACH_COLOR2,
	NYAS_ATTACH_COLOR3,
	NYAS_ATTACH_COLOR4,
	NYAS_ATTACH_COLOR5,
	NYAS_ATTACH_COLOR6
};

typedef int nyas_texture_flags;
enum NYAS_TEX_FLAG_ {
	NYAS_TEX_FLAG_DEFAULT = 0,
	NYAS_TEX_FLAG_GENERATE_MIPMAPS = 1,
	NYAS_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD = 1 << 1,
	NYAS_TEX_FLAG_COUNT
};

struct nyas_texture_desc {
	nyas_texture_flags flags;
	nyas_enum type;
	int width;
	int height;
	nyas_enum fmt;
	nyas_enum min_filter;
	nyas_enum mag_filter;
	nyas_enum wrap_s;
	nyas_enum wrap_t;
	nyas_enum wrap_r;
	float border_color[4];
};

struct nyas_texture_target {
	nyas_tex tex;
	nyas_enum face;
	nyas_framebuffer_attach attach;
	int lod_level;
};

struct nyas_shader_desc {
	const char *name;
	int data_count;
	int tex_count;
	int cubemap_count;
	int shared_data_count;
	int common_tex_count;
	int common_cubemap_count;
};

typedef struct nyas_mat {
	void *ptr; // TODO deprecar
	nyas_shader shader;
} nyas_mat;

struct nyas_draw_target {
	nyas_framebuffer fb;
	struct nyas_color bgcolor;
};

struct nyas_draw_pipeline {
	nyas_mat shader_mat;
	nyas_vertex_attrib va;
};

struct nyas_draw_ops {
	struct nyas_rect viewport;
	struct nyas_rect scissor;
	nyas_flags enable;
	nyas_flags disable;
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

typedef struct nyas_draw_cmd nydrawcmd;
NYAS_DECL_ARR(nydrawcmd);

struct nyas_draw {
	struct nyas_render_state state;
	struct nyarr_nydrawcmd *cmds;
};

void nyas_falloc_set_buffer(void *buffer, ptrdiff_t size);
void *nyas_falloc(ptrdiff_t size);

nyas_tex nyas_tex_create(void);
void nyas_tex_set(nyas_tex tex, struct nyas_texture_desc *desc);
void nyas_tex_load(nyas_tex tex, struct nyas_texture_desc *desc, const char *path);
struct nyas_point nyas_tex_size(nyas_tex tex);

nyas_framebuffer nyas_fb_create(void);
void nyas_fb_set_target(nyas_framebuffer fb, int index, struct nyas_texture_target target);

// TODO(Renderer): Unificar load y reload
nyas_mesh nyas_mesh_create(void);
nyas_mesh nyas_mesh_load_file(const char *path);
void nyas_mesh_reload_file(nyas_mesh mesh, const char *path);

nyas_shader nyas_shader_create(const struct nyas_shader_desc *desc);
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

void nyas_draw(struct nyas_draw *dl);

#endif // NYAS_DEFS_H
