#ifndef NYAS_PIXEL_RENDERER_H
#define NYAS_PIXEL_RENDERER_H

#include "config.h"
#include <stdbool.h>

struct nyas_pbr_desc_unit {
	float model[16];
	float color[3];
	float use_albedo_map;
	float use_pbr_maps;
	float tiling_x;
	float tiling_y;
	float padding;
	float roughness;
	float metallic;
	float normal_map_intensity;
	float paddingg;
};

struct nyas_pbr_desc_scene {
	float view_projection[16];
	float camera_position[3];
	float padding;
	float sunlight[4];
};

enum nyas_textype {
	NYAS_TEX_NONE = 0,
	NYAS_TEX_R,
	NYAS_TEX_RGB,
	NYAS_TEX_SRGB,
	NYAS_TEX_DEPTH,
	NYAS_TEX_SKYBOX,
	NYAS_TEX_RGB_F16,
	NYAS_TEX_RGBA_F16,
	NYAS_TEX_LUT,
	NYAS_TEX_ENVIRONMENT,
	NYAS_TEX_PREFILTER_ENVIRONMENT,
};

typedef int nyas_mesh;
typedef int nyas_tex;
typedef int nyas_framebuffer;
typedef int nyas_shader;

typedef struct nyas_mat {
	void *ptr;
	int data_count;
	int tex_count;
	int cube_count;
	nyas_shader shader;
} nyas_mat;

extern nyas_mesh SPHERE_MESH;
extern nyas_mesh CUBE_MESH;
extern nyas_mesh QUAD_MESH;

void nyas_px_init(void);
void nyas_px_render(void);
void nyas_frame_end(void);
void *nyas_alloc_frame(unsigned int size);

nyas_tex nyas_tex_load_img(const char *path, enum nyas_textype t);
nyas_tex nyas_tex_create(int width, int height, enum nyas_textype t);
int *nyas_tex_size(nyas_tex tex, int *out);
void nyas_tex_freepix(nyas_tex tex); // From RAM

nyas_mesh nyas_mesh_create_cube(void);
nyas_mesh nyas_mesh_create_sphere(int x_segments, int y_segments);
nyas_mesh nyas_mesh_create_quad(void);
nyas_mesh nyas_mesh_load_obj(const char *path);

nyas_shader nyas_shader_create(const char *shader);

// TODO: ints i soportar varios
nyas_framebuffer nyas_fb_create(int width, int height, bool color, bool depth);
nyas_tex nyas_fb_color(nyas_framebuffer fb);
void nyas_fb_size(nyas_framebuffer fb, int *o_w, int *o_h);

nyas_mat nyas_mat_default(void);
/* MaterialAlloc does not initialize the shader value. */
void *nyas_mat_alloc(nyas_mat *mat);

/* MaterialAllocFrame does not initialize the shader value. */
void *nyas_mat_alloc_frame(nyas_mat *mat);

#endif // NYAS_PIXEL_RENDERER_H
