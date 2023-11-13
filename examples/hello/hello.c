#include "the.h"
#include <mathc.h>
#include <string.h>

typedef struct {
	float model[16];
	float vp[16];
	float color[4];
} HelloMatData;

typedef struct HelloCtx {
	HelloMatData hello_mat;
	THE_Shader hellomat;
	THE_Shader fs_img;
	THE_Shader skybox;
	THE_Framebuffer fb;
	THE_Texture skycube;
	THE_Material fs_mat;
	THE_Material skymat;
	THE_Entity *e;
} HelloCtx;

#define NYAS_ARR_TEST
#include "utils/array.h"

void
Init(void *context)
{
	nyas_arr_test();
	HelloCtx *ctx = context;
	ctx->fb = THE_CreateFramebuffer(THE_WindowGetWidth(),
	                                THE_WindowGetHeight(), true, true);
	ctx->hellomat = THE_CreateShader("hello");
	ctx->fs_img = THE_CreateShader("fullscreen-img");
	ctx->skybox = THE_CreateShader("skybox");
	ctx->fs_mat.data_count = 0;
	ctx->fs_mat.tex_count = 1;
	ctx->fs_mat.cube_count = 0;
	THE_Texture *t = THE_MaterialAlloc(&ctx->fs_mat);
	*t = THE_GetFrameColor(ctx->fb);
	ctx->fs_mat.shader = ctx->fs_img;

	ctx->skycube = THE_CreateTextureFromFile("./assets/tex/Xcave.png",
	                                         THE_TEX_SKYBOX);

	ctx->skymat.data_count = 16;
	ctx->skymat.tex_count = 0;
	ctx->skymat.cube_count = 1;
	ctx->skymat.shader = ctx->skybox;
	THE_Texture *cube = THE_MaterialAlloc(&ctx->skymat);
	cube[ctx->skymat.data_count] = ctx->skycube;

	ctx->hello_mat.color[0] = 1.0f;
	ctx->hello_mat.color[1] = 1.0f;
	ctx->hello_mat.color[2] = 0.0f;
	ctx->hello_mat.color[3] = 1.0f;

	ctx->e = THE_EntityCreate();
	float pos[3] = { 0.0f, 0.0f, -4.0f };
	mat4_translation(ctx->e->transform, mat4_identity(ctx->e->transform), pos);
	ctx->e->mesh = SPHERE_MESH;
	ctx->e->mat.data_count = sizeof(HelloMatData) / 4;
	ctx->e->mat.tex_count = 0;
	ctx->e->mat.cube_count = 0;
	HelloMatData *mat_data = THE_MaterialAlloc(&ctx->e->mat);
	ctx->e->mat.shader = ctx->hellomat;
	*mat_data = ctx->hello_mat;
}

bool
Update(void *context)
{
	HelloCtx *ctx = context;
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, deltatime);

	THE_RenderCommand *fbuff = THE_AllocateCommand();
	fbuff->data.set_fb.fb = ctx->fb;
	fbuff->data.set_fb.attachment.slot = THE_IGNORE;
	fbuff->execute = THE_SetFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	fbuff->next = rops;
	rops->data.rend_opts.enable_flags = THE_BLEND | THE_DEPTH_TEST |
	  THE_DEPTH_WRITE | THE_CULL_FACE;
	rops->data.rend_opts.blend_func.src = THE_BLEND_FUNC_ONE;
	rops->data.rend_opts.blend_func.dst = THE_BLEND_FUNC_ZERO;
	rops->data.rend_opts.cull_face = THE_CULL_FACE_BACK;
	rops->data.rend_opts.depth_func = THE_DEPTH_FUNC_LESS;
	rops->execute = THE_RenderOptionsExecute;

	mat4_multiply(((HelloMatData *)ctx->e->mat.ptr)->vp, camera.proj_mat,
	              camera.view_mat);

	THE_RenderCommand *clear = THE_AllocateCommand();
	rops->next = clear;
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = true;
	clear->data.clear.stencil_buffer = false;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;

	THE_RenderCommand *usemat = THE_AllocateCommand();
	usemat->data.mat = THE_MaterialDefault();
	usemat->data.mat.shader = ctx->hellomat;
	usemat->execute = THE_UseShaderExecute;
	clear->next = usemat;
	usemat->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	rops = THE_AllocateCommand();
	rops->data.rend_opts.disable_flags = THE_CULL_FACE;
	rops->data.rend_opts.depth_func = THE_DEPTH_FUNC_LEQUAL;
	rops->execute = THE_RenderOptionsExecute;

	THE_CameraStaticViewProjection(ctx->skymat.ptr, &camera);
	THE_RenderCommand *use_sky_shader = THE_AllocateCommand();
	use_sky_shader->data.mat = ctx->skymat;
	use_sky_shader->execute = THE_UseShaderExecute;
	rops->next = use_sky_shader;

	THE_RenderCommand *draw_sky = THE_AllocateCommand();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material = THE_MaterialDefault();
	draw_sky->data.draw.material.shader = ctx->skybox;
	draw_sky->execute = THE_DrawExecute;
	use_sky_shader->next = draw_sky;

	THE_RenderCommand *fbuff2 = THE_AllocateCommand();
	fbuff2->data.set_fb.fb = THE_DEFAULT;
	fbuff2->data.set_fb.attachment.slot = THE_IGNORE;
	fbuff2->execute = THE_SetFramebufferExecute;
	draw_sky->next = fbuff2;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	fbuff2->next = rops2;
	rops2->data.rend_opts.disable_flags = THE_DEPTH_TEST;
	rops2->execute = THE_RenderOptionsExecute;

	THE_RenderCommand *clear2 = THE_AllocateCommand();
	clear2->data.clear.color_buffer = true;
	clear2->data.clear.depth_buffer = false;
	clear2->data.clear.stencil_buffer = false;
	clear2->data.clear.color[0] = 0.0f;
	clear2->data.clear.color[1] = 1.0f;
	clear2->data.clear.color[2] = 1.0f;
	clear2->data.clear.color[3] = 1.0f;
	clear2->execute = THE_ClearExecute;
	rops2->next = clear2;

	THE_RenderCommand *usefullscreen = THE_AllocateCommand();
	usefullscreen->data.mat = ctx->fs_mat;
	usefullscreen->execute = THE_UseShaderExecute;
	clear2->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material = ctx->fs_mat;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(rops);

	return true;
}

int
main(int argc, char **argv)
{
	HelloCtx ctx;
	struct THE_Config cnfg = { .init_func = Init,
		                       .update_func = Update,
		                       .context = &ctx,
		                       .heap_size = THE_GB(1U),
		                       .window_title = "THE_Hello",
		                       .window_width = 1280,
		                       .window_height = 720,
		                       .vsync = true };

	THE_Start(&cnfg);
	THE_End();

	return 0;
}
