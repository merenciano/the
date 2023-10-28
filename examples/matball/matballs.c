#include "the.h"
#include <string.h>
#include <stdlib.h>
#include <mathc.h>

struct Materials {
	THE_Shader fullscreen_img;
	THE_Shader skybox;
	THE_Shader eqr_to_cube;
	THE_Shader prefilter_env;
	THE_Shader lut_gen;
	THE_Shader pbr;
};

struct Materials g_mats;
static THE_ResourceMap g_resources;
THE_Framebuffer g_fb;

static float g_sunlight[4] = {0.0f, -1.0f, -0.1f, 1.0f};

void Init(void *context)
{
	g_fb = THE_CreateFramebuffer(THE_WindowGetWidth(), THE_WindowGetHeight(), true, true);
	g_resources.meshes = THE_HMapCreate(8, sizeof(THE_Mesh));
	g_resources.textures = THE_HMapCreate(64, sizeof(THE_Texture));

	g_mats.fullscreen_img = THE_CreateShader("fullscreen-img");
	g_mats.skybox = THE_CreateShader("skybox");
	g_mats.eqr_to_cube = THE_CreateShader("eqr-to-cube");
	g_mats.prefilter_env = THE_CreateShader("prefilter-env");
	g_mats.lut_gen = THE_CreateShader("lut-gen");
	g_mats.pbr = THE_CreateShader("pbr");

	THE_Texture fb_color = THE_GetFrameColor(g_fb);
	THE_MaterialSetTexture(THE_ShaderCommonData(g_mats.fullscreen_img), &fb_color, 1, -1);

	THE_ResourceMap *rm = &g_resources;

	THE_ResourceMapAddMeshFromPath(rm, "MatBall", "assets/obj/matball-n.obj");
	THE_ResourceMapAddTexture(rm, "Skybox", 1024, 1024, THE_TEX_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "Irradian", 1024, 1024, THE_TEX_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "Prefilte", 128, 128, THE_TEX_PREFILTER_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "LutMap", 512, 512, THE_TEX_LUT);

	THE_ResourceMapAddTextureFromPath(rm, "Gold_A" ,"assets/tex/celtic-gold/celtic-gold_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Gold_N" ,"assets/tex/celtic-gold/celtic-gold_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Gold_M" ,"assets/tex/celtic-gold/celtic-gold_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Gold_R" ,"assets/tex/celtic-gold/celtic-gold_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Peel_A", "assets/tex/peeling/peeling_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_N", "assets/tex/peeling/peeling_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_M", "assets/tex/peeling/peeling_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_R", "assets/tex/peeling/peeling_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Rust_A", "assets/tex/rusted/rusted_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_N", "assets/tex/rusted/rusted_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_M", "assets/tex/rusted/rusted_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_R", "assets/tex/rusted/rusted_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Tiles_A", "assets/tex/tiles/tiles_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_N", "assets/tex/tiles/tiles_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_M", "assets/tex/tiles/tiles_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_R", "assets/tex/tiles/tiles_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Future_A", "assets/tex/ship-panels/ship-panels_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Future_N", "assets/tex/ship-panels/ship-panels_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Future_M", "assets/tex/ship-panels/ship-panels_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Future_R", "assets/tex/ship-panels/ship-panels_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Shore_A", "assets/tex/shore/shore_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_N", "assets/tex/shore/shore_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_M", "assets/tex/shore/shore_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_R", "assets/tex/shore/shore_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Cliff_A", "assets/tex/cliff/cliff_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_N", "assets/tex/cliff/cliff_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_M", "assets/tex/cliff/cliff_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_R", "assets/tex/cliff/cliff_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Granit_A", "assets/tex/granite/granite_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_N", "assets/tex/granite/granite_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_M", "assets/tex/granite/granite_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_R", "assets/tex/granite/granite_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Foam_A", "assets/tex/foam/foam_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_N", "assets/tex/foam/foam_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_M", "assets/tex/foam/foam_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_R", "assets/tex/foam/foam_R.png", THE_TEX_R);

	struct THE_PbrData pbr;
	pbr.color[0] = 1.0f;
	pbr.color[1] = 1.0f;
	pbr.color[2] = 1.0f;
	pbr.tiling_x = 4.0f;
	pbr.tiling_y = 4.0f;
	pbr.use_albedo_map = 1.0f;
	pbr.use_pbr_maps = 1.0f;
	pbr.metallic = 0.5f;
	pbr.roughness = 0.5f;
	pbr.normal_map_intensity = 1.0f;

	float position[3] = {-2.0f, 0.0f, 0.0f};

	// CelticGold
	{
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Gold_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Gold_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Gold_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Gold_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Shore
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		THE_Entity *e = THE_EntityCreate();
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Shore_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Shore_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Shore_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Shore_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Peeling
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.7f;
		position[0] = 2.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Peel_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Peel_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Peel_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Peel_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Rusted
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.2f;
		position[0] = -2.0f;
		position[2] = -2.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Rust_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Rust_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Rust_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Rust_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Tiles
	{
		pbr.tiling_x = 4.0f;
		pbr.tiling_y = 4.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Tiles_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Tiles_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Tiles_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Tiles_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Ship Panels
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 2.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Future_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Future_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Future_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Future_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Cliff
	{
		pbr.tiling_x = 8.0f;
		pbr.tiling_y = 8.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = -2.0f;
		position[2] = -4.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Cliff_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Cliff_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Cliff_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Cliff_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Granite
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Granit_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Granit_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Granit_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Granit_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	// Foam
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		position[0] = 2.0f;
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(struct THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Foam_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Foam_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Foam_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Foam_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	THE_RenderCommand *rendops = THE_AllocateCommand();
	rendops->data.renderops.depth_test = true;
	rendops->data.renderops.write_depth = true;
	rendops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rendops->data.renderops.blend = true;
	rendops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rendops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rendops->data.renderops.changed_mask = 0xFF & ~THE_DEPTH_FUNC_BIT;
	rendops->execute = THE_RenderOptionsExecute;

	THE_RenderCommand *sky = THE_AllocateCommand();
	strcpy(sky->data.eqr_cube.in_path, "assets/tex/env/helipad-env.hdr");
	sky->data.eqr_cube.out_cube = THE_ResourceMapGetTexture(rm, "Skybox");
	sky->data.eqr_cube.out_prefilt = THE_ResourceMapGetTexture(rm, "Prefilte");
	sky->data.eqr_cube.out_lut = THE_ResourceMapGetTexture(rm, "LutMap");
	sky->execute = THE_EquirectToCubeExecute;
	rendops->next = sky;

	THE_RenderCommand *irradiance = THE_AllocateCommand();
	strcpy(irradiance->data.eqr_cube.in_path, "assets/tex/env/helipad-dif.hdr");
	irradiance->data.eqr_cube.out_cube = THE_ResourceMapGetTexture(rm, "Irradian");
	irradiance->data.eqr_cube.out_prefilt = THE_UNINIT;
	irradiance->data.eqr_cube.out_lut = THE_UNINIT;
	irradiance->execute = THE_EquirectToCubeExecute;
	sky->next = irradiance;
	irradiance->next = NULL;

	THE_AddCommands(rendops);

	THE_ShaderData *scene_data = THE_ShaderCommonData(g_mats.pbr);
	*scene_data = THE_MaterialDefault();
	scene_data->data = malloc(sizeof(struct THE_PbrSceneData));
	scene_data->dcount = sizeof(struct THE_PbrSceneData) / sizeof(float);
	scene_data->tex = malloc(3 * sizeof(THE_Texture));
	scene_data->tcount = 3;
	scene_data->cube_start = 1;
	scene_data->tex[0] = THE_ResourceMapGetTexture(rm, "LutMap");
	scene_data->tex[1] = THE_ResourceMapGetTexture(rm, "Irradian");
	scene_data->tex[2] = THE_ResourceMapGetTexture(rm, "Prefilte");

	THE_Texture skytex = THE_ResourceMapGetTexture(rm, "Skybox");
	THE_MaterialSetTexture(THE_ShaderCommonData(g_mats.skybox), &skytex, 1, 0);
	THE_ShaderCommonData(g_mats.skybox)->data = THE_PersistentAlloc(16 * sizeof(float), 0);
	THE_ShaderCommonData(g_mats.skybox)->dcount = 16;
}

bool Update(void *context)
{
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, deltatime);

	// PBR Material common data.
	struct THE_PbrSceneData *scene_data = (struct THE_PbrSceneData*)THE_ShaderCommonData(g_mats.pbr)->data;
	mat4_multiply(scene_data->view_projection, camera.proj_mat, camera.view_mat);
	THE_CameraPosition(scene_data->camera_position, &camera);
	vec4_assign(scene_data->sunlight, g_sunlight);

	THE_RenderCommand* fbuff = THE_AllocateCommand();
	fbuff->data.usefb = g_fb;
	fbuff->execute = THE_UseFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	rops->data.renderops.blend = true;
	rops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rops->data.renderops.depth_func = THE_DEPTHFUNC_LESS;
	rops->data.renderops.depth_test = true;
	rops->data.renderops.write_depth = true;
	rops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rops->data.renderops.changed_mask = 0xFF; // Everything changed.
	rops->execute = THE_RenderOptionsExecute;
	fbuff->next = rops;

	THE_RenderCommand *clear = THE_AllocateCommand();
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = true;
	clear->data.clear.stencil_buffer = false;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;
	rops->next = clear;


	THE_RenderCommand *use_pbr = THE_AllocateCommand();
	use_pbr->data.use_shader = g_mats.pbr;
	use_pbr->execute = THE_UseShaderExecute;
	clear->next = use_pbr;
	use_pbr->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	rops = THE_AllocateCommand();
	rops->data.renderops.cull_face = THE_CULLFACE_DISABLED;
	rops->data.renderops.depth_func = THE_DEPTHFUNC_LEQUAL;
	rops->data.renderops.changed_mask = THE_CULL_FACE_BIT | THE_DEPTH_FUNC_BIT;
	rops->execute = THE_RenderOptionsExecute;

	THE_CameraStaticViewProjection(THE_ShaderCommonData(g_mats.skybox)->data, &camera);
	THE_RenderCommand *use_sky_shader = THE_AllocateCommand();
	use_sky_shader->data.use_shader = g_mats.skybox;
	use_sky_shader->execute = THE_UseShaderExecute;
	rops->next = use_sky_shader;

	THE_RenderCommand *draw_sky = THE_AllocateCommand();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.shader = THE_IGNORE;
	draw_sky->execute = THE_DrawExecute;
	use_sky_shader->next = draw_sky;
	
	/*
	THE_RenderCommand *sky = THE_AllocateCommand();
	sky->data.skybox.cubemap = THE_ResourceMapGetTexture(&g_resources, "Skybox");
	sky->execute = THE_SkyboxExecute;
	rops->next = sky;
	*/

	fbuff = THE_AllocateCommand();
	fbuff->data.usefb = THE_DEFAULT;
	fbuff->execute = THE_UseFramebufferExecute;
	draw_sky->next = fbuff;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	rops2->data.renderops.depth_test = false;
	rops2->data.renderops.changed_mask = THE_DEPTH_TEST_BIT;
	rops2->execute = THE_RenderOptionsExecute;
	fbuff->next = rops2;

	clear = THE_AllocateCommand();
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = false;
	clear->data.clear.stencil_buffer = false;
	clear->data.clear.color[0] = 1.0f;
	clear->data.clear.color[1] = 0.0f;
	clear->data.clear.color[2] = 0.0f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;
	rops2->next = clear;

	THE_RenderCommand *usefullscreen = THE_AllocateCommand();
	usefullscreen->data.use_shader = g_mats.fullscreen_img;
	usefullscreen->execute = THE_UseShaderExecute;
	clear->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.shader = THE_IGNORE;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(rops);

	return true;
}

int main(int argc, char **argv)
{
	struct THE_Config cnfg = {
		.init_func = Init,
		.update_func = Update,
		.heap_size = THE_GB(1),
		.window_title = "THE Material Demo",
		.window_width = 1280,
		.window_height = 720,
		.vsync = true
	};
	
	THE_Start(&cnfg);

	THE_End();

	return 0;
}
