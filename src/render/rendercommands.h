#ifndef THE_RENDER_COMMANDS_H
#define THE_RENDER_COMMANDS_H

#include "material.h"

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
	THE_Shader newmat;
	THE_Material matdata;
	THE_Buffer inst_attr;
	uint32_t inst_count;
} THE_DrawCommandData;

typedef struct {
	char in_path[128];
	THE_Texture out_cube;
	THE_Texture out_prefilt;
	THE_Texture out_lut;
} THE_EquirectToCubeData;

typedef enum {
	THE_BLENDFUNC_ONE,
	THE_BLENDFUNC_SRC_ALPHA,
	THE_BLENDFUNC_ONE_MINUS_SRC_ALPHA,
	THE_BLENDFUNC_ZERO,
	THE_CULLFACE_DISABLED,
	THE_CULLFACE_FRONT,
	THE_CULLFACE_BACK,
	THE_CULLFACE_FRONT_AND_BACK,
} THE_RenderOptions;

typedef struct {
	uint32_t changed_mask;
	THE_RenderOptions sfactor;
	THE_RenderOptions dfactor;
	THE_RenderOptions cull_face;
	int8_t depth_test;
	int8_t write_depth;
	int8_t blend;
} THE_RenderOptionsData;

typedef struct {
	THE_Framebuffer fb;
} THE_UseFramebufferData;

typedef struct {
	THE_Shader mat;
	THE_Material data;
} THE_UseMaterialData;

typedef union {
	THE_ClearCommandData clear;
	THE_SkyboxCommandData skybox;
	THE_DrawCommandData draw;
	THE_EquirectToCubeData eqr_cube;
	THE_RenderOptionsData renderops;
	THE_UseFramebufferData usefb;
	THE_UseShaderData usemat;
} THE_CommandData;

typedef struct THE_RenderCommand {
	void (*execute)(THE_CommandData *data);
	struct THE_RenderCommand *next;
	THE_CommandData data;
} THE_RenderCommand;

extern void THE_ClearExecute(THE_CommandData *data);
extern void THE_SkyboxExecute(THE_CommandData *data);
extern void THE_DrawExecute(THE_CommandData *data);
extern void THE_EquirectToCubeExecute(THE_CommandData *data);
extern void THE_RenderOptionsExecute(THE_CommandData *data);
extern void THE_UseFramebufferExecute(THE_CommandData *data);
extern void THE_UseShaderExecute(THE_CommandData *data);

#endif
