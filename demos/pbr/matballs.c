#include "gui.h"
#include "nyas.h"
#include "pbr.h"
#include <mathc.h>
#include <string.h>

struct Shaders {
	nyas_shader fullscreen_img;
	nyas_shader skybox;
	nyas_shader pbr;
};

struct pbr_tex_t {
	nyas_tex a, n, r, m;
};

struct Textures {
	nyas_tex sky;
	nyas_tex lut;
	nyas_tex prefilter;
	nyas_tex irradiance;

	struct pbr_tex_t rusted;
	struct pbr_tex_t cliff;
	struct pbr_tex_t peeled;
	struct pbr_tex_t plastic;
	struct pbr_tex_t tiles;
	struct pbr_tex_t gold;
	struct pbr_tex_t shore;
	struct pbr_tex_t granite;
	struct pbr_tex_t foam;
};

struct Shaders g_shaders;
struct Textures g_tex;
nyas_framebuffer g_fb;
nyas_tex fb_tex;
nyas_mesh g_mesh;

static struct pbr_tex_desc g_tex_desc;

struct pbr_tex_load_info_t {
	struct pbr_tex_desc *desc;
	struct pbr_tex_t tex;
	const char *apath;
	const char *npath;
	const char *rpath;
	const char *mpath;
};

void load_pbr_map(void *pbr_maps)
{
	struct pbr_tex_load_info_t *maps = pbr_maps;
	maps->tex.a = nyas_tex_load(&maps->desc->albedo, maps->apath, 0);
	maps->tex.n = nyas_tex_load(&maps->desc->normal, maps->npath, 3);
	maps->tex.r = nyas_tex_load(&maps->desc->pbr_map, maps->rpath, 1);
	maps->tex.m = nyas_tex_load(&maps->desc->pbr_map, maps->mpath, 1);
}

static void load_textures(void)
{
	struct pbr_tex_load_info_t gold_maps = {
		.desc = &g_tex_desc,
		.tex = g_tex.gold,
		.apath = "assets/tex/celtic-gold/celtic-gold_A.png",
		.npath = "assets/tex/celtic-gold/celtic-gold_N.png",
		.rpath = "assets/tex/celtic-gold/celtic-gold_R.png",
		.mpath = "assets/tex/celtic-gold/celtic-gold_M.png",
	};

	struct pbr_tex_load_info_t peeled_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.peeled,
	  .apath = "assets/tex/peeling/peeling_A.png",
	  .npath = "assets/tex/peeling/peeling_N.png",
	  .rpath = "assets/tex/peeling/peeling_R.png",
	  .mpath = "assets/tex/peeling/peeling_M.png",
	};

	struct pbr_tex_load_info_t rusted_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.rusted,
	  .apath = "assets/tex/rusted/rusted_A.png",
	  .npath = "assets/tex/rusted/rusted_N.png",
	  .rpath = "assets/tex/rusted/rusted_R.png",
	  .mpath = "assets/tex/rusted/rusted_M.png",
	};

	struct pbr_tex_load_info_t tiles_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.tiles,
	  .apath = "assets/tex/tiles/tiles_A.png",
	  .npath = "assets/tex/tiles/tiles_N.png",
	  .rpath = "assets/tex/tiles/tiles_R.png",
	  .mpath = "assets/tex/tiles/tiles_M.png",
	};

	struct pbr_tex_load_info_t plastic_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.plastic,
	  .apath = "assets/tex/ship-panels/ship-panels_A.png",
	  .npath = "assets/tex/ship-panels/ship-panels_N.png",
	  .rpath = "assets/tex/ship-panels/ship-panels_R.png",
	  .mpath = "assets/tex/ship-panels/ship-panels_M.png",
	};

	struct pbr_tex_load_info_t shore_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.shore,
	  .apath = "assets/tex/shore/shore_A.png",
	  .npath = "assets/tex/shore/shore_N.png",
	  .rpath = "assets/tex/shore/shore_R.png",
	  .mpath = "assets/tex/shore/shore_M.png",
	};

	struct pbr_tex_load_info_t cliff_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.cliff,
	  .apath = "assets/tex/cliff/cliff_A.png",
	  .npath = "assets/tex/cliff/cliff_N.png",
	  .rpath = "assets/tex/cliff/cliff_R.png",
	  .mpath = "assets/tex/cliff/cliff_M.png",
	};

	struct pbr_tex_load_info_t granite_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.granite,
	  .apath = "assets/tex/granite/granite_A.png",
	  .npath = "assets/tex/granite/granite_N.png",
	  .rpath = "assets/tex/granite/granite_R.png",
	  .mpath = "assets/tex/granite/granite_M.png",
	};

	struct pbr_tex_load_info_t foam_maps = {
	  .desc = &g_tex_desc,
	  .tex = g_tex.foam,
	  .apath = "assets/tex/foam/foam_A.png",
	  .npath = "assets/tex/foam/foam_N.png",
	  .rpath = "assets/tex/foam/foam_R.png",
	  .mpath = "assets/tex/foam/foam_M.png",
	};

	{
		struct nyas_job job = { .job = load_pbr_map, .args = &gold_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &peeled_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &rusted_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &tiles_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &plastic_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &shore_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &granite_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &foam_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &gold_maps};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_pbr_map, .args = &cliff_maps};
		nyas_sched_do(job);
	}

	nyas_sched_wait();
}

void
Init(void)
{
	g_fb = nyas_fb_create();

	g_tex_desc.albedo = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_SRGB, 0, 0);
	g_tex_desc.normal = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_RGB8, 0, 0);
	g_tex_desc.pbr_map = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_R8, 0, 0);

	g_shaders.fullscreen_img = nyas_shader_create(g_shader_descriptors.fullscreen_img);
	g_shaders.skybox = nyas_shader_create(g_shader_descriptors.sky);
	g_shaders.pbr = nyas_shader_create(g_shader_descriptors.pbr);

	g_mesh = nyas_mesh_load_file("assets/obj/matball.msh");
	nyas_load_env(
	  "assets/env/canyon.env", &g_tex.lut, &g_tex.sky, &g_tex.irradiance, &g_tex.prefilter);

	load_textures();

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
		t[0] = g_tex.gold.a;
		t[1] = g_tex.gold.m;
		t[2] = g_tex.gold.r;
		t[3] = g_tex.gold.n;
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
		t[0] = g_tex.shore.a;
		t[1] = g_tex.shore.m;
		t[2] = g_tex.shore.r;
		t[3] = g_tex.shore.n;
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
		t[0] = g_tex.peeled.a;
		t[1] = g_tex.peeled.m;
		t[2] = g_tex.peeled.r;
		t[3] = g_tex.peeled.n;
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
		t[0] = g_tex.rusted.a;
		t[1] = g_tex.rusted.m;
		t[2] = g_tex.rusted.r;
		t[3] = g_tex.rusted.n;
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
		t[0] = g_tex.tiles.a;
		t[1] = g_tex.tiles.m;
		t[2] = g_tex.tiles.r;
		t[3] = g_tex.tiles.n;
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
		t[0] = g_tex.plastic.a;
		t[1] = g_tex.plastic.m;
		t[2] = g_tex.plastic.r;
		t[3] = g_tex.plastic.n;
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
		t[0] = g_tex.cliff.a;
		t[1] = g_tex.cliff.m;
		t[2] = g_tex.cliff.r;
		t[3] = g_tex.cliff.n;
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
		t[0] = g_tex.granite.a;
		t[1] = g_tex.granite.m;
		t[2] = g_tex.granite.r;
		t[3] = g_tex.granite.n;
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
		t[0] = g_tex.foam.a;
		t[1] = g_tex.foam.m;
		t[2] = g_tex.foam.r;
		t[3] = g_tex.foam.n;
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
	struct nyas_vec2i vp = nyas_window_size();
	nyas_camera_control(&camera, dt);

	/* PBR common shader data. */
	struct nyas_pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);

	struct nyas_vec2i fb_size = nyas_tex_size(fb_tex);
	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = g_fb;
	fbuff->data.set_fb.vp_x = fb_size.x;
	fbuff->data.set_fb.vp_y = fb_size.y;
	fbuff->data.set_fb.attach.type = NYAS_IGNORE;
	fbuff->execute = nyas_setfb_fn;

	nyas_cmd *rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.enable_flags = NYAS_BLEND | NYAS_DEPTH_TEST | NYAS_DEPTH_WRITE;
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
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
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
	clear->data.clear.color[0] = 1.0f;
	clear->data.clear.color[1] = 0.0f;
	clear->data.clear.color[2] = 0.0f;
	clear->data.clear.color[3] = 1.0f;
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
	(void)argc;
	(void)argv;
	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS PBR Material Demo", 1920, 1080, true);
	nyas_sched_init(9, 32);
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
