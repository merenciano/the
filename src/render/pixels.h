#ifndef THE_PIXELS_H
#define THE_PIXELS_H

#include "core/common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef THE_ELEM_SIZE_16
typedef uint16_t the_idx;
#else
typedef uint32_t the_idx;
#endif

typedef int the_resource_handle;
typedef the_resource_handle the_mesh;
typedef the_resource_handle the_tex;
typedef the_resource_handle the_framebuffer;
typedef the_resource_handle the_shader;

typedef int the_vertex_attrib;
enum the_vertex_attrib {
	THE_VA_POS = 0,
	THE_VA_NORMAL,
	THE_VA_TAN,
	THE_VA_BITAN,
	THE_VA_UV,
	THE_VA_COUNT
};

typedef int the_blend_func;
enum the_blend_func {
	THE_BLEND_CURRENT = 0,
	THE_BLEND_ONE,
	THE_BLEND_SRC_ALPHA,
	THE_BLEND_ONE_MINUS_SRC_ALPHA,
	THE_BLEND_ZERO
};

typedef int the_cull_face;
enum the_cull_face {
	THE_CULL_CURRENT = 0,
	THE_CULL_FRONT,
	THE_CULL_BACK,
	THE_CULL_FRONT_AND_BACK
};

typedef int the_depth_func;
enum the_depth_func {
	// TODO: Add as needed.
	THE_DEPTH_CURRENT = 0,
	THE_DEPTH_LEQUAL,
	THE_DEPTH_LESS,
};

typedef int the_texture_face;
enum the_texture_face {
	THE_FACE_POS_X = 0,
	THE_FACE_NEG_X,
	THE_FACE_POS_Y,
	THE_FACE_NEG_Y,
	THE_FACE_POS_Z,
	THE_FACE_NEG_Z,
	THE_FACE_2D,
	THE_CUBE_FACE_COUNT
};

typedef int the_framebuffer_attach;
enum the_framebuffer_attach {
	THE_ATTACH_DEPTH_STENCIL = -3,
	THE_ATTACH_STENCIL = -2,
	THE_ATTACH_DEPTH = -1,
	THE_ATTACH_COLOR = 0,
	THE_ATTACH_COLOR1,
	THE_ATTACH_COLOR2,
	THE_ATTACH_COLOR3,
	THE_ATTACH_COLOR4,
	THE_ATTACH_COLOR5,
	THE_ATTACH_COLOR6
};

typedef int the_texture_flags;
enum the_texture_flags {
	THE_TEX_FLAG_DEFAULT = 0,
	THE_TEX_FLAG_GENERATE_MIPMAPS = 1,
	THE_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD = 1 << 1,
	THE_TEX_FLAG_COUNT
};

typedef int the_texture_type;
enum the_texture_type {
	THE_TEX_DEFAULT = 0,
	THE_TEX_2D,
	THE_TEX_ARRAY_2D,
	THE_TEX_CUBEMAP,
	THE_TEX_ARRAY_CUBEMAP,
	THE_TEX_COUNT
};

typedef int the_texture_format; // enum the_texture_formats
enum the_texture_format {
	THE_TEX_FMT_DEFAULT = 0,
	THE_TEX_FMT_DEPTH,
	THE_TEX_FMT_STENCIL,
	THE_TEX_FMT_DEPTH_STENCIL,
	THE_TEX_FMT_SRGB,
	THE_TEX_FMT_SRGBA,
	THE_TEX_FMT_R8,
	THE_TEX_FMT_RG8,
	THE_TEX_FMT_RGB8,
	THE_TEX_FMT_RGBA8,
	THE_TEX_FMT_R16F,
	THE_TEX_FMT_RG16F,
	THE_TEX_FMT_RGB16F,
	THE_TEX_FMT_RGBA16F,
	THE_TEX_FMT_RGB32F,
	THE_TEX_FMT_COUNT
};

typedef int the_texture_filter; // enum the_texture_filters
enum the_texture_filter {
	THE_TEX_FLTR_DEFAULT = 0,
	THE_TEX_FLTR_LINEAR,
	THE_TEX_FLTR_LINEAR_MIPMAP_LINEAR,
	THE_TEX_FLTR_LINEAR_MIPMAP_NEAR,
	THE_TEX_FLTR_NEAR,
	THE_TEX_FLTR_NEAR_MIPMAP_NEAR,
	THE_TEX_FLTR_NEAR_MIPMAP_LINEAR,
	THE_TEX_FLTR_COUNT
};

typedef int the_texture_wrap;
enum the_texture_wrap {
	THE_TEX_WRAP_DEFAULT = 0,
	THE_TEX_WRAP_CLAMP,
	THE_TEX_WRAP_REPEAT,
	THE_TEX_WRAP_MIRROR,
	THE_TEX_WRAP_BORDER,
	THE_TEX_WRAP_COUNT
};

struct the_texture_desc {
	the_texture_flags flags;
	the_texture_type type;
	int width;
	int height;
	the_texture_format fmt;
	the_texture_filter min_filter;
	the_texture_filter mag_filter;
	the_texture_wrap wrap_s;
	the_texture_wrap wrap_t;
	the_texture_wrap wrap_r;
	float border_color[4];
};

struct the_texture_target {
	the_tex tex;
	the_texture_face face;
	the_framebuffer_attach attach;
	int lod_level;
};

struct the_shader_desc {
	const char *name;
	int data_count;
	int tex_count;
	int cubemap_count;
	int shared_data_count;
	int common_tex_count;
	int common_cubemap_count;
};

typedef struct the_mat {
	void *ptr; // TODO deprecar
	the_shader shader;
} the_mat;

typedef int the_draw_flags;
enum the_draw_flags {
	THE_DRAW_CLEAR_COLOR,
	THE_DRAW_CLEAR_DEPTH,
	THE_DRAW_CLEAR_STENCIL,
	THE_DRAW_TEST_DEPTH,
	THE_DRAW_TEST_STENCIL,
	THE_DRAW_WRITE_DEPTH,
	THE_DRAW_WRITE_STENCIL,
	THE_DRAW_BLEND,
	THE_DRAW_CULL,
	THE_DRAW_SCISSOR,
	THE_DRAW_FLAGS_COUNT
};

struct the_draw_target {
	the_framebuffer fb;
	struct the_color bgcolor;
};

struct the_draw_pipeline {
	the_mat shader_mat;
	the_vertex_attrib va;
};

struct the_draw_ops {
	struct the_rect viewport;
	struct the_rect scissor;
	the_draw_flags enable;
	the_draw_flags disable;
	uint8_t blend_src;
	uint8_t blend_dst;
	uint8_t cull_face;
	uint8_t depth_fun;
};

struct the_draw_cmd {
	the_mesh mesh;
	the_mat material;
};

struct the_render_state {
	struct the_draw_target target;
	struct the_draw_pipeline pipeline;
	struct the_draw_ops ops;
};

typedef struct the_draw_cmd thedrawcmd;
THE_DECL_ARR(thedrawcmd);

struct the_draw {
	struct the_render_state state;
	struct thearr_thedrawcmd *cmds;
};

void the_falloc_set_buffer(void *buffer, ptrdiff_t size);
void *the_falloc(ptrdiff_t size);

the_tex the_tex_create(void);
void the_tex_set(the_tex tex, struct the_texture_desc *desc);
void the_tex_load(the_tex tex, struct the_texture_desc *desc, const char *path);
struct the_point the_tex_size(the_tex tex);

the_framebuffer the_fb_create(void);
void the_fb_set_target(the_framebuffer fb, int index, struct the_texture_target target);

// TODO(Renderer): Unificar load y reload
the_mesh the_mesh_create(void);
the_mesh the_mesh_load_file(const char *path);
void the_mesh_reload_file(the_mesh mesh, const char *path);

the_shader the_shader_create(const struct the_shader_desc *desc);
void *the_shader_data(the_shader shader);
void the_shader_reload(the_shader shader);
the_tex *the_shader_tex(the_shader shader);
the_tex *the_shader_cubemap(the_shader shader);

/* Creates a new material and alloc persistent memory for its data */
the_mat the_mat_create(the_shader shader);
/* Creates a new material and alloc frame-scoped memory for its data */
the_mat the_mat_tmp(the_shader shader);
the_mat the_mat_copy(the_mat mat);
the_mat the_mat_copy_shader(the_shader shader);
the_tex *the_mat_tex(the_mat mat); // Ptr to first texture.

void the_draw(struct the_draw *dl);
void the_render_sync(); // Blocks until gpu sync (glFinish in OpenGL)

#endif // THE_PIXELS_H
