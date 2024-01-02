#include "nyas.h"
#include "../helpersdemo.h"
#include <mathc.h>
#include <string.h>

struct Shaders {
	nyas_shader fullscreen_img;
	nyas_shader skybox;
	nyas_shader eqr_to_cube;
	nyas_shader prefilter_env;
	nyas_shader lut_gen;
	nyas_shader pbr;
};

struct Textures {
	nyas_tex sky;
	nyas_tex lut;
	nyas_tex prefilter;
	nyas_tex irradiance;

	nyas_tex rusted_a;
	nyas_tex rusted_n;
	nyas_tex rusted_r;
	nyas_tex rusted_a;

	nyas_tex cliff_n;
	nyas_tex cliff_r;
	nyas_tex cliff_m;
	nyas_tex cliff_m;

	nyas_tex peeled_a;
	nyas_tex peeled_n;
	nyas_tex peeled_r;
	nyas_tex peeled_a;

	nyas_tex plastic_n;
	nyas_tex plastic_r;
	nyas_tex plastic_m;
	nyas_tex plastic_m;

	nyas_tex tiles_a;
	nyas_tex tiles_n;
	nyas_tex tiles_r;
	nyas_tex tiles_a;

	nyas_tex gold_n;
	nyas_tex gold_r;
	nyas_tex gold_m;
	nyas_tex gold_m;

	nyas_tex shore_a;
	nyas_tex shore_n;
	nyas_tex shore_r;
	nyas_tex shore_a;

	nyas_tex granite_a;
	nyas_tex granite_n;
	nyas_tex granite_r;
	nyas_tex granite_a;

	nyas_tex sponge_a;
	nyas_tex sponge_n;
	nyas_tex sponge_r;
	nyas_tex sponge_a;
};

struct Shaders g_shaders;
struct Textures g_tex;
static nyas_resourcemap g_resources;
nyas_framebuffer g_fb;

nyas_mat pbr_common;
nyas_mat sky_common;
nyas_mat fulls;

static float g_sunlight[4] = { 0.0f, -1.0f, -0.1f, 0.0f };

void
Init(void)
{
	g_fb = nyas_fb_create(nyas_window_width(), nyas_window_height(), true,
	                      true);
	g_resources.meshes = nyas_hmap_create(8, sizeof(nyas_mesh));
	g_resources.textures = nyas_hmap_create(64, sizeof(nyas_tex));

	nyas_shader_desc fs_img_shader_desc = {
		.name = "fullscreen-img",
		.data_count = 0,
		.tex_count = 0,
		.cubemap_count = 0,
		.common_data_count = 0,
		.common_tex_count = 1,
		.common_cubemap_count = 0
	};
	g_shaders.fullscreen_img = nyas_shader_create(&fs_img_shader_desc);

	nyas_shader_desc sky_shader_desc = {
		.name = "skybox",
		.data_count = 0,
		.tex_count = 0,
		.cubemap_count = 0,
		.common_data_count = 16,
		.common_tex_count = 0,
		.common_cubemap_count = 1
	};
	g_shaders.skybox = nyas_shader_create(&sky_shader_desc);

	nyas_shader_desc eqr_shader_desc = {
		.name = "eqr-to-cube",
		.data_count = 0,
		.tex_count = 0, // TODO: iba por aqui! hay que rellenar esta tambien.
		.cubemap_count = 0,
		.common_data_count = 16,
		.common_tex_count = 0,
		.common_cubemap_count = 1
	};
	g_shaders.eqr_to_cube = nyas_shader_create("eqr-to-cube");
	g_shaders.prefilter_env = nyas_shader_create("prefilter-env");
	g_shaders.lut_gen = nyas_shader_create("lut-gen");
	g_shaders.pbr = nyas_shader_create("pbr");

	fulls = nyas_mat_pers(g_shaders.fullscreen_img, 0, 1, 0);
	*(nyas_tex *)fulls.ptr = nyas_fb_color(g_fb);

	nyas_resourcemap *rm = &g_resources;

	nyas_resourcemap_mesh_file(rm, "MatBall", "assets/obj/matball.obj");
	int sky_flags = nyas_tex_flags(3, false, false, true, false, false, false);
	g_tex.sky = nyas_tex_empty(1024, 1024, sky_flags);
	nyas_resourcemap_tex_add(rm, "Irradian", 1024, 1024, );
	nyas_resourcemap_tex_add(rm, "Prefilte", 128, 128,
	                         NYAS_TEX_PREFILTER_ENVIRONMENT);
	nyas_resourcemap_tex_add(rm, "LutMap", 512, 512, NYAS_TEX_LUT);

	nyas_resourcemap_tex_file(rm, "Gold_A",
	                          "assets/tex/celtic-gold/celtic-gold_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Gold_N",
	                          "assets/tex/celtic-gold/celtic-gold_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Gold_M",
	                          "assets/tex/celtic-gold/celtic-gold_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Gold_R",
	                          "assets/tex/celtic-gold/celtic-gold_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Peel_A", "assets/tex/peeling/peeling_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Peel_N", "assets/tex/peeling/peeling_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Peel_M", "assets/tex/peeling/peeling_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Peel_R", "assets/tex/peeling/peeling_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Rust_A", "assets/tex/rusted/rusted_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Rust_N", "assets/tex/rusted/rusted_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Rust_M", "assets/tex/rusted/rusted_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Rust_R", "assets/tex/rusted/rusted_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Tiles_A", "assets/tex/tiles/tiles_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Tiles_N", "assets/tex/tiles/tiles_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Tiles_M", "assets/tex/tiles/tiles_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Tiles_R", "assets/tex/tiles/tiles_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Future_A",
	                          "assets/tex/ship-panels/ship-panels_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Future_N",
	                          "assets/tex/ship-panels/ship-panels_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Future_M",
	                          "assets/tex/ship-panels/ship-panels_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Future_R",
	                          "assets/tex/ship-panels/ship-panels_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Shore_A", "assets/tex/shore/shore_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Shore_N", "assets/tex/shore/shore_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Shore_M", "assets/tex/shore/shore_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Shore_R", "assets/tex/shore/shore_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Cliff_A", "assets/tex/cliff/cliff_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Cliff_N", "assets/tex/cliff/cliff_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Cliff_M", "assets/tex/cliff/cliff_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Cliff_R", "assets/tex/cliff/cliff_R.png",
	                          NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Granit_A",
	                          "assets/tex/granite/granite_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Granit_N",
	                          "assets/tex/granite/granite_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Granit_M",
	                          "assets/tex/granite/granite_M.png", NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Granit_R",
	                          "assets/tex/granite/granite_R.png", NYAS_TEX_R);

	nyas_resourcemap_tex_file(rm, "Foam_A", "assets/tex/foam/foam_A.png",
	                          NYAS_TEX_SRGB);
	nyas_resourcemap_tex_file(rm, "Foam_N", "assets/tex/foam/foam_N.png",
	                          NYAS_TEX_RGB);
	nyas_resourcemap_tex_file(rm, "Foam_M", "assets/tex/foam/foam_M.png",
	                          NYAS_TEX_R);
	nyas_resourcemap_tex_file(rm, "Foam_R", "assets/tex/foam/foam_R.png",
	                          NYAS_TEX_R);

	struct nyas_pbr_desc_unit pbr;
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
	int pbr_data_count = sizeof(nyas_pbr_desc_unit) / 4;

	// CelticGold
	{
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Gold_A");
		t[1] = nyas_resourcemap_tex(rm, "Gold_M");
		t[2] = nyas_resourcemap_tex(rm, "Gold_R");
		t[3] = nyas_resourcemap_tex(rm, "Gold_N");
	}

	// Shore
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		nyas_entity *e = nyas_entity_create();
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Shore_A");
		t[1] = nyas_resourcemap_tex(rm, "Shore_M");
		t[2] = nyas_resourcemap_tex(rm, "Shore_R");
		t[3] = nyas_resourcemap_tex(rm, "Shore_N");
	}

	// Peeling
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.7f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Peel_A");
		t[1] = nyas_resourcemap_tex(rm, "Peel_M");
		t[2] = nyas_resourcemap_tex(rm, "Peel_R");
		t[3] = nyas_resourcemap_tex(rm, "Peel_N");
	}

	// Rusted
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.2f;
		position[0] = -2.0f;
		position[2] = -2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Rust_A");
		t[1] = nyas_resourcemap_tex(rm, "Rust_M");
		t[2] = nyas_resourcemap_tex(rm, "Rust_R");
		t[3] = nyas_resourcemap_tex(rm, "Rust_N");
	}

	// Tiles
	{
		pbr.tiling_x = 4.0f;
		pbr.tiling_y = 4.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Tiles_A");
		t[1] = nyas_resourcemap_tex(rm, "Tiles_M");
		t[2] = nyas_resourcemap_tex(rm, "Tiles_R");
		t[3] = nyas_resourcemap_tex(rm, "Tiles_N");
	}

	// Ship Panels
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Future_A");
		t[1] = nyas_resourcemap_tex(rm, "Future_M");
		t[2] = nyas_resourcemap_tex(rm, "Future_R");
		t[3] = nyas_resourcemap_tex(rm, "Future_N");
	}

	// Cliff
	{
		pbr.tiling_x = 8.0f;
		pbr.tiling_y = 8.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = -2.0f;
		position[2] = -4.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Cliff_A");
		t[1] = nyas_resourcemap_tex(rm, "Cliff_M");
		t[2] = nyas_resourcemap_tex(rm, "Cliff_R");
		t[3] = nyas_resourcemap_tex(rm, "Cliff_N");
	}

	// Granite
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Granit_A");
		t[1] = nyas_resourcemap_tex(rm, "Granit_M");
		t[2] = nyas_resourcemap_tex(rm, "Granit_R");
		t[3] = nyas_resourcemap_tex(rm, "Granit_N");
	}

	// Foam
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat = nyas_mat_pers(g_shaders.pbr, pbr_data_count, 4, 0);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(&e->mat);
		t[0] = nyas_resourcemap_tex(rm, "Foam_A");
		t[1] = nyas_resourcemap_tex(rm, "Foam_M");
		t[2] = nyas_resourcemap_tex(rm, "Foam_R");
		t[3] = nyas_resourcemap_tex(rm, "Foam_N");
	}

	int pbr_cmn_data_count = sizeof(nyas_pbr_desc_scene) / sizeof(float);
	pbr_common = nyas_mat_pers(g_shaders.pbr, pbr_cmn_data_count, 1, 2);
	nyas_tex *pbr_scene_tex = nyas_mat_tex(&pbr_common);
	pbr_scene_tex[0] = nyas_resourcemap_tex(rm, "LutMap");
	pbr_scene_tex[1] = nyas_resourcemap_tex(rm, "Irradian");
	pbr_scene_tex[2] = nyas_resourcemap_tex(rm, "Prefilte");

	sky_common = nyas_mat_pers(g_shaders.skybox, 16, 0, 1);
	*nyas_mat_tex(&sky_common) = nyas_resourcemap_tex(rm, "Skybox");

	GenEnv env = {
		.eqr_sh = g_shaders.eqr_to_cube,
		.lut_sh = g_shaders.lut_gen,
		.pref_sh = g_shaders.prefilter_env,
		.eqr_in = nyas_tex_load_img("assets/tex/env/helipad-env.hdr", NYAS_TEX_RGB_F16),
		.eqr_out = nyas_resourcemap_tex(rm, "Skybox"),
		.irr_in = nyas_tex_load_img("assets/tex/env/helipad-dif.hdr", NYAS_TEX_RGB_F16),
		.irr_out = nyas_resourcemap_tex(rm, "Irradian"),
		.pref_out = nyas_resourcemap_tex(rm, "Prefilte"),
		.lut = nyas_resourcemap_tex(rm, "LutMap")
	};

	GeneratePbrEnv(&env);
}

void
Update(nyas_chrono *chrono)
{
	float dt = nyas_time_sec(nyas_elapsed(*chrono));
	*chrono = nyas_time();

	nyas_io_poll();
	nyas_input_read();
	nyas_camera_control(&camera, dt);

	/* PBR common shader data. */
	struct nyas_pbr_desc_scene *common_pbr = pbr_common.ptr;
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);
	vec4_assign(common_pbr->sunlight, g_sunlight);

	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = g_fb;
	fbuff->data.set_fb.vp_x = NYAS_IGNORE;
	fbuff->data.set_fb.attachment.slot = NYAS_IGNORE;
	fbuff->execute = nyas_setfb_fn;

	nyas_cmd *rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.enable_flags = NYAS_BLEND | NYAS_DEPTH_TEST |
	  NYAS_DEPTH_WRITE;
	rops->data.rend_opts.blend_func.src = NYAS_BLEND_FUNC_ONE;
	rops->data.rend_opts.blend_func.dst = NYAS_BLEND_FUNC_ZERO;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LESS;
	rops->data.rend_opts.cull_face = NYAS_CULL_FACE_BACK;
	rops->execute = nyas_rops_fn;
	fbuff->next = rops;

	nyas_cmd *clear = nyas_cmd_alloc();
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = true;
	clear->data.clear.stencil_buffer = false;
	nyas_set_color(clear->data.clear.color, 0.2f, 0.2f, 0.2f, 1.0f);
	clear->execute = nyas_clear_fn;
	rops->next = clear;

	nyas_cmd *use_pbr = nyas_cmd_alloc();
	use_pbr->data.mat = pbr_common;
	use_pbr->execute = nyas_setshader_fn;
	clear->next = use_pbr;
	use_pbr->next = NULL;

	nyas_cmd_add(fbuff);

	nyas_entity_draw(nyas_entities(), nyas_entity_count());

	rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LEQUAL;
	rops->execute = nyas_rops_fn;

	nyas_camera_static_vp(sky_common.ptr, &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = sky_common;
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material = sky_common;
	draw_sky->execute = nyas_draw_fn;
	use_sky_shader->next = draw_sky;

	fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = NYAS_DEFAULT;
	fbuff->data.set_fb.attachment.slot = NYAS_IGNORE;
	fbuff->execute = nyas_setfb_fn;
	draw_sky->next = fbuff;

	nyas_cmd *rops2 = nyas_cmd_alloc();
	memset(&rops2->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops2->data.rend_opts.disable_flags = NYAS_DEPTH_TEST;
	rops2->execute = nyas_rops_fn;
	fbuff->next = rops2;

	clear = nyas_cmd_alloc();
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = false;
	clear->data.clear.stencil_buffer = false;
	nyas_set_color(clear->data.clear.color, 1.0f, 0.0f, 0.0f, 1.0f);
	clear->execute = nyas_clear_fn;
	rops2->next = clear;

	nyas_cmd *usefullscreen = nyas_cmd_alloc();
	usefullscreen->data.mat = fulls;
	usefullscreen->execute = nyas_setshader_fn;
	clear->next = usefullscreen;

	nyas_cmd *draw = nyas_cmd_alloc();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material = fulls;
	draw->execute = nyas_draw_fn;
	usefullscreen->next = draw;
	draw->next = NULL;

	nyas_cmd_add(rops);
}

void
Render(void)
{
	nyas_px_render();
	nyas_imgui_draw();
	nyas_window_swap();
	nyas_frame_end();
}

int
main(int argc, char **argv)
{
	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS PBR Material Demo", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);
	nyas_imgui_init();
	Init();

	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_window_closed()) {
		Update(&frame_chrono);
		Render();
	}

	return 0;
}
