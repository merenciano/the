#ifndef NYAS_CORE_H
#define NYAS_CORE_H

#include "nyas_core.h"

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

typedef struct nyas_draw_list {
	nyas_draw_command *commands;
	ptrdiff_t count;
} nyas_draw_list;

typedef struct nyas_draw_interface {
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

	int win_width;
	int win_height;
} nyas_draw_interface;

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
typedef enum nyas_tex_flag_ {
	NYAS_TEX_FLAG_DEFAULT = 0,
	NYAS_TEX_FLAG_GENERATE_MIPMAPS = 1,
	NYAS_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD = 1 << 1,
	NYAS_TEX_FLAG_COUNT
} nyas_tex_flag_;

typedef struct nyas_texture_desc {
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
} nyas_texture_desc;

typedef struct nyas_texture_target {
	nyas_tex tex;
	nyas_enum face;
	nyas_framebuffer_attach attach;
	int lod_level;
} nyas_texture_target;

typedef struct nyas_shader_desc {
	const char *name;
	int data_count;
	int tex_count;
	int cubemap_count;
	int shared_data_count;
	int common_tex_count;
	int common_cubemap_count;
} nyas_shader_desc;

typedef struct nyas_mat {
	void *ptr; // TODO deprecar
	nyas_shader shader;
} nyas_mat;

typedef int nyas_resource_flags;
enum nyas_resource_flags {
	NYAS_IRF_DIRTY = 1U << 3,
	NYAS_IRF_CREATED = 1U << 4,
	NYAS_IRF_RELEASE_APP_STORAGE = 1U << 5,
	NYAS_IRF_MAPPED = 1U << 7,
};

struct nyas_resource_internal {
	uint32_t id;
	nyas_resource_flags flags;
};

struct nyas_mesh_internal {
	struct nyas_resource_internal res;
	struct nyas_resource_internal res_vb; // vertex buffer resource
	struct nyas_resource_internal res_ib; // index buffer resource
	float *vtx;
	nyas_idx *idx;
	int64_t elem_count;
	uint32_t vtx_size;
	nyas_vertex_attrib attrib;
};

struct nyas_texture_image {
	void *pix;
	nyas_enum face;
	int lod;
};

typedef struct nyas_texture_image nyteximg;
NYAS_DECL_ARR(nyteximg);

struct nyas_texture_internal {
	struct nyas_resource_internal res;
	struct nyas_texture_desc data;
	struct nyarr_nyteximg *img;
};

struct nyas_shader_internal {
	struct nyas_resource_internal res;
	const char *name;
	struct {
		int data, tex, cubemap;
	} loc[2], count[2]; // 0: unit, 1: common
	void *common;
};

struct nyas_framebuffer_internal {
	struct nyas_resource_internal res;
	struct nyas_texture_target target[8];
};

typedef struct nyas_draw_state {
	nyas_enum blend_src_fn; // NYAS_DRAW_BLEND_
	nyas_enum blend_dst_fn; // NYAS_DRAW_BLEND_
	nyas_enum depth_fn; // NYAS_DRAW_DEPTH_
	nyas_enum face_culling; // NYAS_DRAW_CULL_
	int enable_flags; // NYAS_FLAG_DO_
	int disable_flags; // NYAS_FLAG_DO_
	int viewport_min_x; // if (viewport_min_x == viewport_max_x): no-op
	int viewport_min_y;
	int viewport_max_x;
	int viewport_max_y;
	int scissor_min_x; // if (scissor_min_x == scissor_max_x): no-op
	int scissor_min_y;
	int scissor_max_x;
	int scissor_max_y;
	float bg_color_r; // if (bg_color_a == -1.0f): no-op
	float bg_color_g;
	float bg_color_b;
	float bg_color_a;
} nyas_draw_state;

typedef struct nyas_draw_unit {
	nyas_mat material;
	nyas_mesh mesh;
} nyas_draw_unit;

typedef struct nyas_draw_cmd {
	nyas_draw_state state;
	nyas_draw_unit *units;
	int unit_count;
	nyas_framebuffer fb;
	nyas_mat shader_mat;
} nyas_draw_cmd;

typedef struct nyas_mesh_internal mesh;
NYAS_DECL_ARR(mesh);
NYAS_DECL_POOL(mesh);

typedef struct nyas_texture_internal tex;
NYAS_DECL_ARR(tex);
NYAS_DECL_POOL(tex);

typedef struct nyas_shader_internal shad;
NYAS_DECL_ARR(shad);
NYAS_DECL_POOL(shad);

typedef struct nyas_framebuffer_internal fb;
NYAS_DECL_ARR(fb);
NYAS_DECL_POOL(fb);


extern struct nypool_mesh mesh_pool;
extern struct nypool_tex tex_pool;
extern struct nypool_shad shader_pool;
extern struct nypool_fb framebuffer_pool;
#if 0
typedef struct nyas_draw_ctx {
	struct nypool_mesh mesh_pool;
	struct nypool_tex tex_pool;
	struct nypool_shad shader_pool;
	struct nypool_fb framebuffer_pool;

	void *circular_buffer;
	ptrdiff_t buffer_capacity;
} nyas_draw_ctx;
#endif

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

void nyas_draw(nyas_draw_cmd *command);

// ---
// [HELPERS] STRUCT DEFAULTS
// ---

static inline nyas_texture_desc
nyas_texture_desc_default(nyas_enum type, nyas_enum fmt, int w, int h)
{
	return (nyas_texture_desc) {
		.flags = NYAS_TEX_FLAG_DEFAULT,
		.type = type,
		.width = w,
		.height = h,
		.fmt = fmt,
		.min_filter = NYAS_TEXTURE_FILTER_LINEAR,
		.mag_filter = NYAS_TEXTURE_FILTER_LINEAR,
		.wrap_s = NYAS_TEXTURE_WRAP_REPEAT,
		.wrap_t = NYAS_TEXTURE_WRAP_REPEAT,
		.wrap_r = NYAS_TEXTURE_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

static inline nyas_draw_state
nyas_draw_state_default(void)
{
	return (nyas_draw_state) {
		.blend_src_fn = NYAS_DRAW_BLEND_DEFAULT,
		.blend_dst_fn = NYAS_DRAW_BLEND_DEFAULT,
		.depth_fn = NYAS_DRAW_DEPTH_DEFAULT,
		.face_culling = NYAS_DRAW_CULL_DEFAULT,
		.enable_flags = NYAS_FLAG_NONE,
		.disable_flags = NYAS_FLAG_NONE,
		.viewport_min_x = 0,
		.viewport_min_y = 0,
		.viewport_max_x = 0,
		.viewport_max_y = 0,
		.scissor_min_x = 0,
		.scissor_min_y = 0,
		.scissor_max_x = 0,
		.scissor_max_y = 0,
		.bg_color_r = 0.0f,
		.bg_color_g = 0.0f,
		.bg_color_b = 0.0f,
		.bg_color_a = -1.0f
	};
}

static inline nyas_draw_cmd
nyas_draw_cmd_default(void)
{
	return (nyas_draw_cmd) {
		.fb = NYAS_NOOP,
		.shader_mat = {.shader = NYAS_NOOP, .ptr = NULL},
		.state = nyas_draw_state_default(),
		.units = NULL,
		.unit_count = 0
	};
}

#endif // NYAS_CORE_H
