#ifndef NYAS_PIXELS_DEFS_H
#define NYAS_PIXELS_DEFS_H

typedef int nyas_resource_handle;
typedef nyas_resource_handle nyas_mesh;
typedef nyas_resource_handle nyas_tex;
typedef nyas_resource_handle nyas_framebuffer;
typedef nyas_resource_handle nyas_shader;

typedef int nyas_vertex_attrib;
enum nyas_vertex_attrib {
	NYAS_VA_POS = 0,
	NYAS_VA_NORMAL,
	NYAS_VA_TAN,
	NYAS_VA_BITAN,
	NYAS_VA_UV,
	NYAS_VA_COUNT
};

typedef int nyas_blend_func;
enum nyas_blend_func {
	NYAS_BLEND_CURRENT = 0,
	NYAS_BLEND_ONE,
	NYAS_BLEND_SRC_ALPHA,
	NYAS_BLEND_ONE_MINUS_SRC_ALPHA,
	NYAS_BLEND_ZERO
};

typedef int nyas_cull_face;
enum nyas_cull_face {
	NYAS_CULL_CURRENT = 0,
	NYAS_CULL_FRONT,
	NYAS_CULL_BACK,
	NYAS_CULL_FRONT_AND_BACK
};

typedef int nyas_depth_func;
enum nyas_depth_func {
	// TODO: Add as needed.
	NYAS_DEPTH_CURRENT = 0,
	NYAS_DEPTH_LEQUAL,
	NYAS_DEPTH_LESS,
};

typedef int nyas_texture_face;
enum nyas_texture_face {
	NYAS_FACE_POS_X = 0,
	NYAS_FACE_NEG_X,
	NYAS_FACE_POS_Y,
	NYAS_FACE_NEG_Y,
	NYAS_FACE_POS_Z,
	NYAS_FACE_NEG_Z,
	NYAS_FACE_2D,
	NYAS_CUBE_FACE_COUNT
};

typedef int nyas_framebuffer_attach;
enum nyas_framebuffer_attach {
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
enum nyas_texture_flags {
	NYAS_TEX_FLAG_DEFAULT = 0,
	NYAS_TEX_FLAG_GENERATE_MIPMAPS = 1,
	NYAS_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD = 1 << 1,
	NYAS_TEX_FLAG_COUNT
};

typedef int nyas_texture_type;
enum nyas_texture_type {
	NYAS_TEX_DEFAULT = 0,
	NYAS_TEX_2D,
	NYAS_TEX_ARRAY_2D,
	NYAS_TEX_CUBEMAP,
	NYAS_TEX_ARRAY_CUBEMAP,
	NYAS_TEX_COUNT
};

typedef int nyas_texture_format; // enum nyas_texture_formats
enum nyas_texture_format {
	NYAS_TEX_FMT_DEFAULT = 0,
	NYAS_TEX_FMT_DEPTH,
	NYAS_TEX_FMT_STENCIL,
	NYAS_TEX_FMT_DEPTH_STENCIL,
	NYAS_TEX_FMT_SRGB,
	NYAS_TEX_FMT_SRGBA,
	NYAS_TEX_FMT_R8,
	NYAS_TEX_FMT_RG8,
	NYAS_TEX_FMT_RGB8,
	NYAS_TEX_FMT_RGBA8,
	NYAS_TEX_FMT_R16F,
	NYAS_TEX_FMT_RG16F,
	NYAS_TEX_FMT_RGB16F,
	NYAS_TEX_FMT_RGBA16F,
	NYAS_TEX_FMT_RGB32F,
	NYAS_TEX_FMT_COUNT
};

typedef int nyas_texture_filter; // enum nyas_texture_filters
enum nyas_texture_filter {
	NYAS_TEX_FLTR_DEFAULT = 0,
	NYAS_TEX_FLTR_LINEAR,
	NYAS_TEX_FLTR_LINEAR_MIPMAP_LINEAR,
	NYAS_TEX_FLTR_LINEAR_MIPMAP_NEAR,
	NYAS_TEX_FLTR_NEAR,
	NYAS_TEX_FLTR_NEAR_MIPMAP_NEAR,
	NYAS_TEX_FLTR_NEAR_MIPMAP_LINEAR,
	NYAS_TEX_FLTR_COUNT
};

typedef int nyas_texture_wrap;
enum nyas_texture_wrap {
	NYAS_TEX_WRAP_DEFAULT = 0,
	NYAS_TEX_WRAP_CLAMP,
	NYAS_TEX_WRAP_REPEAT,
	NYAS_TEX_WRAP_MIRROR,
	NYAS_TEX_WRAP_BORDER,
	NYAS_TEX_WRAP_COUNT
};

struct nyas_texture_desc {
	nyas_texture_flags flags;
	nyas_texture_type type;
	int width;
	int height;
	nyas_texture_format fmt;
	nyas_texture_filter min_filter;
	nyas_texture_filter mag_filter;
	nyas_texture_wrap wrap_s;
	nyas_texture_wrap wrap_t;
	nyas_texture_wrap wrap_r;
	float border_color[4];
};

struct nyas_texture_target {
	nyas_tex tex;
	nyas_texture_face face;
	nyas_framebuffer_attach attach;
	int lod_level;
};

#endif // NYAS_PIXELS_DEFS_H
