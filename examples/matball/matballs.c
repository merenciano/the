#include "the.h"
#include <string.h>

struct Materials {
	THE_Shader fullscreen_img;
	THE_Shader skybox;
	THE_Shader eqr_to_cube;
	THE_Shader prefilter_env;
	THE_Shader lut_gen;
	THE_Shader pbr;
};

struct Materials g_mats;

void Init(void)
{
	g_mats.fullscreen_img = THE_CreateShader("fullscreen-img");
	g_mats.skybox = THE_CreateShader("skybox");
	g_mats.eqr_to_cube = THE_CreateShader("eqr-to-cube");
	g_mats.prefilter_env = THE_CreateShader("prefilter-env");
	g_mats.lut_gen = THE_CreateShader("lut-gen");
	g_mats.pbr = THE_CreateShader("pbr");

	THE_ResourceMap *rm = &resource_map;

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

	THE_PbrData pbr;
	pbr.color = svec3(1.0f, 1.0f, 1.0f);
	pbr.tiling_x = 4.0f;
	pbr.tiling_y = 4.0f;
	pbr.use_albedo_map = 1.0f;
	pbr.use_pbr_maps = 1.0f;
	pbr.metallic = 0.5f;
	pbr.roughness = 0.5f;
	pbr.normal_map_intensity = 1.0f;

	sun_dir_intensity = svec4(0.0f, -1.0f, -0.1f, 1.0f);

	// CelticGold
	{
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(2.0f, 0.0f, 0.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		e->transform = smat4_translation(smat4_identity(), svec3(4.0f, 0.0f, 0.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(6.0f, 0.0f, 0.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(2.0f, 0.0f, 2.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(4.0f, 0.0f, 2.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(6.0f, 0.0f, 2.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(2.0f, 0.0f, 4.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(4.0f, 0.0f, 4.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
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
		THE_Entity *e = THE_EntityCreate();
		e->transform = smat4_translation(smat4_identity(), svec3(6.0f, 0.0f, 4.0f));
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat = g_mats.pbr;
		e->mat_data = THE_MaterialDefault();
		THE_MaterialSetData(&e->mat_data, (float*)&pbr, sizeof(THE_PbrData) / 4);
		THE_Texture t[4];
		t[0] = THE_ResourceMapGetTexture(rm, "Foam_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Foam_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Foam_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Foam_N");
		THE_MaterialSetTexture(&e->mat_data, t, 4, -1);
	}

	THE_RenderCommand *rendops = THE_AllocateCommand();
	rendops->data.renderops.depth_test = 1;
	rendops->data.renderops.write_depth = 1;
	rendops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rendops->data.renderops.blend = 1;
	rendops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rendops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rendops->data.renderops.changed_mask = 0xFF; // Changed all
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
}

void Update(void)
{
	//THE_ScriptingExecute("assets/scripts/update.lua");
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, THE_DeltaTime());

	// Render commands
	THE_PbrSceneData pbr_sd;
	pbr_sd.view_projection = smat4_multiply(camera.proj_mat, camera.view_mat);
	pbr_sd.camera_position = THE_CameraPosition(&camera);
	pbr_sd.light_direction_intensity = sun_dir_intensity;

	THE_Material full_screen_img = THE_MaterialDefault();
	THE_Texture fbtex = THE_CameraOutputColorTexture(&camera); // TODO: Revisar si deberia hacerse en aqui algo tan low level del renderer (algo como link camera to tex?)
	THE_MaterialSetFrameTexture(&full_screen_img, &fbtex, 1, -1);

	THE_RenderCommand* fbuff = THE_AllocateCommand();
	fbuff->data.usefb.fb = camera.fb;
	fbuff->execute = THE_UseFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	rops->data.renderops.blend = 1;
	rops->data.renderops.sfactor = THE_BLENDFUNC_ONE;
	rops->data.renderops.dfactor = THE_BLENDFUNC_ZERO;
	rops->data.renderops.depth_test = 1;
	rops->data.renderops.write_depth = 1;
	rops->data.renderops.cull_face = THE_CULLFACE_BACK;
	rops->data.renderops.changed_mask = 0xFF; // Everything changed.
	rops->execute = THE_RenderOptionsExecute;
	fbuff->next = rops;

	THE_RenderCommand *clear = THE_AllocateCommand();
	clear->data.clear.bcolor = 1;
	clear->data.clear.bdepth = 1;
	clear->data.clear.bstencil = 0;
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;
	rops->next = clear;

	THE_Texture scene_tex[3];
	scene_tex[0] = THE_ResourceMapGetTexture(&resource_map, "LutMap");  //GM.resource_map()->textures.at("LutMap");
	scene_tex[1] = THE_ResourceMapGetTexture(&resource_map, "Irradian"); //GM.resource_map()->textures.at("IrradianceEnv");
	scene_tex[2] = THE_ResourceMapGetTexture(&resource_map, "Prefilte"); //GM.resource_map()->textures.at("PrefilterSpec");

	THE_RenderCommand *usemat = THE_AllocateCommand();
	THE_Material newmatdat = THE_MaterialDefault();
	THE_MaterialSetFrameData(&newmatdat, (float*)&pbr_sd, sizeof(THE_PbrSceneData) / sizeof(float));
	THE_MaterialSetFrameTexture(&newmatdat, scene_tex, 3, 1);
	usemat->data.usemat.data = newmatdat;
	usemat->data.usemat.mat = g_mats.pbr;
	usemat->execute = THE_UseShaderExecute;
	clear->next = usemat;
	usemat->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	rops = THE_AllocateCommand();
	rops->data.renderops.cull_face = THE_CULLFACE_DISABLED;
	rops->data.renderops.changed_mask = THE_CULL_FACE_BIT;
	rops->execute = THE_RenderOptionsExecute;

	THE_RenderCommand *sky = THE_AllocateCommand();
	sky->data.skybox.cubemap = THE_ResourceMapGetTexture(&resource_map, "Skybox");
	sky->execute = THE_SkyboxExecute;
	rops->next = sky;

	fbuff = THE_AllocateCommand();
	fbuff->data.usefb.fb = THE_DEFAULT;
	fbuff->execute = THE_UseFramebufferExecute;
	sky->next = fbuff;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	rops2->data.renderops.depth_test = 0;
	rops2->data.renderops.changed_mask = THE_DEPTH_TEST_BIT;
	rops2->execute = THE_RenderOptionsExecute;
	fbuff->next = rops2;

	clear = THE_AllocateCommand();
	clear->data.clear.bcolor = 1;
	clear->data.clear.bdepth = 0;
	clear->data.clear.bstencil = 0;
	clear->data.clear.color[0] = 1.0f;
	clear->data.clear.color[1] = 0.0f;
	clear->data.clear.color[2] = 0.0f;
	clear->data.clear.color[3] = 1.0f;
	clear->execute = THE_ClearExecute;
	rops2->next = clear;

	THE_RenderCommand *usefullscreen = THE_AllocateCommand();
	usefullscreen->data.usemat.mat = g_mats.fullscreen_img;
	usefullscreen->data.usemat.data = full_screen_img;
	usefullscreen->execute = THE_UseShaderExecute;
	clear->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.newmat = g_mats.fullscreen_img;
	draw->data.draw.matdata = full_screen_img;
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
