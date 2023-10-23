#include "render/material.h"
#include "render/renderer.h"
#include "the.h"
#include <string.h>

typedef struct {
	union {
		float buffer[16+16+4];
		struct {
			float model[16];
			struct mat4 vp;
			float color[4];
		} fields;
	} data;
} HelloMatData;

static HelloMatData data;
static THE_Shader hellomat;
static THE_Shader fs_img;
THE_Entity *e;

void Init(void)
{
	hellomat = THE_CreateShader("hello");
	fs_img = THE_CreateShader("fullscreen-img");

	data.data.fields.color[0] = 1.0f;
	data.data.fields.color[1] = 0.0f;
	data.data.fields.color[2] = 0.0f;
	data.data.fields.color[3] = 1.0f;

	e = THE_EntityCreate();
	e->transform = smat4_translation(smat4_identity(), svec3(0.0f, 0.0f, -4.0f));
	e->mesh = CUBE_MESH;
	e->mat = hellomat;
	e->mat_data = THE_MaterialDefault();
}

void Update(void)
{
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, THE_DeltaTime());

	// Render commands

	THE_RenderCommand *fbuff = THE_AllocateCommand();
	fbuff->data.usefb.fb = camera.fb;
	fbuff->execute = THE_UseFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	fbuff->next = rops;
	rops->data.renderops.blend = 1;
	rops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rops->data.renderops.depth_test = 1;
	rops->data.renderops.write_depth = 1;
	rops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rops->data.renderops.changed_mask = 0xFF; // Everything changed.
	rops->execute = THE_RenderOptionsExecute;

	data.data.fields.vp = smat4_multiply(camera.proj_mat, camera.view_mat);
	THE_MaterialSetData(&e->mat_data, data.data.buffer, sizeof(HelloMatData) / 4);

	THE_RenderCommand *clear = THE_AllocateCommand();
	rops->next = clear;
	clear->data.clear.bcolor = 1;
	clear->data.clear.bdepth = 1;
	clear->data.clear.bstencil = 0;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;

	THE_RenderCommand *usemat = THE_AllocateCommand();
	usemat->data.use_shader.material = THE_MaterialDefault();
	usemat->data.use_shader.shader = hellomat;
	usemat->execute = THE_UseShaderExecute;
	clear->next = usemat;
	usemat->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	THE_RenderCommand *fbuff2 = THE_AllocateCommand();
	fbuff2->data.usefb.fb = THE_DEFAULT;
	fbuff2->execute = THE_UseFramebufferExecute;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	fbuff2->next = rops2;
	rops2->data.renderops.depth_test = 0;
	rops2->data.renderops.changed_mask = THE_DEPTH_TEST_BIT;
	rops2->execute = THE_RenderOptionsExecute;

	THE_RenderCommand *clear2 = THE_AllocateCommand();
	clear2->data.clear.bcolor = 1;
	clear2->data.clear.bdepth = 0;
	clear2->data.clear.bstencil = 0;
	clear2->data.clear.color[0] = 1.0f;
	clear2->data.clear.color[1] = 0.0f;
	clear2->data.clear.color[2] = 0.0f;
	clear2->data.clear.color[3] = 1.0f;
	clear2->execute = THE_ClearExecute;
	rops2->next = clear2;

	THE_RenderCommand *usefullscreen = THE_AllocateCommand();
	usefullscreen->data.use_shader.shader = fs_img;
	usefullscreen->data.use_shader.material = THE_MaterialDefault();
	THE_Texture fbtex = THE_CameraOutputColorTexture(&camera);
	THE_MaterialSetFrameTexture(&usefullscreen->data.use_shader.material, &fbtex, 1, -1);
	usefullscreen->execute = THE_UseShaderExecute;
	clear2->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.shader = fs_img;
	draw->data.draw.mat = usefullscreen->data.use_shader.material;
	draw->data.draw.inst_count = 1;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(fbuff2);
}

void Close(void)
{

}

int main(int argc, char **argv)
{
	THE_Config cnfg = {
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
	THE_StartFrameTimer();
	while (!THE_WindowShouldClose()) {
		THE_Logic();
		THE_Render();
		THE_ShowFrame();
	}
	THE_Close();

	return 0;
}
