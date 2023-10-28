#include "the.h"
#include <string.h>
#include <mathc.h>

typedef struct {
	union {
		float buffer[16+16+4];
		struct {
			float model[16];
			float vp[16];
			float color[4];
		} fields;
	} data;
} HelloMatData;

typedef struct HelloCtx {
	HelloMatData hello_mat;
	THE_Shader hellomat;
	THE_Shader fs_img;
	THE_Shader skybox;
	THE_Framebuffer fb;
	THE_Texture skycube;
	THE_Entity *e;

} HelloCtx;

void Init(void *context)
{
	HelloCtx *ctx = context;
	ctx->fb = THE_CreateFramebuffer(THE_WindowGetWidth(), THE_WindowGetHeight(), true, true);
	ctx->hellomat = THE_CreateShader("hello");
	ctx->fs_img = THE_CreateShader("fullscreen-img");
	ctx->skybox = THE_CreateShader("skybox");
	THE_Texture fb_color = THE_GetFrameColor(ctx->fb);
	THE_MaterialSetTexture(THE_ShaderCommonData(ctx->fs_img), &fb_color, 1, -1);

	ctx->skycube = THE_CreateTexture("./assets/tex/Xcave.png", THE_TEX_SKYBOX);
	THE_MaterialSetTexture(THE_ShaderCommonData(ctx->skybox), &ctx->skycube, 1, 0);
	THE_ShaderCommonData(ctx->skybox)->data = THE_PersistentAlloc(16 * sizeof(float), 0);
	THE_ShaderCommonData(ctx->skybox)->dcount = 16;

	ctx->hello_mat.data.fields.color[0] = 1.0f;
	ctx->hello_mat.data.fields.color[1] = 1.0f;
	ctx->hello_mat.data.fields.color[2] = 0.0f;
	ctx->hello_mat.data.fields.color[3] = 1.0f;

	ctx->e = THE_EntityCreate();
	float pos[3] = {0.0f, 0.0f, -4.0f};
	mat4_translation(ctx->e->transform, mat4_identity(ctx->e->transform), pos);
	ctx->e->mesh = SPHERE_MESH;
	ctx->e->mat = ctx->hellomat;
	ctx->e->mat_data = THE_MaterialDefault();
	THE_MaterialSetData(&ctx->e->mat_data, ctx->hello_mat.data.buffer, sizeof(HelloMatData) / 4);
}

bool Update(void *context)
{
	HelloCtx *ctx = context;
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, deltatime);

	// Render commands

	THE_RenderCommand *fbuff = THE_AllocateCommand();
	fbuff->data.usefb = ctx->fb;
	fbuff->execute = THE_UseFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	fbuff->next = rops;
	rops->data.renderops.blend = true;
	rops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rops->data.renderops.depth_test = true;
	rops->data.renderops.write_depth = true;
	rops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rops->data.renderops.depth_func = THE_DEPTHFUNC_LESS;
	rops->data.renderops.changed_mask = 0xFF; // Everything changed.
	rops->execute = THE_RenderOptionsExecute;

	mat4_multiply(((HelloMatData*)ctx->e->mat_data.data)->data.fields.vp, camera.proj_mat, camera.view_mat);

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
	usemat->data.use_shader = ctx->hellomat;
	usemat->execute = THE_UseShaderExecute;
	clear->next = usemat;
	usemat->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	rops = THE_AllocateCommand();
	rops->data.renderops.cull_face = THE_CULLFACE_DISABLED;
	rops->data.renderops.depth_func = THE_DEPTHFUNC_LEQUAL;
	rops->data.renderops.changed_mask = THE_CULL_FACE_BIT | THE_DEPTH_FUNC_BIT;
	rops->execute = THE_RenderOptionsExecute;
	
	THE_CameraStaticViewProjection(THE_ShaderCommonData(ctx->skybox)->data, &camera);
	THE_RenderCommand *use_sky_shader = THE_AllocateCommand();
	use_sky_shader->data.use_shader = ctx->skybox;
	use_sky_shader->execute = THE_UseShaderExecute;
	rops->next = use_sky_shader;

	THE_RenderCommand *draw_sky = THE_AllocateCommand();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.shader = ctx->skybox;
	draw_sky->execute = THE_DrawExecute;
	use_sky_shader->next = draw_sky;

	THE_RenderCommand *fbuff2 = THE_AllocateCommand();
	fbuff2->data.usefb = THE_DEFAULT;
	fbuff2->execute = THE_UseFramebufferExecute;
	draw_sky->next = fbuff2;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	fbuff2->next = rops2;
	rops2->data.renderops.depth_test = false;
	rops2->data.renderops.changed_mask = THE_DEPTH_TEST_BIT;
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
	usefullscreen->data.use_shader = ctx->fs_img;
	usefullscreen->execute = THE_UseShaderExecute;
	clear2->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.shader = ctx->fs_img;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(rops);

	return true;
}

int main(int argc, char **argv)
{
	HelloCtx ctx;
	struct THE_Config cnfg = {
		.init_func = Init,
		.update_func = Update,
		.context = &ctx,
		.heap_size = THE_GB(1U),
		.window_title = "THE_Hello",
		.window_width = 1280,
		.window_height = 720,
		.vsync = true
	};
	
	THE_Start(&cnfg);
	THE_End();

	return 0;
}
