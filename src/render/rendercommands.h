#ifndef THE_RENDER_COMMANDS_H
#define THE_RENDER_COMMANDS_H

#include "renderer.h"

#define THE_BLEND_FUNC_BIT     1
#define THE_ENABLE_BLEND_BIT   1 << 1
#define THE_WRITE_DEPTH_BIT    1 << 2
#define THE_DEPTH_TEST_BIT     1 << 3
#define THE_CULL_FACE_BIT      1 << 4

typedef struct {
	float color[4];
	int8_t bcolor;
	int8_t bdepth;
	int8_t bstencil;
} THE_ClearCommandData;

typedef struct {
	THE_Texture cubemap;
} THE_SkyboxCommandData;

typedef struct {
	THE_Mesh mesh;
	THE_Shader shader;
	THE_Material mat;
	int *inst_attr;
	uint32_t inst_count;
} THE_DrawCommandData;

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
};

typedef struct {
	uint32_t changed_mask;
	enum THE_RenderOptions sfactor;
	enum THE_RenderOptions dfactor;
	enum THE_RenderOptions cull_face;
	int8_t depth_test;
	int8_t write_depth;
	int8_t blend;
} THE_RenderOptionsData;

typedef struct {
	THE_Framebuffer fb;
} THE_UseFramebufferData;

typedef struct {
	THE_Shader shader;
	THE_Material material;
} THE_UseShaderData;

typedef union {
	THE_ClearCommandData clear;
	THE_SkyboxCommandData skybox;
	THE_DrawCommandData draw;
	THE_EquirectToCubeData eqr_cube;
	THE_RenderOptionsData renderops;
	THE_UseFramebufferData usefb;
	THE_UseShaderData use_shader;
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
extern void THE_SkyboxExecute(THE_CommandData *data);
extern void THE_DrawExecute(THE_CommandData *data);
extern void THE_EquirectToCubeExecute(THE_CommandData *data);
extern void THE_RenderOptionsExecute(THE_CommandData *data);
extern void THE_UseFramebufferExecute(THE_CommandData *data);
extern void THE_UseShaderExecute(THE_CommandData *data);

#endif
