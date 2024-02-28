#include "core/utils.h"
#include "gui.h"
#include "nyas.h"
#include "pbr.h"
#include <mathc.h>
#include <string.h>

typedef struct nyas_draw nydraw;
NYAS_DECL_ARR(nydraw);
NYAS_IMPL_ARR_MA(nydraw, nyas_falloc);

struct {
	nyas_shader fullscreen_img;
	nyas_shader skybox;
	nyas_shader pbr;
} g_shaders;

struct {
	nyas_tex sky;

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

nyas_framebuffer g_fb;
nyas_tex fb_tex;
nyas_mesh g_mesh;

void
Init(void)
{
	nyut_mesh_init_geometry();
	nyas_tex irradiance;
	nyas_tex prefilter;
	nyas_tex lut;

	// TODO: create repe, porque peta sino?
	g_shaders.fullscreen_img = nyas_shader_create(&g_shader_descriptors.fullscreen_img);
	g_shaders.skybox = nyas_shader_create(&g_shader_descriptors.sky);
	g_shaders.pbr = nyas_shader_create(&g_shader_descriptors.pbr);

	struct nyut_shad_ldargs fsimgargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct nyut_shad_ldargs skyargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct nyut_shad_ldargs pbrargs = {
		.desc = g_shader_descriptors.fullscreen_img,
		.shader = &g_shaders.fullscreen_img
	};

	struct nyut_mesh_ldargs mesh = {
		.path = "assets/obj/matball.msh",
		.mesh = &g_mesh
	};

	struct nyut_env_ldargs envargs = {
		.path = "assets/env/canyon.env",
		.sky = &g_tex.sky,
		.irr = &irradiance,
		.pref = &prefilter,
		.lut = &lut
	};

	nyut_assetldr *ldr = nyut_assets_create();
	nyut_assets_add_shader(ldr, &fsimgargs);
	nyut_assets_add_shader(ldr, &skyargs);
	nyut_assets_add_shader(ldr, &pbrargs);
	nyut_assets_add_mesh(ldr, &mesh);
	nyut_assets_add_env(ldr, &envargs);

	struct nyas_texture_desc texdesc[] = {
		nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_SRGB, 0, 0),
		nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_RGB8, 0, 0),
		nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_R8, 0, 0),
		nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_R8, 0, 0)
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

	struct nyut_tex_ldargs loadtexargs[9 * 4];
	nyas_tex *tex_begin = &g_tex.gold.a;
	for (int i = 0; i < 9 * 4; ++i) {
		*tex_begin = nyas_tex_create();
		loadtexargs[i].tex = *tex_begin++;
		loadtexargs[i].desc = texdesc[i & 3];
		loadtexargs[i].path = texpaths[i];
	}

	for (int i = 0; i < 9 * 4; ++i) {
		nyut_assets_add_tex(ldr, &loadtexargs[i]);
	}

	nyut_assets_load(ldr, 8);

	struct pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	common_pbr->sunlight[0] = 0.0f;
	common_pbr->sunlight[1] = -1.0f;
	common_pbr->sunlight[2] = -0.1f;
	common_pbr->sunlight[3] = 0.0f;

	nyas_tex *pbr_scene_tex = nyas_shader_tex(g_shaders.pbr);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irradiance;
	pbr_scene_tex[2] = prefilter;

	g_fb = nyas_fb_create();
	struct nyas_point *vp = &nyas_io->window_size;
	struct nyas_texture_desc descriptor =
	  nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_RGB32F, vp->x, vp->y);
	fb_tex = nyas_tex_create();
	nyas_tex_set(fb_tex, &descriptor);
	struct nyas_texture_target color = {
		.tex = fb_tex, .attach = NYAS_ATTACH_COLOR, .face = NYAS_FACE_2D, .lod_level = 0
	};

	struct nyas_texture_desc depthscriptor =
	  nyut_texture_desc_default(NYAS_TEX_2D, NYAS_TEX_FMT_DEPTH, vp->x, vp->y);
	nyas_tex fb_depth = nyas_tex_create();
	nyas_tex_set(fb_depth, &depthscriptor);
	struct nyas_texture_target depth = {
		.tex = fb_depth, .attach = NYAS_ATTACH_DEPTH, .face = NYAS_FACE_2D, .lod_level = 0
	};

	nyas_fb_set_target(g_fb, 0, color);
	nyas_fb_set_target(g_fb, 1, depth);

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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
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
		int eidx = nypool_nyent_add(&entity_pool);
		struct nyas_entity *e = &entity_pool.buf->at[eidx];
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->mat.ptr = pbr;
		nyas_tex *t = nyas_mat_tex(e->mat);
		t[0] = g_tex.foam.a;
		t[1] = g_tex.foam.m;
		t[2] = g_tex.foam.r;
		t[3] = g_tex.foam.n;
	}

	*nyas_shader_tex(g_shaders.skybox) = g_tex.sky;
	*nyas_shader_tex(g_shaders.fullscreen_img) = fb_tex;
}

void
BuildFrame(struct nyarr_nydraw **new_frame, float delta_time)
{
	nyas_io_poll();
	struct nyas_point *vp = &nyas_io->window_size;
	nyas_camera_control(&camera, (struct nyas_control_config){ 10.0f, 0.001f, 1.0f, delta_time });

	/* PBR common shader data. */
	struct pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	common_pbr->camera_position = nyas_camera_eye(&camera);
	struct nyas_point fb_size = nyas_tex_size(fb_tex);

	nyarr_nydraw_push_value(new_frame, nyut_draw_default());
	struct nyas_draw *dl = &(*new_frame)->at[(*new_frame)->count - 1];
	dl->state.target.bgcolor = (struct nyas_color){ 0.2f, 0.2f, 0.2f, 1.0f };
	dl->state.target.fb = g_fb;
	dl->state.pipeline.shader_mat = nyas_mat_copy_shader(g_shaders.pbr);
	dl->state.ops.viewport = (struct nyas_rect){ 0, 0, fb_size.x, fb_size.y };
	dl->state.ops.enable |= (1 << NYAS_DRAW_CLEAR_COLOR);
	dl->state.ops.enable |= (1 << NYAS_DRAW_CLEAR_DEPTH);
	dl->state.ops.enable |= (1 << NYAS_DRAW_BLEND);
	dl->state.ops.enable |= (1 << NYAS_DRAW_TEST_DEPTH);
	dl->state.ops.enable |= (1 << NYAS_DRAW_WRITE_DEPTH);
	dl->state.ops.blend_src = NYAS_BLEND_ONE;
	dl->state.ops.blend_dst = NYAS_BLEND_ZERO;
	dl->state.ops.depth_fun = NYAS_DEPTH_LESS;
	dl->state.ops.cull_face = NYAS_CULL_BACK;

	for (int i = 0; i < entity_pool.count; ++i) {
		struct nyas_draw_cmd *cmd = nyarr_nydrawcmd_push(&dl->cmds);
		mat4_assign(entity_pool.buf->at[i].mat.ptr, entity_pool.buf->at[i].transform);
		cmd->material = nyas_mat_copy(entity_pool.buf->at[i].mat);
		cmd->mesh = entity_pool.buf->at[i].mesh;
	}

	nyarr_nydraw_push_value(new_frame, nyut_draw_default());
	dl = &(*new_frame)->at[(*new_frame)->count - 1];
	nyas_camera_static_vp(&camera, nyas_shader_data(g_shaders.skybox));
	dl->state.pipeline.shader_mat = nyas_mat_copy_shader(g_shaders.skybox);

	dl->state.ops.disable |= (1 << NYAS_DRAW_CULL);
	dl->state.ops.depth_fun = NYAS_DEPTH_LEQUAL;

	struct nyas_draw_cmd *cmd = nyarr_nydrawcmd_push(&dl->cmds);
	cmd->material.shader = g_shaders.skybox;
	cmd->mesh = NYAS_UTILS_CUBE;

	nyarr_nydraw_push_value(new_frame, nyut_draw_default());
	dl = &(*new_frame)->at[(*new_frame)->count - 1];
	dl->state.target.bgcolor = (struct nyas_color){ 1.0f, 0.0f, 0.0f, 1.0f };
	dl->state.target.fb = NYAS_DEFAULT;
	dl->state.pipeline.shader_mat = nyas_mat_copy_shader(g_shaders.fullscreen_img);
	dl->state.ops.viewport = (struct nyas_rect){ 0, 0, vp->x, vp->y };
	dl->state.ops.disable |= (1 << NYAS_DRAW_TEST_DEPTH);

	cmd = nyarr_nydrawcmd_push(&dl->cmds);
	cmd->material.shader = g_shaders.fullscreen_img;
	cmd->mesh = NYAS_UTILS_QUAD;
}

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	void *mem_chunk = malloc(NYAS_MB(256));
	nyas_mem_init(mem_chunk, NYAS_MB(256));
	nyas_falloc_set_buffer(nyas_palloc(NYAS_MB(16)), NYAS_MB(16));
	nyas_io_init("NYAS PBR Material Demo", (struct nyas_point){ 1920, 1080 });
	nyas_camera_init_default(&camera);
	nuklear_init();
	Init();
	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_io->window_closed) {
		float delta_time = nyas_time_sec(nyas_elapsed(frame_chrono));
		frame_chrono = nyas_time();

		struct nyarr_nydraw *frame = NULL;
		BuildFrame(&frame, delta_time);

		// Render
		for (int i = 0; i < frame->count; ++i) {
			nyas_draw(&frame->at[i]);
		}
		nuklear_draw();
		nyas_window_swap();
		// End render
	}

	return 0;
}
