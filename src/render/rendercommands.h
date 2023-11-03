#ifndef THE_RENDER_COMMANDS_H
#define THE_RENDER_COMMANDS_H

#include "renderer.h"

#include <stdint.h>

typedef struct {
	float color[4];
	bool color_buffer;
	bool depth_buffer;
	bool stencil_buffer;
} THE_ClearData;

typedef struct THE_DrawData {
	THE_Mesh mesh;
	THE_Material material;
} THE_DrawData;

enum THE_BlendFuncOpt {
	// TODO: Add as needed.
	THE_BLEND_FUNC_INVALID = 0,
	THE_BLEND_FUNC_ONE,
	THE_BLEND_FUNC_SRC_ALPHA,
	THE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA,
	THE_BLEND_FUNC_ZERO,
};

typedef struct THE_BlendFunc {
	enum THE_BlendFuncOpt src;
	enum THE_BlendFuncOpt dst;
} THE_BlendFunc;

typedef enum THE_CullFace {
	THE_CULL_FACE_CURRENT = 0,
	THE_CULL_FACE_FRONT,
	THE_CULL_FACE_BACK,
	THE_CULL_FACE_FRONT_AND_BACK,
} THE_CullFace;

typedef enum THE_DepthFunc {
	// TODO: Add as needed.
	THE_DEPTH_FUNC_CURRENT = 0,
	THE_DEPTH_FUNC_LEQUAL,
	THE_DEPTH_FUNC_LESS,
} THE_DepthFunc;

typedef enum THE_RenderOptions {
	THE_BLEND = 1 << 0,
	THE_CULL_FACE = 1 << 1,
	THE_DEPTH_TEST = 1 << 2,
	THE_DEPTH_WRITE = 1 << 3,
	THE_REND_OPTS_COUNT
} THE_RenderOptions;

typedef struct {
	int enable_flags;
	int disable_flags;
	THE_BlendFunc blend_func;
	THE_CullFace cull_face;
	THE_DepthFunc depth_func;
} THE_RenderOptionsData;

typedef enum THE_AttachSlot {
	THE_ATTACH_IGNORE,
	THE_ATTACH_DEPTH,
	THE_ATTACH_COLOR
} THE_AttachSlot;

typedef struct THE_FBAttachment {
	THE_Texture tex;
	THE_AttachSlot slot;
	int level;
	int side;
} THE_FBAttachment;

typedef struct THE_SetFramebufferData {
	THE_Framebuffer fb;
	int16_t vp_x;
	int16_t vp_y;
	THE_FBAttachment attachment;
} THE_SetFramebufferData;

typedef union {
	THE_ClearData clear;
	THE_DrawData draw;
	THE_RenderOptionsData rend_opts;
	THE_Material mat;
	THE_SetFramebufferData set_fb;
} THE_CommandData;

struct THE_RenderCommand {
	void (*execute)(THE_CommandData *data);
	struct THE_RenderCommand *next;
	THE_CommandData data;
};

typedef struct THE_RenderCommand THE_RenderCommand;

void THE_AddCommands(THE_RenderCommand *rc);
THE_RenderCommand *THE_AllocateCommand(void);

typedef struct {
	THE_RenderCommand *curr;
	THE_RenderCommand *curr_last;
	THE_RenderCommand *next;
	THE_RenderCommand *next_last;
} THE_RenderQueue;

extern void THE_ClearExecute(THE_CommandData *data);
extern void THE_DrawExecute(THE_CommandData *data);
extern void THE_RenderOptionsExecute(THE_CommandData *data);
extern void THE_UseShaderExecute(THE_CommandData *data);
extern void THE_SetFramebufferExecute(THE_CommandData *data);

#endif
