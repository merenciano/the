#ifndef THEPIX_H
#define THEPIX_H

#include "pixels.h"
#include <core/common.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int the_resource_flags;
enum the_resource_flags {
	THE_IRF_DIRTY = 1U << 3,
	THE_IRF_CREATED = 1U << 4,
	THE_IRF_RELEASE_APP_STORAGE = 1U << 5,
	THE_IRF_MAPPED = 1U << 7,
};

struct the_resource_internal {
	uint32_t id;
	the_resource_flags flags;
};

struct the_mesh_internal {
	struct the_resource_internal res;
	struct the_resource_internal res_vb; // vertex buffer resource
	struct the_resource_internal res_ib; // index buffer resource
	float *vtx;
	the_idx *idx;
	int64_t elem_count;
	uint32_t vtx_size;
	the_vertex_attrib attrib;
};

struct the_texture_image {
	void *pix;
	the_texture_face face;
	int lod;
};

typedef struct the_texture_image theteximg;
THE_DECL_ARR(theteximg);

struct the_texture_internal {
	struct the_resource_internal res;
	struct the_texture_desc data;
	struct thearr_theteximg *img;
};

struct the_shader_internal {
	struct the_resource_internal res;
	const char *name;
	struct {
		int data, tex, cubemap;
	} loc[2], count[2]; // 0: unit, 1: common
	void *common;
};

struct the_framebuffer_internal {
	struct the_resource_internal res;
	struct the_texture_target target[8];
};

void thepx_tex_create(struct the_texture_internal *t);
void thepx_tex_set(struct the_texture_internal *t);
void thepx_tex_release(uint32_t *id);

void thepx_mesh_create(uint32_t *id, uint32_t *vid, uint32_t *iid);
void thepx_mesh_use(struct the_mesh_internal *m, struct the_shader_internal *s);
void thepx_mesh_set(struct the_mesh_internal *mesh, uint32_t shader_id);
void thepx_mesh_release(uint32_t *id, uint32_t *vid, uint32_t *iid);

void thepx_shader_create(uint32_t *id);
void thepx_shader_compile(uint32_t id, const char *name);
void thepx_shader_use(uint32_t id);
void thepx_shader_release(uint32_t id);
void thepx_shader_loc(uint32_t id, int *o_loc, const char **i_unif, int count);
void thepx_shader_set_data(int loc, float *data, int v4count);
void thepx_shader_set_tex(int loc, int *tex, int count, int texunit_offset);
void thepx_shader_set_cube(int loc, int *tex, int count, int texunit_offset);

void thepx_fb_create(struct the_framebuffer_internal *fb);
void thepx_fb_set(uint32_t fb_id, uint32_t tex_id, struct the_texture_target *tt);
void thepx_fb_use(uint32_t id);
void thepx_fb_release(struct the_framebuffer_internal *fb);

void thepx_clear(bool color, bool depth, bool stencil);
void thepx_draw(int elem_count, int index_type);
void thepx_clear_color(float r, float g, float b, float a);
void thepx_scissor_enable(void);
void thepx_scissor_disable(void);
void thepx_blend_enable(void);
void thepx_blend_disable(void);
void thepx_blend_set(int blend_func_src, int blend_func_dst);
void thepx_cull_enable(void);
void thepx_cull_disable(void);
void thepx_cull_set(int cull_face);
void thepx_depth_enable_test(void);
void thepx_depth_disable_test(void);
void thepx_depth_enable_mask(void);
void thepx_depth_disable_mask(void);
void thepx_depth_set(int depth_func);
void thepx_stencil_enable_test(void);
void thepx_stencil_disable_test(void);
void thepx_stencil_enable_mask(void);
void thepx_stencil_disable_mask(void);
void thepx_viewport(struct the_rect rect);
void thepx_scissor(struct the_rect rect);

typedef struct the_mesh_internal mesh;
THE_DECL_ARR(mesh);
THE_DECL_POOL(mesh);

typedef struct the_texture_internal tex;
THE_DECL_ARR(tex);
THE_DECL_POOL(tex);

typedef struct the_shader_internal shad;
THE_DECL_ARR(shad);
THE_DECL_POOL(shad);

typedef struct the_framebuffer_internal fb;
THE_DECL_ARR(fb);
THE_DECL_POOL(fb);

extern struct thepool_mesh mesh_pool;
extern struct thepool_tex tex_pool;
extern struct thepool_shad shader_pool;
extern struct thepool_fb framebuffer_pool;

#endif // THEPIX_H
