#ifndef NYASPIX_H
#define NYASPIX_H

#include "pixels.h"
#include <core/common.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
	nyas_texture_face face;
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

void nypx_tex_create(struct nyas_texture_internal *t);
void nypx_tex_set(struct nyas_texture_internal *t);
void nypx_tex_release(uint32_t *id);

void nypx_mesh_create(uint32_t *id, uint32_t *vid, uint32_t *iid);
void nypx_mesh_use(struct nyas_mesh_internal *m, struct nyas_shader_internal *s);
void nypx_mesh_set(struct nyas_mesh_internal *mesh, uint32_t shader_id);
void nypx_mesh_release(uint32_t *id, uint32_t *vid, uint32_t *iid);

void nypx_shader_create(uint32_t *id);
void nypx_shader_compile(uint32_t id, const char *name);
void nypx_shader_use(uint32_t id);
void nypx_shader_release(uint32_t id);
void nypx_shader_loc(uint32_t id, int *o_loc, const char **i_unif, int count);
void nypx_shader_set_data(int loc, float *data, int v4count);
void nypx_shader_set_tex(int loc, int *tex, int count, int texunit_offset);
void nypx_shader_set_cube(int loc, int *tex, int count, int texunit_offset);

void nypx_fb_create(struct nyas_framebuffer_internal *fb);
void nypx_fb_set(uint32_t fb_id, uint32_t tex_id, struct nyas_texture_target *tt);
void nypx_fb_use(uint32_t id);
void nypx_fb_release(struct nyas_framebuffer_internal *fb);

void nypx_clear(bool color, bool depth, bool stencil);
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
void nypx_viewport(struct nyas_rect rect);
void nypx_scissor(struct nyas_rect rect);

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

#endif // NYASPIX_H
