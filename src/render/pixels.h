#ifndef NYAS_PIXELS_H
#define NYAS_PIXELS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * nyaspix config
 */

#ifdef NYAS_ELEM_ARRAY_32
#define IDX_T unsigned int
#elif defined(NYAS_ELEM_ARRAY_16)
#include <stdint.h>
#define IDX_T uint16_t
#else
#define NYAS_ELEM_ARRAY_32
#define IDX_T unsigned int
#endif

#define NYAS_PIXEL_CHECKS

#define NYAS_RENDER_QUEUE_CAPACITY 1024
#define NYAS_FRAME_POOL_SIZE (16 * 1024 * 1024)
#define NYAS_TEX_RESERVE 64
#define NYAS_MESH_RESERVE 64
#define NYAS_FB_RESERVE 32
#define NYAS_SHADER_RESERVE 32

/*
 * nyaspix resources
 */

typedef int nyas_mesh;
typedef int nyas_tex;
typedef int nyas_framebuffer;
typedef int nyas_shader;

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

void nyas_px_init(void);
void nyas_px_render(void);
void nyas_frame_end(void);
void *nyas_alloc_frame(unsigned int size);

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

extern nyas_mesh SPHERE_MESH;
extern nyas_mesh CUBE_MESH;
extern nyas_mesh QUAD_MESH;

/*
 * nyaspix commands
 */

typedef struct nyas_clear_cmdata {
	float color[4];
	bool color_buffer;
	bool depth_buffer;
	bool stencil_buffer;
} nyas_clear_cmdata;

typedef struct nyas_draw_cmdata {
	nyas_mesh mesh;
	nyas_mat material;
} nyas_draw_cmdata;

enum nyas_blendfn_opt {
	// TODO: Add as needed.
	NYAS_BLEND_FUNC_INVALID = 0,
	NYAS_BLEND_FUNC_ONE,
	NYAS_BLEND_FUNC_SRC_ALPHA,
	NYAS_BLEND_FUNC_ONE_MINUS_SRC_ALPHA,
	NYAS_BLEND_FUNC_ZERO,
};

typedef struct nyas_blend_fn {
	enum nyas_blendfn_opt src;
	enum nyas_blendfn_opt dst;
} nyas_blend_fn;

typedef enum nyas_cullface_opt {
	NYAS_CULL_FACE_CURRENT = 0,
	NYAS_CULL_FACE_FRONT,
	NYAS_CULL_FACE_BACK,
	NYAS_CULL_FACE_FRONT_AND_BACK,
} nyas_cullface_opt;

typedef enum nyas_depthfn_opt {
	// TODO: Add as needed.
	NYAS_DEPTH_FUNC_CURRENT = 0,
	NYAS_DEPTH_FUNC_LEQUAL,
	NYAS_DEPTH_FUNC_LESS,
} nyas_depthfn_opt;

typedef enum nyas_rops_opt {
	NYAS_BLEND = 1 << 0,
	NYAS_CULL_FACE = 1 << 1,
	NYAS_DEPTH_TEST = 1 << 2,
	NYAS_DEPTH_WRITE = 1 << 3,
	NYAS_REND_OPTS_COUNT
} nyas_rops_opt;

typedef struct nyas_rops_cmdata {
	int enable_flags;
	int disable_flags;
	nyas_blend_fn blend_func;
	nyas_cullface_opt cull_face;
	nyas_depthfn_opt depth_func;
} nyas_rops_cmdata;

typedef enum nyas_attach_slot {
	NYAS_ATTACH_IGNORE,
	NYAS_ATTACH_DEPTH,
	NYAS_ATTACH_COLOR
} nyas_attach_slot;

typedef struct nyas_fbattach {
	nyas_tex tex;
	nyas_attach_slot slot;
	int level;
	int side;
} nyas_fbattach;

typedef struct nyas_set_fb_cmdata {
	nyas_framebuffer fb;
	int16_t vp_x;
	int16_t vp_y;
	nyas_fbattach attachment;
} nyas_set_fb_cmdata;

typedef union nyas_cmdata {
	nyas_clear_cmdata clear;
	nyas_draw_cmdata draw;
	nyas_rops_cmdata rend_opts;
	nyas_mat mat;
	nyas_set_fb_cmdata set_fb;
} nyas_cmdata;

typedef struct nyas_cmd {
	struct nyas_cmd *next;
	void (*execute)(nyas_cmdata *data);
	nyas_cmdata data;
} nyas_cmd;

void nyas_cmd_add(nyas_cmd *rc);
nyas_cmd *nyas_cmd_alloc(void);

extern void nyas_clear_fn(nyas_cmdata *data);
extern void nyas_draw_fn(nyas_cmdata *data);
extern void nyas_rops_fn(nyas_cmdata *data);
extern void nyas_setshader_fn(nyas_cmdata *data);
extern void nyas_setfb_fn(nyas_cmdata *data);

#endif // NYAS_PIXELS_H
