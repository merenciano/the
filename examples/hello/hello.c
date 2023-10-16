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

HelloMatData data;
THE_Entity *e;

void Init(void)
{
	THE_InitMaterial(THE_MT_HELLO, "hello");

	data.data.fields.color[0] = 1.0f;
	data.data.fields.color[1] = 0.0f;
	data.data.fields.color[2] = 0.0f;
	data.data.fields.color[3] = 1.0f;

	e = THE_EntityCreate();
	e->transform = smat4_translation(smat4_identity(), svec3(2.0f, 0.0f, 0.0f));
	e->mesh = SPHERE_MESH;
	e->mat.type = THE_MT_HELLO;
}

void Update(void)
{
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, THE_DeltaTime());

	// Render commands
	data.data.fields.vp = smat4_multiply(camera.proj_mat, camera.view_mat);
	THE_MaterialSetData(&e->mat, data.data.buffer, sizeof(HelloMatData) / 4);

	THE_RenderCommand *clear = THE_AllocateCommand();
	clear->data.clear.bcolor = 1;
	clear->data.clear.bdepth = 1;
	clear->data.clear.bstencil = 0;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;

	THE_RenderCommand *usemat = THE_AllocateCommand();
	usemat->data.usemat.mat = THE_AllocateFrameResource(sizeof(THE_Material));
	THE_InitNewMaterial(usemat->data.usemat.mat);
	usemat->data.usemat.mat->type = THE_MT_HELLO;
	usemat->execute = THE_UseMaterialExecute;
	clear->next = usemat;
	usemat->next = NULL;

	THE_AddCommands(clear);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());
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
