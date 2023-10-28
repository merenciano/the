#ifndef THE_RENDER_COMMANDS_H
#define THE_RENDER_COMMANDS_H

#include "renderer.h"

#define THE_BLEND_FUNC_BIT     1U
#define THE_ENABLE_BLEND_BIT   1U << 1U
#define THE_WRITE_DEPTH_BIT    1U << 2U
#define THE_DEPTH_TEST_BIT     1U << 3U
#define THE_CULL_FACE_BIT      1U << 4U
#define THE_DEPTH_FUNC_BIT     1U << 5U

typedef struct {
	float color[4];
	bool color_buffer;
	bool depth_buffer;
	bool stencil_buffer;
} THE_ClearData;

typedef struct {
	THE_Mesh mesh;
	THE_Shader shader;
	THE_Material mat;
	int *inst_attr;
	uint32_t inst_count;
} THE_DrawCommandData;

typedef struct THE_DrawData {
	THE_Mesh mesh;
	THE_Mat material;
} THE_DrawData;

typedef struct {
	char in_path[128];
	THE_Texture out_cube;
	THE_Texture out_prefilt;
	THE_Texture out_lut;
} THE_EquirectToCubeData;

enum THE_RenderOptions {
	THE_BLENDFUNC_ONE,
	THE_BLENDFUNC_SRC_ALPHA,
	THE_BLENDFUNC_ONE_MINUS_SRC_ALPHA,
	THE_BLENDFUNC_ZERO,
	THE_CULLFACE_DISABLED,
	THE_CULLFACE_FRONT,
	THE_CULLFACE_BACK,
	THE_CULLFACE_FRONT_AND_BACK,
	THE_DEPTHFUNC_LEQUAL,
	THE_DEPTHFUNC_LESS,
};

typedef struct {
	uint32_t changed_mask;
	enum THE_RenderOptions sfactor;
	enum THE_RenderOptions dfactor;
	enum THE_RenderOptions cull_face;
	enum THE_RenderOptions depth_func;
	bool depth_test;
	bool write_depth;
	bool blend;
} THE_RenderOptionsData;

typedef union {
	THE_ClearData clear;
	THE_DrawData draw;
	THE_EquirectToCubeData eqr_cube;
	THE_RenderOptionsData renderops;
	THE_Mat mat;
	THE_Framebuffer usefb;
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

uint32_t THE_RenderQueueUsed(void);

extern void THE_ClearExecute(THE_CommandData *data);
extern void THE_DrawExecute(THE_CommandData *data);
extern void THE_EquirectToCubeExecute(THE_CommandData *data);
extern void THE_RenderOptionsExecute(THE_CommandData *data);
extern void THE_UseFramebufferExecute(THE_CommandData *data);
extern void THE_UseShaderExecute(THE_CommandData *data);

#endif
