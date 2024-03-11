#ifndef NYAS_DEFS_H
#define NYAS_DEFS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef int nyas_handle;
typedef nyas_handle nyas_tex_h;
typedef nyas_handle nyas_mesh_h;
typedef nyas_handle nyas_shader_h;
typedef nyas_handle nyas_fb_h;

typedef int nyas_enum;
typedef enum nyas_enum_ {
	NYAS_NULL,

	NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_TYPE_2D = NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_TYPE_2D_ARRAY,
	NYAS_TEXTURE_TYPE_CUBEMAP,
	NYAS_TEXTURE_TYPE_CUBEMAP_ARRAY,
	NYAS_TEXTURE_TYPE_DEFAULT,

	NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FORMAT_DEPTH = NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FORMAT_STENCIL,
	NYAS_TEXTURE_FORMAT_DEPTH_STENCIL,
	NYAS_TEXTURE_FORMAT_R_8,
	NYAS_TEXTURE_FORMAT_RG_8,
	NYAS_TEXTURE_FORMAT_RGB_8,
	NYAS_TEXTURE_FORMAT_RGBA_8,
	NYAS_TEXTURE_FORMAT_SRGB_8,
	NYAS_TEXTURE_FORMAT_SRGBA_8,
	NYAS_TEXTURE_FORMAT_R_16F,
	NYAS_TEXTURE_FORMAT_RG_16F,
	NYAS_TEXTURE_FORMAT_RGB_16F,
	NYAS_TEXTURE_FORMAT_RGBA_16F,
	NYAS_TEXTURE_FORMAT_R_32F,
	NYAS_TEXTURE_FORMAT_RG_32F,
	NYAS_TEXTURE_FORMAT_RGB_32F,
	NYAS_TEXTURE_FORMAT_RGBA_32F,
	NYAS_TEXTURE_FORMAT_DEFAULT,

	NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_FILTER_LINEAR = NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_DEFAULT,

	NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_WRAP_CLAMP_EDGE = NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_WRAP_WHITE,
	NYAS_TEXTURE_WRAP_BLACK,
	NYAS_TEXTURE_WRAP_CLAMP_REPEAT,
	NYAS_TEXTURE_WRAP_CLAMP_MIRROR,
	NYAS_TEXTURE_WRAP_DEFAULT,

	NYAS_TEXTURE_FACE,
	NYAS_TEXTURE_FACE_POS_X = NYAS_TEXTURE_FACE,
	NYAS_TEXTURE_FACE_NEG_X,
	NYAS_TEXTURE_FACE_POS_Y,
	NYAS_TEXTURE_FACE_NEG_Y,
	NYAS_TEXTURE_FACE_POS_Z,
	NYAS_TEXTURE_FACE_DEFAULT,

	NYAS_VERTEX_ATTRIBUTE,
	NYAS_VERTEX_ATTRIBUTE_POSITION = NYAS_VERTEX_ATTRIBUTE,
	NYAS_VERTEX_ATTRIBUTE_NORMAL,
	NYAS_VERTEX_ATTRIBUTE_TANGENT,
	NYAS_VERTEX_ATTRIBUTE_BITANGENT,
	NYAS_VERTEX_ATTRIBUTE_UV,
	NYAS_VERTEX_ATTRIBUTE_COLOR,
	NYAS_VERTEX_ATTRIBUTE_DEFAULT,

	NYAS_TEXTURE_TYPE_COUNT = NYAS_TEXTURE_TYPE_DEFAULT - NYAS_TEXTURE_TYPE,
	NYAS_TEXTURE_FORMAT_COUNT = NYAS_TEXTURE_FORMAT_DEFAULT - NYAS_TEXTURE_FORMAT,
	NYAS_TEXTURE_FILTER_COUNT = NYAS_TEXTURE_FILTER_DEFAULT - NYAS_TEXTURE_FILTER,
	NYAS_TEXTURE_WRAP_COUNT = NYAS_TEXTURE_WRAP_DEFAULT - NYAS_TEXTURE_WRAP,
	NYAS_TEXTURE_FACE_COUNT = NYAS_TEXTURE_FACE_DEFAULT - NYAS_TEXTURE_FACE,
	NYAS_VERTEX_ATTRIBUTE_COUNT = NYAS_VERTEX_ATTRIBUTE_DEFAULT - NYAS_VERTEX_ATTRIBUTE,

	NYAS_FLAG_NONE = 0,

	NYAS_FLAG_VA_POSITION = 1 << (NYAS_VERTEX_ATTRIBUTE_POSITION - NYAS_VERTEX_ATTRIBUTE),
	NYAS_FLAG_VA_NORMAL = 1 << (NYAS_VERTEX_ATTRIBUTE_NORMAL - NYAS_VERTEX_ATTRIBUTE),
	NYAS_FLAG_VA_TANGENT = 1 << (NYAS_VERTEX_ATTRIBUTE_TANGENT - NYAS_VERTEX_ATTRIBUTE),
	NYAS_FLAG_VA_BITANGENT = 1 << (NYAS_VERTEX_ATTRIBUTE_BITANGENT - NYAS_VERTEX_ATTRIBUTE),
	NYAS_FLAG_VA_UV = 1 << (NYAS_VERTEX_ATTRIBUTE_UV - NYAS_VERTEX_ATTRIBUTE),
	NYAS_FLAG_VA_COLOR = 1 << (NYAS_VERTEX_ATTRIBUTE_COLOR - NYAS_VERTEX_ATTRIBUTE),

	NYAS_FLAG_TEXTURE_CUBEMAP = 1 << 0,
	NYAS_FLAG_TEXTURE_ARRAY = 1 << 1,
} nyas_enum_;

typedef struct nyas_texture_data {
	void *pixel_buffer;
	ptrdiff_t pixel_buffer_size;
	int lod; /* level of detail */
	int index; /* texture arrays only */
	nyas_enum face; /* cubemaps only */
} nyas_texture_data;

typedef struct nyas_texture {
	nyas_texture_data *images;
	int img_count;
	int width;
	int height;
	nyas_enum type; // NYAS_TEXTURE_TYPE_
	nyas_enum format; // NYAS_TEXTURE_FORMAT_
	nyas_enum min_filter;
	nyas_enum mag_filter;
	nyas_enum wrap_s;
	nyas_enum wrap_t;
	nyas_enum wrap_r;
} nyas_texture;

typedef struct nyas_mesh {
	float *vertices;
	void *indices;
	int vtx_count;
	int idx_count;
	int indices_stride;
	int vertex_attributes; // nyas_texture_flags_
} nyas_mesh;

typedef struct nyas_material {
	float *values;
	nyas_handle *textures;
	nyas_handle *cubemaps;
	nyas_handle shader;
} nyas_material;

typedef struct nyas_shader {
	nyas_material shared_data; // Read-only data for material instances.
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
	int vertex_attributes; // ?? NYAS_FLAG_VA (see: struct nyas_mesh)
} nyas_shader;

#define NYAS_CONFIG_FRAMEBUFFER_COLOR_TEXTURES 6
typedef struct nyas_framebuffer {
	nyas_handle textures[NYAS_CONFIG_FRAMEBUFFER_COLOR_TEXTURES];
	nyas_handle depth;
	nyas_handle stencil;
} nyas_framebuffer;

typedef struct nyas_draw_state {
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
	bool clear_color;
	bool clear_depth;
	bool clear_stencil;
} nyas_draw_state;

typedef struct nyas_draw_unit {
	nyas_handle mesh;
	nyas_material material;
} nyas_draw_unit;

typedef struct nyas_draw_command {
	nyas_draw_state state;
	nyas_draw_unit *units;
	int unit_count;
	nyas_handle framebuffer;
	nyas_handle shader;
} nyas_draw_command;

typedef struct nyas_draw_list {
	nyas_draw_command *commands;
	int count;
} nyas_draw_list;

#endif // NYAS_DEFS_H
