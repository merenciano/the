#ifndef NYASPIX_H
#define NYASPIX_H

#include <stddef.h>
#include <stdint.h>

typedef uint16_t nypx_index;

enum nypx_vertex_attributes {
	VA_POSITION = 0,
	VA_NORMAL,
	VA_TANGENT,
	VA_BITANGENT,
	VA_UV,
	VTXATTR_COUNT
};

enum nypx_texture_flags {
	TF_CHANNELS = 0, // 0 and 1 bits
	TF_CUBE = 1 << 2,
	TF_DEPTH = 1 << 3,
	TF_MIPMAP = 1 << 4,
	TF_FLOAT = 1 << 5,
	TF_TILING = 1 << 6,
	TF_MIN_FILTER_LERP = 1 << 7,
	TF_MAG_FILTER_LERP = 1 << 8,
	TF_MAG_MIP_FILTER_LERP = 1 << 9,
	TF_LINEAR_COLOR = 1 << 10
};

typedef struct nypx_res {
	int id; // internal handle
	int flags;
} nypx_res;

enum nypx_framebuffer_slots {
	NYPX_SLOT_DEPTH,
	NYPX_SLOT_STENCIL,
	NYPX_SLOT_DEPTH_STENCIL,
	NYPX_SLOT_COLOR0,
	NYPX_SLOT_COLOR1,
	NYPX_SLOT_COLOR2,
	NYPX_SLOT_COLOR3,
	NYPX_SLOT_COLOR4,
	NYPX_SLOT_COLOR5,
};

enum nypx_cube_faces {
	NYPX_CUBE_POS_X,
	NYPX_CUBE_NEG_X,
	NYPX_CUBE_POS_Y,
	NYPX_CUBE_NEG_Y,
	NYPX_CUBE_POS_Z,
	NYPX_CUBE_NEG_Z,
	NYPX_CUBE_FACE_COUNT
};

typedef struct nypx_fb {
	nypx_res res;
} nypx_fb;

void nypx_tex_create(uint32_t *id, int type);
void nypx_tex_set(uint32_t id, int type, int width, int height, void **pix);
void nypx_tex_release(uint32_t *id);

void nypx_mesh_create(uint32_t *id, uint32_t *vid, uint32_t *iid);
void nypx_mesh_use(uint32_t id);
void nypx_mesh_set(uint32_t id,
                   uint32_t vid,
                   uint32_t iid,
                   uint32_t shader_id,
                   int attrib,
                   float *vtx,
                   size_t vsize,
                   nypx_index *idx,
                   size_t elements);
void nypx_mesh_release(uint32_t *id, uint32_t *vid, uint32_t *iid);

void nypx_shader_create(uint32_t *id);
void nypx_shader_compile(uint32_t id, const char *name);
void nypx_shader_loc(uint32_t id, int *o_loc, const char **i_unif, int count);
void nypx_shader_set_data(int loc, float *data, int v4count);
void nypx_shader_set_tex(int loc, int *tex, int count, int texunit_offset);
void nypx_shader_set_cube(int loc, int *tex, int count, int texunit_offset);
void nypx_shader_use(uint32_t id);
void nypx_shader_release(uint32_t id);

void nypx_fb_create(uint32_t *id);

// Sets fb attach from cubemap face
// id: framebuffer internal id
// texid: cubemap internal id
// slot: framebuffer attachment
// level: mipmap level
// face: cubemap face (see enum nypx_cube_faces)
void nypx_fb_set(uint32_t id,
                      uint32_t texid,
                      int slot,
                      int level,
                      int face);
void nypx_fb_use(uint32_t id);
void nypx_fb_release(uint32_t *id);

void nypx_clear(int color, int depth, int stencil);
void nypx_draw(int elem_count, int half_type); // half_type is uint16_t

enum nypx_blend_func {
	NYPX_BLEND_INVALID = 0,
	NYPX_BLEND_ONE,
	NYPX_BLEND_SRC_ALPHA,
	NYPX_BLEND_ONE_MINUS_SRC_ALPHA,
	NYPX_BLEND_ZERO,
};

enum nypx_cull_face {
	NYPX_CULL_CURRENT = 0,
	NYPX_CULL_FRONT,
	NYPX_CULL_BACK,
	NYPX_CULL_FRONT_AND_BACK,
};

enum nypx_depth_func {
	// TODO: Add as needed.
	NYPX_DEPTH_CURRENT = 0,
	NYPX_DEPTH_LEQUAL,
	NYPX_DEPTH_LESS,
};

void nypx_clear_color(float r, float g, float b, float a);
void nypx_blend_enable(void);
void nypx_blend_disable(void);
void nypx_blend_set(enum nypx_blend_func src, enum nypx_blend_func dst);
void nypx_cull_enable(void);
void nypx_cull_disable(void);
void nypx_cull_set(enum nypx_cull_face cull);
void nypx_depth_enable_test(void);
void nypx_depth_disable_test(void);
void nypx_depth_enable_mask(void);
void nypx_depth_disable_mask(void);
void nypx_depth_set(enum nypx_depth_func depth);
void nypx_viewport(int x, int y, int width, int height);

#endif // NYASPIX_H
