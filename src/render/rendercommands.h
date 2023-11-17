#ifndef NYAS_PIXEL_COMMANDS_H
#define NYAS_PIXEL_COMMANDS_H

#include "renderer.h"

#include <stdint.h>

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
	void (*execute)(nyas_cmdata *data);
	struct nyas_cmd *next;
	nyas_cmdata data;
} nyas_cmd;

void nyas_cmd_add(nyas_cmd *rc);
nyas_cmd *nyas_cmd_alloc(void);

typedef struct nyas_cmd_queue {
	nyas_cmd *curr;
	nyas_cmd *curr_last;
	nyas_cmd *next;
	nyas_cmd *next_last;
} nyas_cmd_queue;

extern void nyas_clear_fn(nyas_cmdata *data);
extern void nyas_draw_fn(nyas_cmdata *data);
extern void nyas_rops_fn(nyas_cmdata *data);
extern void nyas_setshader_fn(nyas_cmdata *data);
extern void nyas_setfb_fn(nyas_cmdata *data);

#endif // NYAS_PIXEL_COMMANDS_H
