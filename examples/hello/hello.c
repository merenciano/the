#include "render/camera.h"
#include "render/rendercommands.h"
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

static HelloMatData data;
static THE_Shader hellomat;
static THE_Shader fs_img;
static THE_Shader skybox;
THE_Entity *e;
THE_Framebuffer g_fb;
THE_Texture skycube;

void Init(void)
{
	g_fb = THE_CreateFramebuffer(THE_WindowGetWidth(), THE_WindowGetHeight(), true, true);
	hellomat = THE_CreateShader("hello");
	fs_img = THE_CreateShader("fullscreen-img");
	skybox = THE_CreateShader("skybox");
	THE_Texture fb_color = THE_GetFrameColor(g_fb);
	THE_MaterialSetTexture(THE_ShaderCommonData(fs_img), &fb_color, 1, -1);

	skycube = THE_CreateTexture("./assets/tex/Xcave.png", THE_TEX_SKYBOX);
	THE_MaterialSetTexture(THE_ShaderCommonData(skybox), &skycube, 1, 0);
	THE_ShaderCommonData(skybox)->data = THE_PersistentAlloc(16 * sizeof(float), 0);
	THE_ShaderCommonData(skybox)->dcount = 16;



	data.data.fields.color[0] = 1.0f;
	data.data.fields.color[1] = 1.0f;
	data.data.fields.color[2] = 0.0f;
	data.data.fields.color[3] = 1.0f;

	e = THE_EntityCreate();
	float pos[3] = {0.0f, 0.0f, -4.0f};
	mat4_translation(e->transform, mat4_identity(e->transform), pos);
	e->mesh = THE_CreateMeshFromFile_OBJ("../matball/assets/obj/matball-n.obj");
	e->mat = hellomat;
	e->mat_data = THE_MaterialDefault();
	THE_MaterialSetData(&e->mat_data, data.data.buffer, sizeof(HelloMatData) / 4);
}

void Update(void)
{
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, THE_DeltaTime());

	// Render commands

	THE_RenderCommand *fbuff = THE_AllocateCommand();
	fbuff->data.usefb = g_fb;
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

	mat4_multiply(((HelloMatData*)e->mat_data.data)->data.fields.vp, camera.proj_mat, camera.view_mat);

	THE_RenderCommand *clear = THE_AllocateCommand();
	rops->next = clear;
	clear->data.clear.bcolor = true;
	clear->data.clear.bdepth = true;
	clear->data.clear.bstencil = false;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;

	THE_RenderCommand *usemat = THE_AllocateCommand();
	usemat->data.use_shader = hellomat;
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
	
	THE_CameraStaticViewProjection(THE_ShaderCommonData(skybox)->data, &camera);
	THE_RenderCommand *use_sky_shader = THE_AllocateCommand();
	use_sky_shader->data.use_shader = skybox;
	use_sky_shader->execute = THE_UseShaderExecute;
	rops->next = use_sky_shader;

	THE_RenderCommand *draw_sky = THE_AllocateCommand();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.shader = skybox;
	draw_sky->data.draw.mat = THE_MaterialDefault();
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
	clear2->data.clear.bcolor = true;
	clear2->data.clear.bdepth = false;
	clear2->data.clear.bstencil = false;
	clear2->data.clear.color[0] = 0.0f;
	clear2->data.clear.color[1] = 1.0f;
	clear2->data.clear.color[2] = 1.0f;
	clear2->data.clear.color[3] = 1.0f;
	clear2->execute = THE_ClearExecute;
	rops2->next = clear2;

	THE_RenderCommand *usefullscreen = THE_AllocateCommand();
	usefullscreen->data.use_shader = fs_img;
	usefullscreen->execute = THE_UseShaderExecute;
	clear2->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.shader = fs_img;
	draw->data.draw.mat = THE_MaterialDefault();
	draw->data.draw.inst_count = 1;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(rops);
}

void Close(void)
{

}

int main(int argc, char **argv)
{
	struct THE_Config cnfg = {
		.init_func = Init,
		.logic_func = Update,
		.close_func = Close,
		.alloc_capacity = THE_GB(1),
		.max_geometries = 128,
		.window_width = 1280,
		.window_height = 720,
		.vsync = true
	};
	
	THE_Init(&cnfg);
	while (!THE_WindowShouldClose()) {
		THE_Logic();
		THE_Render();
		THE_ShowFrame();
	}
	THE_Close();

	return 0;
}
