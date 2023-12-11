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

typedef struct nypx_res {
	int id; // internal handle
	int flags;
} nypx_res;

typedef struct nypx_tex {
	nypx_res res;
	void *pix[6];
	int width;
	int height;
	int type;
} nypx_tex;

typedef struct nypx_mesh {
	nypx_res res;
	nypx_res res_vb; // vertex buffer resource
	nypx_res res_ib; // index buffer resource
	float *vtx;
	size_t vtx_size;
	nypx_index *idx;
	size_t elem_count;
	int attrib;
} nypx_mesh;

typedef struct nypx_shader {
	nypx_res res;
	nypx_res res_vert;
	nypx_res res_frag;
	const char *name;
	int data_count; // uniform locations
	int tex_count;
	int cubemap_count;
	int common_data_count; // values common to all materials using the shader
	int common_tex_count;
	int common_cubemap_count;
	int loc_data; // uniform locations
	int loc_tex;
	int loc_cubemap;
	int loc_common_data; // values common to all materials using the shader
	int loc_common_tex;
	int loc_common_cubemap;
} nypx_shader;

enum nypx_framebuffer_slots {
	NYPX_FB_IGNORE = 0,
	NYPX_FB_DEPTH,
	NYPX_FB_STENCIL,
	NYPX_FB_DEPTH_STENCIL,
	NYPX_FB_COLOR
};

typedef struct nypx_fb_slot {
	nypx_tex tex;
	uint8_t type;
	uint8_t lod;
	uint8_t side;
} nypx_fb_slot;

typedef struct nypx_fb {
	nypx_res res;
	nypx_fb_slot slot[6];
} nypx_fb;

void nypx_tex_create(nypx_tex *t);
void nypx_tex_set(nypx_tex *t);
void nypx_tex_release(nypx_tex *t);

void nypx_mesh_create(nypx_mesh *m);
void nypx_mesh_use(nypx_mesh *m);
void nypx_mesh_set(nypx_mesh *m);
void nypx_mesh_release(nypx_mesh *m);

void nypx_shader_create(nypx_shader *s);
void nypx_shader_compile(nypx_shader *s);
void nypx_shader_use(nypx_shader *s);
void nypx_shader_set(nypx_shader *s, float *data, int *tex, int *cubemap);
void nypx_shader_set_common(nypx_shader *s, float *data, int *tex, int *cubemap);
void nypx_shader_release(nypx_shader *s);

void nypx_fb_create(nypx_fb *fb);
void nypx_fb_use(nypx_fb *fb);
void nypx_fb_set(nypx_fb *fb);
void nypx_fb_release(nypx_fb *fb);

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
