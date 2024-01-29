#include "nyas.h"
#include "pbr.h"
#include "gui.h"
#include <mathc.h>
#include <string.h>

struct Shaders {
	nyas_shader fullscreen_img;
	nyas_shader skybox;
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
	nyas_tex rusted_m;

	nyas_tex cliff_a;
	nyas_tex cliff_n;
	nyas_tex cliff_r;
	nyas_tex cliff_m;

	nyas_tex peeled_a;
	nyas_tex peeled_n;
	nyas_tex peeled_r;
	nyas_tex peeled_m;

	nyas_tex plastic_a;
	nyas_tex plastic_n;
	nyas_tex plastic_r;
	nyas_tex plastic_m;

	nyas_tex tiles_a;
	nyas_tex tiles_n;
	nyas_tex tiles_r;
	nyas_tex tiles_m;

	nyas_tex gold_a;
	nyas_tex gold_n;
	nyas_tex gold_r;
	nyas_tex gold_m;

	nyas_tex shore_a;
	nyas_tex shore_n;
	nyas_tex shore_r;
	nyas_tex shore_m;

	nyas_tex granite_a;
	nyas_tex granite_n;
	nyas_tex granite_r;
	nyas_tex granite_m;

	nyas_tex foam_a;
	nyas_tex foam_n;
	nyas_tex foam_r;
	nyas_tex foam_m;
};

struct Shaders g_shaders;
struct Textures g_tex;
nyas_framebuffer g_fb;
nyas_tex fb_tex;
nyas_mesh g_mesh;

void
Init(void)
{
	g_fb = nyas_fb_create();

	g_shaders.fullscreen_img = nyas_shader_create(g_shader_descriptors.fullscreen_img);
	g_shaders.skybox = nyas_shader_create(g_shader_descriptors.sky);
	g_shaders.pbr = nyas_shader_create(g_shader_descriptors.pbr);

	g_mesh = nyas_mesh_load_file("assets/obj/matball.msh");
	nyas_load_env("assets/env/helipad.env", &g_tex.lut, &g_tex.sky, &g_tex.irradiance, &g_tex.prefilter);

	g_tex.gold_a = nyas_tex_load("assets/tex/celtic-gold/celtic-gold_A.png", 1, g_tex_flags.albedo);
	g_tex.gold_n = nyas_tex_load("assets/tex/celtic-gold/celtic-gold_N.png", 1, g_tex_flags.normal);
	g_tex.gold_r = nyas_tex_load("assets/tex/celtic-gold/celtic-gold_R.png", 1, g_tex_flags.pbr_map);
	g_tex.gold_m = nyas_tex_load("assets/tex/celtic-gold/celtic-gold_M.png", 1, g_tex_flags.pbr_map);

	g_tex.peeled_a = nyas_tex_load("assets/tex/peeling/peeling_A.png", 1, g_tex_flags.albedo);
	g_tex.peeled_n = nyas_tex_load("assets/tex/peeling/peeling_N.png", 1, g_tex_flags.normal);
	g_tex.peeled_r = nyas_tex_load("assets/tex/peeling/peeling_R.png", 1, g_tex_flags.pbr_map);
	g_tex.peeled_m = nyas_tex_load("assets/tex/peeling/peeling_M.png", 1, g_tex_flags.pbr_map);

	g_tex.rusted_a = nyas_tex_load("assets/tex/rusted/rusted_A.png", 1, g_tex_flags.albedo);
	g_tex.rusted_n = nyas_tex_load("assets/tex/rusted/rusted_N.png", 1, g_tex_flags.normal);
	g_tex.rusted_r = nyas_tex_load("assets/tex/rusted/rusted_R.png", 1, g_tex_flags.pbr_map);
	g_tex.rusted_m = nyas_tex_load("assets/tex/rusted/rusted_M.png", 1, g_tex_flags.pbr_map);

	g_tex.tiles_a = nyas_tex_load("assets/tex/tiles/tiles_A.png", 1, g_tex_flags.albedo);
	g_tex.tiles_n = nyas_tex_load("assets/tex/tiles/tiles_N.png", 1, g_tex_flags.normal);
	g_tex.tiles_r = nyas_tex_load("assets/tex/tiles/tiles_R.png", 1, g_tex_flags.pbr_map);
	g_tex.tiles_m = nyas_tex_load("assets/tex/tiles/tiles_M.png", 1, g_tex_flags.pbr_map);
	
	g_tex.plastic_a = nyas_tex_load("assets/tex/ship-panels/ship-panels_A.png", 1, g_tex_flags.albedo);
	g_tex.plastic_n = nyas_tex_load("assets/tex/ship-panels/ship-panels_N.png", 1, g_tex_flags.normal);
	g_tex.plastic_r = nyas_tex_load("assets/tex/ship-panels/ship-panels_R.png", 1, g_tex_flags.pbr_map);
	g_tex.plastic_m = nyas_tex_load("assets/tex/ship-panels/ship-panels_M.png", 1, g_tex_flags.pbr_map);

	g_tex.shore_a = nyas_tex_load("assets/tex/shore/shore_A.png", 1, g_tex_flags.albedo);
	g_tex.shore_n = nyas_tex_load("assets/tex/shore/shore_N.png", 1, g_tex_flags.normal);
	g_tex.shore_r = nyas_tex_load("assets/tex/shore/shore_R.png", 1, g_tex_flags.pbr_map);
	g_tex.shore_m = nyas_tex_load("assets/tex/shore/shore_M.png", 1, g_tex_flags.pbr_map);

	g_tex.cliff_a = nyas_tex_load("assets/tex/cliff/cliff_A.png", 1, g_tex_flags.albedo);
	g_tex.cliff_n = nyas_tex_load("assets/tex/cliff/cliff_N.png", 1, g_tex_flags.normal);
	g_tex.cliff_r = nyas_tex_load("assets/tex/cliff/cliff_R.png", 1, g_tex_flags.pbr_map);
	g_tex.cliff_m = nyas_tex_load("assets/tex/cliff/cliff_M.png", 1, g_tex_flags.pbr_map);

	g_tex.granite_a = nyas_tex_load("assets/tex/granite/granite_A.png", 1, g_tex_flags.albedo);
	g_tex.granite_n = nyas_tex_load("assets/tex/granite/granite_N.png", 1, g_tex_flags.normal);
	g_tex.granite_r = nyas_tex_load("assets/tex/granite/granite_R.png", 1, g_tex_flags.pbr_map);
	g_tex.granite_m = nyas_tex_load("assets/tex/granite/granite_M.png", 1, g_tex_flags.pbr_map);

	g_tex.granite_a = nyas_tex_load("assets/tex/granite/granite_A.png", 1, g_tex_flags.albedo);
	g_tex.granite_n = nyas_tex_load("assets/tex/granite/granite_N.png", 1, g_tex_flags.normal);
	g_tex.granite_r = nyas_tex_load("assets/tex/granite/granite_R.png", 1, g_tex_flags.pbr_map);
	g_tex.granite_m = nyas_tex_load("assets/tex/granite/granite_M.png", 1, g_tex_flags.pbr_map);

	g_tex.foam_a = nyas_tex_load("assets/tex/foam/foam_A.png", 1, g_tex_flags.albedo);
	g_tex.foam_n = nyas_tex_load("assets/tex/foam/foam_N.png", 1, g_tex_flags.normal);
	g_tex.foam_r = nyas_tex_load("assets/tex/foam/foam_R.png", 1, g_tex_flags.pbr_map);
	g_tex.foam_m = nyas_tex_load("assets/tex/foam/foam_M.png", 1, g_tex_flags.pbr_map);

	struct nyas_pbr_desc_unit pbr;
	pbr.color[0] = 1.0f;
	pbr.color[1] = 1.0f;
	pbr.color[2] = 1.0f;
	pbr.tiling_x = 4.0f;
	pbr.tiling_y = 4.0f;
	pbr.reflectance = 0.5f;
	pbr.use_albedo_map = 1.0f;
	pbr.use_pbr_maps = 1.0f;
	pbr.metallic = 0.5f;
	pbr.roughness = 0.5f;
	pbr.normal_map_intensity = 1.0f;

	float position[3] = { -2.0f, 0.0f, 0.0f };

	// CelticGold
	{
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.gold_a;
		t[1] = g_tex.gold_m;
		t[2] = g_tex.gold_r;
		t[3] = g_tex.gold_n;
	}

	// Shore
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		nyas_entity *e = nyas_entity_create();
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.shore_a;
		t[1] = g_tex.shore_m;
		t[2] = g_tex.shore_r;
		t[3] = g_tex.shore_n;
	}

	// Peeling
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.7f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.peeled_a;
		t[1] = g_tex.peeled_m;
		t[2] = g_tex.peeled_r;
		t[3] = g_tex.peeled_n;
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
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.rusted_a;
		t[1] = g_tex.rusted_m;
		t[2] = g_tex.rusted_r;
		t[3] = g_tex.rusted_n;
	}

	// Tiles
	{
		pbr.tiling_x = 4.0f;
		pbr.tiling_y = 4.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.tiles_a;
		t[1] = g_tex.tiles_m;
		t[2] = g_tex.tiles_r;
		t[3] = g_tex.tiles_n;
	}

	// Ship Panels
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.plastic_a;
		t[1] = g_tex.plastic_m;
		t[2] = g_tex.plastic_r;
		t[3] = g_tex.plastic_n;
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
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.cliff_a;
		t[1] = g_tex.cliff_m;
		t[2] = g_tex.cliff_r;
		t[3] = g_tex.cliff_n;
	}

	// Granite
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.granite_a;
		t[1] = g_tex.granite_m;
		t[2] = g_tex.granite_r;
		t[3] = g_tex.granite_n;
	}

	// Foam
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		position[0] = 2.0f;
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_pers(g_shaders.pbr);
		*(nyas_pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.foam_a;
		t[1] = g_tex.foam_m;
		t[2] = g_tex.foam_r;
		t[3] = g_tex.foam_n;
	}

	struct nyas_pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	common_pbr->sunlight[0] = 0.0f;
	common_pbr->sunlight[1] = -1.0f;
	common_pbr->sunlight[2] = -0.1f;
	common_pbr->sunlight[3] = 0.0f;

	nyas_tex *pbr_scene_tex = nyas_shader_tex(g_shaders.pbr);
	pbr_scene_tex[0] = g_tex.lut;
	pbr_scene_tex[1] = g_tex.irradiance;
	pbr_scene_tex[2] = g_tex.prefilter;

	*nyas_shader_tex(g_shaders.skybox) = g_tex.sky;
	fb_tex = InitMainFramebuffer(g_fb);
	*nyas_shader_tex(g_shaders.fullscreen_img) = fb_tex;
}

void
Update(nyas_chrono *chrono)
{
	float dt = nyas_time_sec(nyas_elapsed(*chrono));
	*chrono = nyas_time();

	nyas_io_poll();
	nyas_input_read();
	nyas_v2i vp = nyas_window_size();
	nyas_camera_control(&camera, dt);

	/* PBR common shader data. */
	struct nyas_pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);

	int fb_size[2];
	nyas_tex_size(fb_tex, fb_size);
	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = g_fb;
	fbuff->data.set_fb.vp_x = fb_size[0];
	fbuff->data.set_fb.vp_y = fb_size[1];
	fbuff->data.set_fb.attach.type = NYAS_IGNORE;
	fbuff->execute = nyas_setfb_fn;

	nyas_cmd *rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.enable_flags = NYAS_BLEND | NYAS_DEPTH_TEST |
	  NYAS_DEPTH_WRITE;
	rops->data.rend_opts.blend_src = NYAS_BLEND_ONE;
	rops->data.rend_opts.blend_dst = NYAS_BLEND_ZERO;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_LESS;
	rops->data.rend_opts.cull_face = NYAS_CULL_BACK;
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
	use_pbr->data.mat = nyas_mat_copy_shader(g_shaders.pbr);
	use_pbr->execute = nyas_setshader_fn;
	clear->next = use_pbr;
	use_pbr->next = NULL;

	nyas_cmd_add(fbuff);

	nyas_entity_draw(nyas_entities(), nyas_entity_count());

	rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_LEQUAL;
	rops->execute = nyas_rops_fn;

	nyas_camera_static_vp(nyas_shader_data(g_shaders.skybox), &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = nyas_mat_copy_shader(g_shaders.skybox);
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material.shader = g_shaders.skybox;
	draw_sky->execute = nyas_draw_fn;
	use_sky_shader->next = draw_sky;

	fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = NYAS_DEFAULT;
	fbuff->data.set_fb.vp_x = vp.x;
	fbuff->data.set_fb.vp_y = vp.y;
	fbuff->data.set_fb.attach.type = NYAS_IGNORE;
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
	usefullscreen->data.mat = nyas_mat_copy_shader(g_shaders.fullscreen_img);
	usefullscreen->execute = nyas_setshader_fn;
	clear->next = usefullscreen;

	nyas_cmd *draw = nyas_cmd_alloc();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material.shader = g_shaders.fullscreen_img;
	draw->execute = nyas_draw_fn;
	usefullscreen->next = draw;
	draw->next = NULL;

	nyas_cmd_add(rops);
}

void
Render(void)
{
	nyas_px_render();
	nuklear_draw();
	nyas_window_swap();
	nyas_frame_end();
}

int
main(int argc, char **argv)
{
	(void)argc; (void)argv;
	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS PBR Material Demo", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);
	nuklear_init();
	Init();

	nyas_chrono frame_chrono = nyas_time();
	Update(&frame_chrono);
	nyas_frame_end();
	Render();
	while (!nyas_window_closed()) {
		Update(&frame_chrono);
		Render();
	}

	return 0;
}
