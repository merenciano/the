#ifndef NYASPIX_H
#define NYASPIX_H

#include "pixels_defs.h"

#include <stddef.h>
#include <stdint.h>

// TODO: Glue nyas and nypx common types better
#ifdef NYAS_ELEM_SIZE_16
typedef uint16_t nypx_index;
#else
typedef uint32_t nypx_index;
#endif

#define NYPX_HALF_INDEX 0
#define NYPX_WORD_INDEX 1

enum nyas_resource_flags {
	NYAS_IRF_DIRTY = 1U << 3,
	NYAS_IRF_CREATED = 1U << 4,
	NYAS_IRF_RELEASE_APP_STORAGE = 1U << 5,
	NYAS_IRF_MAPPED = 1U << 7,
};

enum nypx_vertex_attributes {
	VA_POSITION = 0,
	VA_NORMAL,
	VA_TANGENT,
	VA_BITANGENT,
	VA_UV,
	VTXATTR_COUNT
};

struct nyas_resource_internal {
	uint32_t id;
	int flags;
};

struct nyas_mesh_internal {
	struct nyas_resource_internal res;
	struct nyas_resource_internal res_vb; // vertex buffer resource
	struct nyas_resource_internal res_ib; // index buffer resource
	float *vtx;
	nypx_index *idx;
	int64_t elem_count;
	uint32_t vtx_size;
	int attrib;
};

struct nyas_texture_image {
	void *pix;
	nyas_texture_face face;
	int lod;
};

struct nyas_texture_internal {
	struct nyas_resource_internal res;
	struct nyas_texture_desc data;
	struct nyas_texture_image *img; // nyas_arr
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

/*static struct nyas_framebuffer_internal
nyas_framebuffer_internal(void)
{
	return (struct nyas_framebuffer_internal){
		.res = { .id = 0, .flags = NYAS_IRF_DIRTY },
		.color = {
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0},
		  { .tex = NYAS_NONE, .face = NYAS_FACE_2D, .lod_level = 0}
		},
		.depth = NYAS_NONE,
		.stencil = NYAS_NONE
	};
}*/

void nypx_tex_create(struct nyas_texture_internal *t);
void nypx_tex_set(struct nyas_texture_internal *t);

void nypx_tex_release(uint32_t *id);

void nypx_mesh_create(uint32_t *id, uint32_t *vid, uint32_t *iid);

void nypx_mesh_use(struct nyas_mesh_internal *m, struct nyas_shader_internal *s);

void nypx_mesh_set(
  uint32_t id,
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

void nypx_fb_create(struct nyas_framebuffer_internal *fb);

void nypx_fb_set(struct nyas_framebuffer_internal *fb, int index);

void nypx_fb_use(struct nyas_framebuffer_internal *fb);

void nypx_fb_release(struct nyas_framebuffer_internal *fb);

void nypx_clear(int color, int depth, int stencil);

void nypx_draw(int elem_count, int index_type);

void nypx_clear_color(float r, float g, float b, float a);

void nypx_scissor_enable(void);

void nypx_scissor_disable(void);

void nypx_blend_enable(void);

void nypx_blend_disable(void);

void nypx_blend_set(int blend_func_src, int blend_func_dst);

void nypx_cull_enable(void);

void nypx_cull_disable(void);

void nypx_cull_set(int cull_face);

void nypx_depth_enable_test(void);

void nypx_depth_disable_test(void);

void nypx_depth_enable_mask(void);

void nypx_depth_disable_mask(void);

void nypx_depth_set(int depth_func);

void nypx_stencil_enable_test(void);

void nypx_stencil_disable_test(void);

void nypx_stencil_enable_mask(void);

void nypx_stencil_disable_mask(void);

void nypx_stencil_set(int stencil_func);

void nypx_viewport(struct nyas_rect rect);
void nypx_scissor(struct nyas_rect rect);

extern struct nyas_mesh_internal *mesh_pool;
extern struct nyas_texture_internal *tex_pool;
extern struct nyas_shader_internal *shader_pool;
extern struct nyas_framebuffer_internal *framebuffer_pool;

#endif // NYASPIX_H
