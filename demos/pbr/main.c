#include "gui.h"
#include "pbr.h"

#include <the.h>
#include <mathc.h>
#include <string.h>
#include <stdlib.h>

static void dummyfree(void *p)
{
	(void)p;
}

typedef struct the_draw thedraw;
THE_DECL_ARR(thedraw);
THE_IMPL_ARR_MA(thedraw, the_falloc, dummyfree);

struct {
	the_shader fullscreen_img;
	the_shader skybox;
	the_shader pbr;
} g_shaders;

struct {
	the_tex sky;

	struct pbr_maps gold;
	struct pbr_maps peeled;
	struct pbr_maps rusted;
	struct pbr_maps tiles;
	struct pbr_maps plastic;
	struct pbr_maps shore;
	struct pbr_maps cliff;
	struct pbr_maps granite;
	struct pbr_maps foam;
} g_tex;

the_framebuffer g_fb;
the_tex fb_tex;
the_mesh g_mesh;

void
Init(void)
{
	tut_mesh_init_geometry();
	the_tex irradiance;
	the_tex prefilter;
	the_tex lut;

	// TODO: create repe, porque peta sino?
	g_shaders.fullscreen_img = the_shader_create(&g_shader_descriptors.fullscreen_img);
	g_shaders.skybox = the_shader_create(&g_shader_descriptors.sky);
	g_shaders.pbr = the_shader_create(&g_shader_descriptors.pbr);

	struct tut_shad_ldargs fsimgargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct tut_shad_ldargs skyargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct tut_shad_ldargs pbrargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct tut_mesh_ldargs mesh = {
		.path = "assets/obj/matball.msh",
		.mesh = &g_mesh
	};

	struct tut_env_ldargs envargs = {
		.path = "assets/env/canyon.env",
		.sky = &g_tex.sky,
		.irr = &irradiance,
		.pref = &prefilter,
		.lut = &lut
	};

	tut_assetldr *ldr = tut_assets_create();
	tut_assets_add_shader(ldr, &fsimgargs);
	tut_assets_add_shader(ldr, &skyargs);
	tut_assets_add_shader(ldr, &pbrargs);
	tut_assets_add_mesh(ldr, &mesh);
	tut_assets_add_env(ldr, &envargs);

	struct the_texture_desc texdesc[] = {
		tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_SRGB, 0, 0),
		tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_RGB8, 0, 0),
		tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_R8, 0, 0),
		tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_R8, 0, 0)
	};

	const char *texpaths[] = {
		"assets/tex/celtic-gold/celtic-gold_A.png",
		"assets/tex/celtic-gold/celtic-gold_N.png",
		"assets/tex/celtic-gold/celtic-gold_R.png",
		"assets/tex/celtic-gold/celtic-gold_M.png",
		"assets/tex/peeling/peeling_A.png",
		"assets/tex/peeling/peeling_N.png",
		"assets/tex/peeling/peeling_R.png",
		"assets/tex/peeling/peeling_M.png",
		"assets/tex/rusted/rusted_A.png",
		"assets/tex/rusted/rusted_N.png",
		"assets/tex/rusted/rusted_R.png",
		"assets/tex/rusted/rusted_M.png",
		"assets/tex/tiles/tiles_A.png",
		"assets/tex/tiles/tiles_N.png",
		"assets/tex/tiles/tiles_R.png",
		"assets/tex/tiles/tiles_M.png",
		"assets/tex/ship-panels/ship-panels_A.png",
		"assets/tex/ship-panels/ship-panels_N.png",
		"assets/tex/ship-panels/ship-panels_R.png",
		"assets/tex/ship-panels/ship-panels_M.png",
		"assets/tex/shore/shore_A.png",
		"assets/tex/shore/shore_N.png",
		"assets/tex/shore/shore_R.png",
		"assets/tex/shore/shore_M.png",
		"assets/tex/cliff/cliff_A.png",
		"assets/tex/cliff/cliff_N.png",
		"assets/tex/cliff/cliff_R.png",
		"assets/tex/cliff/cliff_M.png",
		"assets/tex/granite/granite_A.png",
		"assets/tex/granite/granite_N.png",
		"assets/tex/granite/granite_R.png",
		"assets/tex/granite/granite_M.png",
		"assets/tex/foam/foam_A.png",
		"assets/tex/foam/foam_N.png",
		"assets/tex/foam/foam_R.png",
		"assets/tex/foam/foam_M.png"
	};

	struct tut_tex_ldargs loadtexargs[9 * 4];
	the_tex *tex_begin = &g_tex.gold.a;
	for (int i = 0; i < 9 * 4; ++i) {
		*tex_begin = the_tex_create();
		loadtexargs[i].tex = *tex_begin++;
		loadtexargs[i].desc = texdesc[i & 3];
		loadtexargs[i].path = texpaths[i];
	}

	for (int i = 0; i < 9 * 4; ++i) {
		tut_assets_add_tex(ldr, &loadtexargs[i]);
	}

	tut_assets_load(ldr, 24);

	struct pbr_desc_scene *common_pbr = the_shader_data(g_shaders.pbr);
	common_pbr->sunlight[0] = 0.0f;
	common_pbr->sunlight[1] = -1.0f;
	common_pbr->sunlight[2] = -0.1f;
	common_pbr->sunlight[3] = 0.0f;

	the_tex *pbr_scene_tex = the_shader_tex(g_shaders.pbr);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irradiance;
	pbr_scene_tex[2] = prefilter;

	g_fb = the_fb_create();
	struct the_point *vp = &the_io->window_size;
	struct the_texture_desc descriptor =
	  tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_RGB32F, vp->x, vp->y);
	fb_tex = the_tex_create();
	the_tex_set(fb_tex, &descriptor);
	struct the_texture_target color = {
		.tex = fb_tex, .attach = THE_ATTACH_COLOR, .face = THE_FACE_2D, .lod_level = 0
	};

	struct the_texture_desc depthscriptor =
	  tut_texture_desc_default(THE_TEX_2D, THE_TEX_FMT_DEPTH, vp->x, vp->y);
	the_tex fb_depth = the_tex_create();
	the_tex_set(fb_depth, &depthscriptor);
	struct the_texture_target depth = {
		.tex = fb_depth, .attach = THE_ATTACH_DEPTH, .face = THE_FACE_2D, .lod_level = 0
	};

	the_fb_set_target(g_fb, 0, color);
	the_fb_set_target(g_fb, 1, depth);

	struct pbr_desc_unit pbr;
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
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
		int eidx = thepool_theent_add(&entity_pool);
		struct the_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = the_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		the_tex *t = the_mat_tex(e->mat);
		t[0] = g_tex.foam.a;
		t[1] = g_tex.foam.m;
		t[2] = g_tex.foam.r;
		t[3] = g_tex.foam.n;
	}

	*the_shader_tex(g_shaders.skybox) = g_tex.sky;
	*the_shader_tex(g_shaders.fullscreen_img) = fb_tex;
}

void
BuildFrame(struct thearr_thedraw **new_frame, float delta_time)
{
	the_io_poll();
	struct the_point *vp = &the_io->window_size;
	the_camera_control(&camera, (struct the_control_config){ 10.0f, 0.001f, 1.0f, delta_time });

	/* PBR common shader data. */
	struct pbr_desc_scene *common_pbr = the_shader_data(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	common_pbr->camera_position = the_camera_eye(&camera);
	struct the_point fb_size = the_tex_size(fb_tex);

	thearr_thedraw_push_value(new_frame, tut_draw_default());
	struct the_draw *dl = &(*new_frame)->at[(*new_frame)->count - 1];
	dl->state.target.bgcolor = (struct the_color){ 0.2f, 0.2f, 0.2f, 1.0f };
	dl->state.target.fb = g_fb;
	dl->state.pipeline.shader_mat = the_mat_copy_shader(g_shaders.pbr);
	dl->state.ops.viewport = (struct the_rect){ 0, 0, fb_size.x, fb_size.y };
	dl->state.ops.enable |= (1 << THE_DRAW_CLEAR_COLOR);
	dl->state.ops.enable |= (1 << THE_DRAW_CLEAR_DEPTH);
	dl->state.ops.enable |= (1 << THE_DRAW_BLEND);
	dl->state.ops.enable |= (1 << THE_DRAW_TEST_DEPTH);
	dl->state.ops.enable |= (1 << THE_DRAW_WRITE_DEPTH);
	dl->state.ops.blend_src = THE_BLEND_ONE;
	dl->state.ops.blend_dst = THE_BLEND_ZERO;
	dl->state.ops.depth_fun = THE_DEPTH_LESS;
	dl->state.ops.cull_face = THE_CULL_BACK;

	for (int i = 0; i < entity_pool.count; ++i) {
		struct the_draw_cmd *cmd = thearr_thedrawcmd_push(&dl->cmds);
		mat4_assign(entity_pool.buf->at[i].mat.ptr, entity_pool.buf->at[i].transform);
		cmd->material = the_mat_copy(entity_pool.buf->at[i].mat);
		cmd->mesh = entity_pool.buf->at[i].mesh;
	}

	thearr_thedraw_push_value(new_frame, tut_draw_default());
	dl = &(*new_frame)->at[(*new_frame)->count - 1];
	the_camera_static_vp(&camera, the_shader_data(g_shaders.skybox));
	dl->state.pipeline.shader_mat = the_mat_copy_shader(g_shaders.skybox);

	dl->state.ops.disable |= (1 << THE_DRAW_CULL);
	dl->state.ops.depth_fun = THE_DEPTH_LEQUAL;

	struct the_draw_cmd *cmd = thearr_thedrawcmd_push(&dl->cmds);
	cmd->material.shader = g_shaders.skybox;
	cmd->mesh = THE_UTILS_CUBE;

	thearr_thedraw_push_value(new_frame, tut_draw_default());
	dl = &(*new_frame)->at[(*new_frame)->count - 1];
	dl->state.target.bgcolor = (struct the_color){ 1.0f, 0.0f, 0.0f, 1.0f };
	dl->state.target.fb = THE_DEFAULT;
	dl->state.pipeline.shader_mat = the_mat_copy_shader(g_shaders.fullscreen_img);
	dl->state.ops.viewport = (struct the_rect){ 0, 0, vp->x, vp->y };
	dl->state.ops.disable |= (1 << THE_DRAW_TEST_DEPTH);

	cmd = thearr_thedrawcmd_push(&dl->cmds);
	cmd->material.shader = g_shaders.fullscreen_img;
	cmd->mesh = THE_UTILS_QUAD;
}

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	void *mem_chunk = malloc(THE_MB(256));
	the_mem_init(mem_chunk, THE_MB(256));
	the_falloc_set_buffer(the_palloc(THE_MB(16)), THE_MB(16));
	the_io_init("THE PBR Material Demo", (struct the_point){ 1920, 1080 });
	the_camera_init_default(&camera);
	nuklear_init();
	Init();
	the_chrono frame_chrono = the_time();
	while (!the_io->window_closed) {
		float delta_time = the_time_sec(the_elapsed(frame_chrono));
		frame_chrono = the_time();

		struct thearr_thedraw *frame = NULL;
		BuildFrame(&frame, delta_time);

		// Render
		for (int i = 0; i < frame->count; ++i) {
			the_draw(&frame->at[i]);
		}
		nuklear_draw();
		the_window_swap();
		// End render
	}

	return 0;
}
