#ifndef NYAS_PIXELS_H
#define NYAS_PIXELS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * nyaspix config
 */

#ifdef NYAS_ELEM_SIZE_16
typedef uint16_t nyas_idx;
#else
typedef uint32_t nyas_idx;
#endif

#define NYAS_RENDER_QUEUE_CAPACITY 1024
#define NYAS_FRAME_POOL_SIZE (16 * 1024 * 1024)
#define NYAS_TEX_RESERVE 64
#define NYAS_MESH_RESERVE 64
#define NYAS_FB_RESERVE 32
#define NYAS_SHADER_RESERVE 32

/*
 * nyaspix resources
 */

typedef int nyas_resource_handle;
typedef nyas_resource_handle nyas_mesh;
typedef nyas_resource_handle nyas_tex;
typedef nyas_resource_handle nyas_framebuffer;
typedef nyas_resource_handle nyas_shader;

typedef struct nyas_mat {
	void *ptr;
	nyas_shader shader;
} nyas_mat;

typedef struct nyas_shader_desc {
	const char *name;
	int data_count;
	int tex_count;
	int cubemap_count;
	int common_data_count;
	int common_tex_count;
	int common_cubemap_count;
} nyas_shader_desc;

enum nyas_geometry {
	NYAS_QUAD,
	NYAS_CUBE,
	NYAS_SPHERE
};

void nyas_px_init(void);
void nyas_px_render(void);
void nyas_frame_end(void);
void *nyas_alloc_frame(unsigned int size);

#define NYAS_TEX_FLAGS(CHANNELS, FLOAT, LINEAR, CUBEMAP, DEPTH, TILE, MIPS) ( \
	(((CHANNELS) - 1) & 0x03) \
	| (TF_FLOAT * (FLOAT)) | (TF_CUBE * (CUBEMAP)) | (TF_DEPTH * (DEPTH)) \
	| (TF_TILING * (TILE)) | (TF_MIPMAP * (MIPS)) | (TF_LINEAR_COLOR * (LINEAR)) \
	| TF_MAG_FILTER_LERP | TF_MIN_FILTER_LERP | (TF_MAG_MIP_FILTER_LERP * (MIPS)))

int nyas_tex_flags(int nchann,
                   bool fp,
                   bool linear,
                   bool cube,
                   bool depth,
                   bool tile,
                   bool mipmap);
nyas_tex nyas_tex_empty(int width, int height, int tex_flags);
nyas_tex nyas_tex_load(const char *path, int flip, int tex_flags);
int *nyas_tex_size(nyas_tex tex, int *out_vec2i);

nyas_framebuffer nyas_fb_create(void);

nyas_mesh nyas_mesh_load_file(const char *path);
nyas_mesh nyas_mesh_load_geometry(enum nyas_geometry geo);
void nyas_mesh_reload_file(nyas_mesh mesh, const char *path);
void nyas_mesh_reload_geometry(nyas_mesh mesh, enum nyas_geometry geo);

nyas_shader nyas_shader_create(const nyas_shader_desc *desc);
void *nyas_shader_data(nyas_shader shader);
nyas_tex *nyas_shader_tex(nyas_shader shader);
nyas_tex *nyas_shader_cubemap(nyas_shader shader);
void nyas_shader_reload(nyas_shader shader);

/* Creates a new material and alloc persistent memory for its data */
nyas_mat nyas_mat_pers(nyas_shader shader);

/* Creates a new material and alloc frame-scoped memory for its data */
nyas_mat nyas_mat_tmp(nyas_shader shader);
nyas_mat nyas_mat_dft(nyas_shader shader);
nyas_mat nyas_mat_from_shader(nyas_shader shader);

/* MaterialAlloc does not initialize the shader value. */
void *nyas_mat_alloc(nyas_mat *mat);
nyas_tex *nyas_mat_tex(nyas_mat *mat); // Ptr to first texture.

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

typedef struct nyas_fb_slot {
	nyas_tex tex;
	int mip_level;
	int type;
	int face;
} nyas_fb_slot;

typedef struct nyas_set_fb_cmdata {
	nyas_framebuffer fb;
	int16_t vp_x;
	int16_t vp_y;
	nyas_fb_slot attach;
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
