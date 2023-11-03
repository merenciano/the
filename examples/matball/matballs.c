#include "render/renderer.h"
#include "the.h"
#include <mathc.h>
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
static THE_ResourceMap g_resources;
THE_Framebuffer g_fb;

THE_Material pbr_common;
THE_Material sky_common;
THE_Material fulls;

static float g_sunlight[4] = { 0.0f, -1.0f, -0.1f, 1.0f };

static THE_Material
GenerateRenderToCubeMat(void)
{
	float proj[16] = { 0.0f };
	mat4_perspective(proj, to_radians(90.0f), 1.0f, 0.1f, 10.0f);

	struct mat4 views[] = {
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(1.0f, 0.0f, 0.0f),
		              svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(-1.0f, 0.0f, 0.0f),
		              svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 1.0f, 0.0f),
		              svec3(0.0f, 0.0f, 1.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, -1.0f, 0.0f),
		              svec3(0.0f, 0.0f, -1.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 0.0f, 1.0f),
		              svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 0.0f, -1.0f),
		              svec3(0.0f, -1.0f, 0.0f)),
	};

	THE_Material mat_tocube = { .shader = g_mats.eqr_to_cube,
		                        .data_count = 16 * 6,
		                        .tex_count = 0,
		                        .cube_count = 0 };

	float *vp = THE_MaterialAllocFrame(&mat_tocube);
	for (int i = 0; i < 6; ++i) {
		float *view = (float *)&views[i];
		mat4_multiply(vp + 16 * i, proj, view);
	}

	return mat_tocube;
}

static THE_RenderCommand *
ConvertEqrToCubeCommandList(THE_Texture tex_in,
                            THE_Texture tex_out,
                            THE_Framebuffer fb,
                            THE_RenderCommand *last_command)
{
	THE_RenderCommand *use_tocube = THE_AllocateCommand();
	use_tocube->data.mat.shader = g_mats.eqr_to_cube;
	use_tocube->data.mat.data_count = 0;
	use_tocube->data.mat.tex_count = 1;
	use_tocube->data.mat.cube_count = 0;
	THE_Texture *eqr = THE_MaterialAllocFrame(&use_tocube->data.mat);
	*eqr = tex_in;
	use_tocube->execute = THE_UseShaderExecute;
	last_command->next = use_tocube;

	THE_FBAttachment atta = { .slot = THE_ATTACH_COLOR,
		                      .tex = tex_out,
		                      .level = 0 };

	THE_RenderCommand *last = use_tocube;
	THE_Material draw_tocube = GenerateRenderToCubeMat();
	for (int i = 0; i < 6; ++i) {
		THE_RenderCommand *cube_fb = THE_AllocateCommand();
		cube_fb->data.set_fb.fb = fb;
		cube_fb->data.set_fb.attachment = atta;
		cube_fb->data.set_fb.attachment.side = i;
		cube_fb->execute = THE_SetFramebufferExecute;
		last->next = cube_fb;

		THE_RenderCommand *draw_cube = THE_AllocateCommand();
		draw_cube->data.draw.material = draw_tocube;
		draw_cube->data.draw.material.data_count = 16;
		draw_cube->data.draw.material.ptr = (float *)draw_tocube.ptr + 16 * i;
		draw_cube->execute = THE_DrawExecute;
		cube_fb->next = draw_cube;
		last = draw_cube;
	}

	return last;
}

static THE_RenderCommand *
GeneratePrefilterCommandList(THE_Texture tex_in,
                             THE_Texture tex_out,
                             THE_Framebuffer fb,
                             THE_RenderCommand *last_command)
{
	THE_RenderCommand *use_pref = THE_AllocateCommand();
	use_pref->data.mat.shader = g_mats.prefilter_env;
	use_pref->data.mat.data_count = 0;
	use_pref->data.mat.tex_count = 0;
	use_pref->data.mat.cube_count = 1;
	THE_Texture *sky_tex = THE_MaterialAllocFrame(&use_pref->data.mat);
	*sky_tex = tex_in;
	use_pref->execute = THE_UseShaderExecute;
	last_command->next = use_pref;

	THE_Material mat = GenerateRenderToCubeMat();
	THE_RenderCommand *prefilter = THE_AllocateCommand();
	prefilter->data.eqr_cube.fb = fb;
	prefilter->data.eqr_cube.out_prefilt = tex_out;
	prefilter->data.eqr_cube.draw_cubemap = mat;
	prefilter->execute = THE_EquirectToCubeExecute;
	use_pref->next = prefilter;
	return prefilter;
}

static THE_RenderCommand *
GenerateLutCommandlist(THE_Framebuffer fb, THE_RenderCommand *last_command)
{
	THE_Texture lut_tex = THE_ResourceMapGetTexture(&g_resources, "LutMap");
	THE_RenderCommand *set_init_fb = THE_AllocateCommand();
	set_init_fb->data.set_fb.fb = fb;
	set_init_fb->data.set_fb.attachment.slot = THE_ATTACH_COLOR;
	set_init_fb->data.set_fb.attachment.tex = lut_tex;
	set_init_fb->data.set_fb.attachment.side = -1;
	set_init_fb->data.set_fb.attachment.level = 0;
	set_init_fb->execute = THE_SetFramebufferExecute;
	last_command->next = set_init_fb;

	THE_RenderCommand *clear_comm = THE_AllocateCommand();
	clear_comm->data.clear.color_buffer = true;
	clear_comm->data.clear.depth_buffer = false;
	clear_comm->data.clear.stencil_buffer = false;
	clear_comm->execute = THE_ClearExecute;
	set_init_fb->next = clear_comm;

	THE_Material lutmat = THE_MaterialDefault();
	lutmat.shader = g_mats.lut_gen;

	THE_RenderCommand *set_lut = THE_AllocateCommand();
	set_lut->data.mat = lutmat;
	set_lut->execute = THE_UseShaderExecute;
	clear_comm->next = set_lut;

	THE_RenderCommand *draw_lut = THE_AllocateCommand();
	draw_lut->data.draw.mesh = QUAD_MESH;
	draw_lut->data.draw.material = lutmat;
	draw_lut->execute = THE_DrawExecute;
	set_lut->next = draw_lut;
	draw_lut->next = NULL;

	return draw_lut;
}

static void
GeneratePbrEnv(void)
{
	THE_ResourceMap *rm = &g_resources;
	THE_Framebuffer auxfb = THE_CreateFramebuffer(1, 1, false, false);

	THE_RenderCommand *rendops = THE_AllocateCommand();
	rendops->data.rend_opts.enable_flags = THE_DEPTH_TEST | THE_DEPTH_WRITE |
	  THE_BLEND;
	rendops->data.rend_opts.disable_flags = THE_CULL_FACE;
	rendops->data.rend_opts.cull_face = THE_CULL_FACE_BACK;
	rendops->data.rend_opts.blend_func.src = THE_BLEND_FUNC_ONE;
	rendops->data.rend_opts.blend_func.dst = THE_BLEND_FUNC_ZERO;
	rendops->execute = THE_RenderOptionsExecute;

	THE_Texture eqr_in =
	  THE_CreateTextureFromFile("assets/tex/env/helipad-env.hdr",
	                            THE_TEX_RGB_F16);
	THE_Texture eqr_cube_out = THE_ResourceMapGetTexture(rm, "Skybox");

	THE_RenderCommand *last_tocube =
	  ConvertEqrToCubeCommandList(eqr_in, eqr_cube_out, auxfb, rendops);

	THE_Texture irrad_in =
	  THE_CreateTextureFromFile("assets/tex/env/helipad-dif.hdr",
	                            THE_TEX_RGB_F16);
	THE_Texture irrad_out = THE_ResourceMapGetTexture(rm, "Irradian");

	last_tocube = ConvertEqrToCubeCommandList(irrad_in, irrad_out, auxfb,
	                                          last_tocube);

	THE_Texture pref_out = THE_ResourceMapGetTexture(rm, "Prefilte");
	THE_RenderCommand *pref =
	  GeneratePrefilterCommandList(eqr_cube_out, pref_out, auxfb, last_tocube);

	GenerateLutCommandlist(auxfb, pref);
	THE_AddCommands(rendops);
}

void
Init(void *context)
{
	g_fb = THE_CreateFramebuffer(THE_WindowGetWidth(), THE_WindowGetHeight(),
	                             true, true);
	g_resources.meshes = THE_HMapCreate(8, sizeof(THE_Mesh));
	g_resources.textures = THE_HMapCreate(64, sizeof(THE_Texture));

	g_mats.fullscreen_img = THE_CreateShader("fullscreen-img");
	g_mats.skybox = THE_CreateShader("skybox");
	g_mats.eqr_to_cube = THE_CreateShader("eqr-to-cube");
	g_mats.prefilter_env = THE_CreateShader("prefilter-env");
	g_mats.lut_gen = THE_CreateShader("lut-gen");
	g_mats.pbr = THE_CreateShader("pbr");

	fulls.shader = g_mats.fullscreen_img;
	fulls.data_count = 0;
	fulls.tex_count = 1;
	fulls.cube_count = 0;
	THE_Texture *fst = THE_MaterialAlloc(&fulls);
	*fst = THE_GetFrameColor(g_fb);

	THE_ResourceMap *rm = &g_resources;

	THE_ResourceMapAddMeshFromPath(rm, "MatBall", "assets/obj/matball-n.obj");
	THE_ResourceMapAddTexture(rm, "Skybox", 1024, 1024, THE_TEX_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "Irradian", 1024, 1024, THE_TEX_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "Prefilte", 128, 128,
	                          THE_TEX_PREFILTER_ENVIRONMENT);
	THE_ResourceMapAddTexture(rm, "LutMap", 512, 512, THE_TEX_LUT);

	THE_ResourceMapAddTextureFromPath(
	  rm, "Gold_A", "assets/tex/celtic-gold/celtic-gold_A.png", THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Gold_N", "assets/tex/celtic-gold/celtic-gold_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Gold_M", "assets/tex/celtic-gold/celtic-gold_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Gold_R", "assets/tex/celtic-gold/celtic-gold_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Peel_A",
	                                  "assets/tex/peeling/peeling_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_N",
	                                  "assets/tex/peeling/peeling_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_M",
	                                  "assets/tex/peeling/peeling_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Peel_R",
	                                  "assets/tex/peeling/peeling_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Rust_A",
	                                  "assets/tex/rusted/rusted_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_N",
	                                  "assets/tex/rusted/rusted_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_M",
	                                  "assets/tex/rusted/rusted_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Rust_R",
	                                  "assets/tex/rusted/rusted_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Tiles_A",
	                                  "assets/tex/tiles/tiles_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_N",
	                                  "assets/tex/tiles/tiles_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_M",
	                                  "assets/tex/tiles/tiles_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Tiles_R",
	                                  "assets/tex/tiles/tiles_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(
	  rm, "Future_A", "assets/tex/ship-panels/ship-panels_A.png",
	  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Future_N", "assets/tex/ship-panels/ship-panels_N.png", THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Future_M", "assets/tex/ship-panels/ship-panels_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(
	  rm, "Future_R", "assets/tex/ship-panels/ship-panels_R.png", THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Shore_A",
	                                  "assets/tex/shore/shore_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_N",
	                                  "assets/tex/shore/shore_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_M",
	                                  "assets/tex/shore/shore_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Shore_R",
	                                  "assets/tex/shore/shore_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Cliff_A",
	                                  "assets/tex/cliff/cliff_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_N",
	                                  "assets/tex/cliff/cliff_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_M",
	                                  "assets/tex/cliff/cliff_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Cliff_R",
	                                  "assets/tex/cliff/cliff_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Granit_A",
	                                  "assets/tex/granite/granite_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_N",
	                                  "assets/tex/granite/granite_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_M",
	                                  "assets/tex/granite/granite_M.png",
	                                  THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Granit_R",
	                                  "assets/tex/granite/granite_R.png",
	                                  THE_TEX_R);

	THE_ResourceMapAddTextureFromPath(rm, "Foam_A",
	                                  "assets/tex/foam/foam_A.png",
	                                  THE_TEX_SRGB);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_N",
	                                  "assets/tex/foam/foam_N.png",
	                                  THE_TEX_RGB);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_M",
	                                  "assets/tex/foam/foam_M.png", THE_TEX_R);
	THE_ResourceMapAddTextureFromPath(rm, "Foam_R",
	                                  "assets/tex/foam/foam_R.png", THE_TEX_R);

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

	float position[3] = { -2.0f, 0.0f, 0.0f };

	// CelticGold
	{
		THE_Entity *e = THE_EntityCreate();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = THE_ResourceMapGetMesh(rm, "MatBall");
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Gold_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Gold_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Gold_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Gold_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Shore_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Shore_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Shore_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Shore_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Peel_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Peel_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Peel_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Peel_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Rust_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Rust_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Rust_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Rust_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Tiles_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Tiles_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Tiles_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Tiles_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Future_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Future_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Future_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Future_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Cliff_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Cliff_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Cliff_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Cliff_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Granit_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Granit_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Granit_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Granit_N");
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
		e->mat.data_count = sizeof(struct THE_PbrData) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct THE_PbrData *d = THE_MaterialAlloc(&e->mat);
		*d = pbr;
		THE_Texture *t = (THE_Texture *)d + e->mat.data_count;
		t[0] = THE_ResourceMapGetTexture(rm, "Foam_A");
		t[1] = THE_ResourceMapGetTexture(rm, "Foam_M");
		t[2] = THE_ResourceMapGetTexture(rm, "Foam_R");
		t[3] = THE_ResourceMapGetTexture(rm, "Foam_N");
	}

	GeneratePbrEnv();

	pbr_common.data_count = sizeof(struct THE_PbrSceneData) / sizeof(float);
	pbr_common.tex_count = 1;
	pbr_common.cube_count = 2;
	pbr_common.shader = g_mats.pbr;
	THE_Texture *pbr_scene_tex = THE_MaterialAlloc(&pbr_common);
	pbr_scene_tex += pbr_common.data_count;
	pbr_scene_tex[0] = THE_ResourceMapGetTexture(rm, "LutMap");
	pbr_scene_tex[1] = THE_ResourceMapGetTexture(rm, "Irradian");
	pbr_scene_tex[2] = THE_ResourceMapGetTexture(rm, "Prefilte");

	sky_common.data_count = 16;
	sky_common.tex_count = 0;
	sky_common.cube_count = 1;
	sky_common.shader = g_mats.skybox;
	THE_Texture *skytex = THE_MaterialAlloc(&sky_common);
	skytex[sky_common.data_count] = THE_ResourceMapGetTexture(rm, "Skybox");
}

bool
Update(void *context)
{
	THE_InputUpdate();
	THE_CameraMovementSystem(&camera, deltatime);

	/* PBR common shader data. */
	struct THE_PbrSceneData *common_pbr = pbr_common.ptr;
	mat4_multiply(common_pbr->view_projection, camera.proj_mat,
	              camera.view_mat);
	THE_CameraPosition(common_pbr->camera_position, &camera);
	vec4_assign(common_pbr->sunlight, g_sunlight);

	THE_RenderCommand *fbuff = THE_AllocateCommand();
	fbuff->data.set_fb.fb = g_fb;
	fbuff->data.set_fb.attachment.slot = THE_IGNORE;
	fbuff->execute = THE_SetFramebufferExecute;

	THE_RenderCommand *rops = THE_AllocateCommand();
	memset(&rops->data.rend_opts, 0, sizeof(THE_RenderOptionsData));
	rops->data.rend_opts.enable_flags = THE_BLEND | THE_DEPTH_TEST |
	  THE_DEPTH_WRITE;
	rops->data.rend_opts.blend_func.src = THE_BLEND_FUNC_ONE;
	rops->data.rend_opts.blend_func.dst = THE_BLEND_FUNC_ZERO;
	rops->data.rend_opts.depth_func = THE_DEPTH_FUNC_LESS;
	rops->data.rend_opts.cull_face = THE_CULL_FACE_BACK;
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
	use_pbr->data.mat = pbr_common;
	use_pbr->execute = THE_UseShaderExecute;
	clear->next = use_pbr;
	use_pbr->next = NULL;

	THE_AddCommands(fbuff);

	THE_RenderEntities(THE_GetEntities(), THE_EntitiesSize());

	rops = THE_AllocateCommand();
	memset(&rops->data.rend_opts, 0, sizeof(THE_RenderOptionsData));
	rops->data.rend_opts.disable_flags = THE_CULL_FACE;
	rops->data.rend_opts.depth_func = THE_DEPTH_FUNC_LEQUAL;
	rops->execute = THE_RenderOptionsExecute;

	THE_CameraStaticViewProjection(sky_common.ptr, &camera);
	THE_RenderCommand *use_sky_shader = THE_AllocateCommand();
	use_sky_shader->data.mat = sky_common;
	use_sky_shader->execute = THE_UseShaderExecute;
	rops->next = use_sky_shader;

	THE_RenderCommand *draw_sky = THE_AllocateCommand();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material = sky_common;
	draw_sky->execute = THE_DrawExecute;
	use_sky_shader->next = draw_sky;

	fbuff = THE_AllocateCommand();
	fbuff->data.set_fb.fb = THE_DEFAULT;
	fbuff->data.set_fb.attachment.slot = THE_IGNORE;
	fbuff->execute = THE_SetFramebufferExecute;
	draw_sky->next = fbuff;

	THE_RenderCommand *rops2 = THE_AllocateCommand();
	memset(&rops2->data.rend_opts, 0, sizeof(THE_RenderOptionsData));
	rops2->data.rend_opts.disable_flags = THE_DEPTH_TEST;
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
	usefullscreen->data.mat = fulls;
	usefullscreen->execute = THE_UseShaderExecute;
	clear->next = usefullscreen;

	THE_RenderCommand *draw = THE_AllocateCommand();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material = fulls;
	draw->execute = THE_DrawExecute;
	usefullscreen->next = draw;
	draw->next = NULL;

	THE_AddCommands(rops);

	return true;
}

int
main(int argc, char **argv)
{
	struct THE_Config cnfg = { .init_func = Init,
		                       .update_func = Update,
		                       .heap_size = THE_GB(1),
		                       .window_title = "THE Material Demo",
		                       .window_width = 1280,
		                       .window_height = 720,
		                       .vsync = true };

	THE_Start(&cnfg);

	THE_End();

	return 0;
}
