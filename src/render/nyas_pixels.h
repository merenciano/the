#ifndef NYAS_PIXELS_H
#define NYAS_PIXELS_H

#include <stdbool.h>

typedef int nyas_mesh;
typedef int nyas_tex;
typedef int nyas_framebuffer;
typedef int nyas_shader;

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

typedef void nyas_cmd;
void nyas_cmd_add(nyas_cmd *rc);
nyas_cmd *nyas_cmd_alloc(void);

nyas_tex nyas_tex_create(int width, int height, enum nyas_textype t);
nyas_tex nyas_tex_load_img(const char *path, enum nyas_textype t);
int *nyas_tex_size(nyas_tex tex, int *out);

nyas_framebuffer nyas_fb_create(int width, int height, bool color, bool depth);
nyas_tex nyas_fb_color(nyas_framebuffer fb);
void nyas_fb_size(nyas_framebuffer fb, int *o_w, int *o_h);

nyas_mesh nyas_mesh_create_cube(void);
nyas_mesh nyas_mesh_create_sphere(int x_segments, int y_segments);
nyas_mesh nyas_mesh_create_quad(void);
nyas_mesh nyas_mesh_load_obj(const char *path);

nyas_shader nyas_shader_create(const char *shader);
nyas_mat nyas_mat_default(void);
void *nyas_mat_alloc(nyas_mat *mat); /* MaterialAlloc does not initialize the shader value. */
void *nyas_mat_alloc_frame(nyas_mat *mat); /* MaterialAllocFrame does not initialize the shader value. */

#endif //NYAS_PIXELS_H
