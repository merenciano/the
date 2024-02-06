#ifndef NYAS_PIXELS_DEFS_H
#define NYAS_PIXELS_DEFS_H

enum nyas_blend_func {
	NYAS_BLEND_INVALID = 0,
	NYAS_BLEND_ONE,
	NYAS_BLEND_SRC_ALPHA,
	NYAS_BLEND_ONE_MINUS_SRC_ALPHA,
	NYAS_BLEND_ZERO
};

enum nyas_cull_face {
	NYAS_CULL_CURRENT = 0,
	NYAS_CULL_FRONT,
	NYAS_CULL_BACK,
	NYAS_CULL_FRONT_AND_BACK
};

enum nyas_depth_func {
	// TODO: Add as needed.
	NYAS_DEPTH_CURRENT = 0,
	NYAS_DEPTH_LEQUAL,
	NYAS_DEPTH_LESS,
};

enum nyas_texture_flagggs {
	TF_CHANNELS = 0, // 0 and 1 bits
	TF_CUBE = 1 << 2,
	TF_DEPTH = 1 << 3,
	TF_MIPMAP = 1 << 4,
	TF_FLOAT = 1 << 5,
	TF_TILING = 1 << 6,
	TF_MIN_FILTER_LERP = 1 << 7,
	TF_MAG_FILTER_LERP = 1 << 8,
	TF_MAG_MIP_FILTER_LERP = 1 << 9,
	TF_LINEAR_COLOR = 1 << 10,
	TF_HALF_FLOAT = 1 << 11
};

enum nyas_framebuffer_slots {
	NYAS_SLOT_DEPTH,
	NYAS_SLOT_STENCIL,
	NYAS_SLOT_DEPTH_STENCIL,
	NYAS_SLOT_COLOR0,
	NYAS_SLOT_COLOR1,
	NYAS_SLOT_COLOR2,
	NYAS_SLOT_COLOR3,
	NYAS_SLOT_COLOR4,
	NYAS_SLOT_COLOR5,
};

enum nyas_cube_faces {
	NYAS_CUBE_POS_X,
	NYAS_CUBE_NEG_X,
	NYAS_CUBE_POS_Y,
	NYAS_CUBE_NEG_Y,
	NYAS_CUBE_POS_Z,
	NYAS_CUBE_NEG_Z,
	NYAS_CUBE_FACE_COUNT
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
	int levels;
	int desired_channels;
	nyas_texture_format fmt;
	nyas_texture_filter min_filter;
	nyas_texture_filter mag_filter;
	nyas_texture_wrap wrap_s;
	nyas_texture_wrap wrap_t;
	nyas_texture_wrap wrap_r;
	float border_color[4];
};


#endif // NYAS_PIXELS_DEFS_H
