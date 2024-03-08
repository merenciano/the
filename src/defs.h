#ifndef NYAS_DEFS_H
#define NYAS_DEFS_H

#include <stddef.h>
#include <stdint.h>

typedef struct nyas_range {
	void *start;
	ptrdiff_t len; // bytes
} nyas_range;

typedef struct nyas_rangec {
	char *start;
	ptrdiff_t len;
} nyas_rangec;

typedef struct nyas_rangei {
	int *start;
	ptrdiff_t len; // count
} nyas_rangei;

typedef struct nyas_rangef {
	float *start;
	ptrdiff_t len; // count
} nyas_rangef;

typedef int nyas_texture_flags;
typedef enum nyas_texture_flags_ {
	NYAS_TEXTURE_FLAGS_NONE = 0,
	NYAS_TEXTURE_FLAGS_CUBEMAP = 1 << 0,
	NYAS_TEXTURE_FLAGS_ARRAY = 1 << 1,
} nyas_texture_flags_;

typedef int nyas_texture_format;
typedef enum nyas_texture_format_ {
	NYAS_TEXTURE_FORMAT_NONE,
	NYAS_TEXTURE_FORMAT_DEFAULT,
	NYAS_TEXTURE_FORMAT_DEPTH,
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
	NYAS_TEXTURE_FORMAT_COUNT
} nyas_texture_format_;

typedef int8_t nyas_texture_filter;
typedef enum nyas_texture_filter_ {
	NYAS_TEXTURE_FILTER_NONE,
	NYAS_TEXTURE_FILTER_DEFAULT,
	NYAS_TEXTURE_FILTER_LINEAR,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	NYAS_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
	NYAS_TEXTURE_FILTER_COUNT
} nyas_texture_filter_;

typedef int8_t nyas_texture_wrap;
typedef enum nyas_texture_wrap_ {
	NYAS_TEXTURE_WRAP_NONE,
	NYAS_TEXTURE_WRAP_DEFAULT,
	NYAS_TEXTURE_WRAP_CLAMP_EDGE,
	NYAS_TEXTURE_WRAP_WHITE,
	NYAS_TEXTURE_WRAP_BLACK,
	NYAS_TEXTURE_WRAP_CLAMP_REPEAT,
	NYAS_TEXTURE_WRAP_CLAMP_MIRROR,
	NYAS_TEXTURE_WRAP_CLAMP_COUNT
} nyas_texture_wrap_;

typedef int8_t nyas_texture_face;
typedef enum nyas_texture_face_ {
	NYAS_TEXTURE_FACE_NONE,
	NYAS_TEXTURE_FACE_DEFAULT,
	NYAS_TEXTURE_FACE_POS_X,
	NYAS_TEXTURE_FACE_NEG_X,
	NYAS_TEXTURE_FACE_POS_Y,
	NYAS_TEXTURE_FACE_NEG_Y,
	NYAS_TEXTURE_FACE_POS_Z,
	NYAS_TEXTURE_FACE_COUNT
} nyas_texture_face_;

typedef struct nyas_texture_data {
	nyas_range pixels;
	int16_t index; /* texture arrays only */
	int8_t lod; /* level of detail */
	nyas_texture_face face; /* cubemaps only */
} nyas_texture_data;

typedef struct nyas_texture {
	nyas_texture_flags flags;
	nyas_texture_format format;
	int width;
	int height;

	nyas_texture_filter min_filter;
	nyas_texture_filter mag_filter;
	nyas_texture_wrap wrap_s;
	nyas_texture_wrap wrap_t;
	nyas_texture_wrap wrap_r;

	nyas_texture_data *images;
	int images_count;

} nyas_texture;

typedef int nyas_vertex_attribute_flags;
typedef enum nyas_vertex_attribute_flags_ {
	NYAS_VERTEX_ATTRIBUTE_FLAGS_POS = 0,
	NYAS_VA_NORMAL,
	NYAS_VA_TAN,
	NYAS_VA_BITAN,
	NYAS_VA_UV,
	NYAS_VA_COUNT
} nyas_vertex_attribute_flags_;

typedef struct nyas_mesh {
	nyas_rangef vertices;
	nyas_rangei indices;

} nyas_mesh;
typedef int nyas_shader;
typedef int nyas_framebuffer;

typedef struct nyas_material {
	nyas_shader shader;
	nyas_rangef data;
	nyas_rangei textures;
	nyas_rangei cube_maps;
} nyas_material;

typedef struct nyas_draw_target {
	nyas_framebuffer fb;
} nyas_draw_target;

typedef struct nyas_draw_unit {
	nyas_mesh mesh;
	nyas_material material;
} nyas_draw_unit;

typedef struct nyas_draw_data {


	nyas_draw_unit *units;
	int unit_count;
} nyas_draw_data;

#endif // NYAS_DEFS_H
