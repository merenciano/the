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
	struct pbr_tex_t *tex;
	const char *apath;
	const char *npath;
	const char *rpath;
	const char *mpath;
};

static void load_pbr_map(void *pbr_maps)
{
	struct pbr_tex_load_info_t *maps = pbr_maps;
	nyas_tex_load(maps->tex->a, &maps->desc->albedo, maps->apath);
	nyas_tex_load(maps->tex->n, &maps->desc->normal, maps->npath);
	nyas_tex_load(maps->tex->r, &maps->desc->pbr_map, maps->rpath);
	nyas_tex_load(maps->tex->m, &maps->desc->pbr_map, maps->mpath);
}

static void load_textures(void)
{
	g_tex_desc.albedo = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_SRGB, 0, 0);
	g_tex_desc.normal = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_RGB8, 0, 0);
	g_tex_desc.pbr_map = nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_R8, 0, 0);

	struct pbr_tex_load_info_t img_files[] = {
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.gold,
	  .apath = "assets/tex/celtic-gold/celtic-gold_A.png",
	  .npath = "assets/tex/celtic-gold/celtic-gold_N.png",
	  .rpath = "assets/tex/celtic-gold/celtic-gold_R.png",
	  .mpath = "assets/tex/celtic-gold/celtic-gold_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.peeled,
	  .apath = "assets/tex/peeling/peeling_A.png",
	  .npath = "assets/tex/peeling/peeling_N.png",
	  .rpath = "assets/tex/peeling/peeling_R.png",
	  .mpath = "assets/tex/peeling/peeling_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.rusted,
	  .apath = "assets/tex/rusted/rusted_A.png",
	  .npath = "assets/tex/rusted/rusted_N.png",
	  .rpath = "assets/tex/rusted/rusted_R.png",
	  .mpath = "assets/tex/rusted/rusted_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.tiles,
	  .apath = "assets/tex/tiles/tiles_A.png",
	  .npath = "assets/tex/tiles/tiles_N.png",
	  .rpath = "assets/tex/tiles/tiles_R.png",
	  .mpath = "assets/tex/tiles/tiles_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.plastic,
	  .apath = "assets/tex/ship-panels/ship-panels_A.png",
	  .npath = "assets/tex/ship-panels/ship-panels_N.png",
	  .rpath = "assets/tex/ship-panels/ship-panels_R.png",
	  .mpath = "assets/tex/ship-panels/ship-panels_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.shore,
	  .apath = "assets/tex/shore/shore_A.png",
	  .npath = "assets/tex/shore/shore_N.png",
	  .rpath = "assets/tex/shore/shore_R.png",
	  .mpath = "assets/tex/shore/shore_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.cliff,
	  .apath = "assets/tex/cliff/cliff_A.png",
	  .npath = "assets/tex/cliff/cliff_N.png",
	  .rpath = "assets/tex/cliff/cliff_R.png",
	  .mpath = "assets/tex/cliff/cliff_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.granite,
	  .apath = "assets/tex/granite/granite_A.png",
	  .npath = "assets/tex/granite/granite_N.png",
	  .rpath = "assets/tex/granite/granite_R.png",
	  .mpath = "assets/tex/granite/granite_M.png",
	},
	{
	  .desc = &g_tex_desc,
	  .tex = &g_tex.foam,
	  .apath = "assets/tex/foam/foam_A.png",
	  .npath = "assets/tex/foam/foam_N.png",
	  .rpath = "assets/tex/foam/foam_R.png",
	  .mpath = "assets/tex/foam/foam_M.png",
	}};

	for (int i = 0; i < 9; ++i) {
		*img_files[i].tex = (struct pbr_tex_t){ .a = nyas_tex_create(1), .n = nyas_tex_create(1), .r = nyas_tex_create(1), .m = nyas_tex_create(1)};
	}

	for (int i = 0; i < 9; ++i) {
		//*img_files[i].tex = (struct pbr_tex_t){ .a = nyas_tex_create(1), .n = nyas_tex_create(1), .r = nyas_tex_create(1), .m = nyas_tex_create(1)};
		struct nyas_job job = { .job = load_pbr_map, .args = img_files + i};
		nyas_sched_do(job);
	}
	nyas_sched_wait();
}

struct mesh_args {
	const char *path;
	nyas_mesh *out_mesh;
};

static void load_mesh(void *args)
{
	struct mesh_args *ma = args;
	*ma->out_mesh = nyas_mesh_load_file(ma->path);
}

struct env_args {
	const char *path;
	nyas_tex *out_sky;
	nyas_tex *out_irr;
	nyas_tex *out_pref;
	nyas_tex *out_lut;
};

static void load_env(void *args)
{
	struct env_args *ea = args;
	nyas_load_env(ea->path, ea->out_lut, ea->out_sky, ea->out_irr, ea->out_pref);
}

static void load_assets(void)
{
	struct mesh_args mshargs = { .path = "assets/obj/matball.msh", .out_mesh = &g_mesh };
	struct env_args envargs = {.path = "assets/env/canyon.env", .out_sky = &g_tex.sky, .out_irr = &g_tex.irradiance, .out_pref = &g_tex.prefilter, .out_lut = &g_tex.lut};
	{
		struct nyas_job job = { .job = load_mesh, .args = &mshargs};
		nyas_sched_do(job);
	}
	{
		struct nyas_job job = { .job = load_env, .args = &envargs};
		nyas_sched_do(job);
	}
	load_textures();

	nyas_sched_wait();
}

void
Init(void)
{
	g_fb = nyas_fb_create();

	g_shaders.fullscreen_img = nyas_shader_create(g_shader_descriptors.fullscreen_img);
	g_shaders.skybox = nyas_shader_create(g_shader_descriptors.sky);
	g_shaders.pbr = nyas_shader_create(g_shader_descriptors.pbr);

	load_assets();

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

	entities = nyas_arr_create(struct nyas_entity, 16);
	// CelticGold
	{
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		position[0] = 0.0f;
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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
		struct nyas_entity *e = nyas_arr_push(entities);
		mat4_identity(e->transform);
		mat4_translation(e->transform, e->transform, position);
		e->mesh = g_mesh;
		e->mat = nyas_mat_create(g_shaders.pbr);
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

	struct nyas_point *vp = &nyas_io->window_size;
	struct nyas_texture_desc descriptor =
	  nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_RGB32F, vp->x, vp->y);
	fb_tex = nyas_tex_create(1);
	nyas_tex_set(fb_tex, &descriptor);
	struct nyas_texture_target color = {
		.tex = fb_tex,
		.attach = NYAS_ATTACH_COLOR,
		.face = NYAS_FACE_2D,
		.lod_level = 0
	};

	struct nyas_texture_desc depthscriptor =
	  nyas_tex_defined_desc(NYAS_TEX_2D, NYAS_TEX_FMT_DEPTH, vp->x, vp->y);
	nyas_tex fb_depth = nyas_tex_create(1);
	nyas_tex_set(fb_depth, &depthscriptor);
	struct nyas_texture_target depth = {
	  .tex = fb_depth,
	  .attach = NYAS_ATTACH_DEPTH,
	  .face = NYAS_FACE_2D,
	  .lod_level = 0
	};

	nyas_fb_set_target(g_fb, 0, color);
	nyas_fb_set_target(g_fb, 1, depth);
	*nyas_shader_tex(g_shaders.fullscreen_img) = fb_tex;
}

static void *renderalloc(void *ptr, ptrdiff_t size, void *_2)
{
	(void)_2;
	void *mem = nyas_frame_alloc(size);
	if (ptr) {
		memcpy(mem, ptr, size / 2);
	}
	return mem;
}

void BuildFrame(struct nyas_frame_ctx *new_frame, float delta_time)
{
	nyas_io_poll();
	struct nyas_point *vp = &nyas_io->window_size;
	nyas_camera_control(&camera, (struct nyas_control_config){ 10.0f, 0.001f, 1.0f, delta_time });

	/* PBR common shader data. */
	struct nyas_pbr_desc_scene *common_pbr = nyas_shader_data(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	common_pbr->camera_position = nyas_camera_eye(&camera);
	struct nyas_point fb_size = nyas_tex_size(fb_tex);

	new_frame->draw_lists = nyas_arr_create_a(struct nyas_drawlist, 8, renderalloc, NULL);
	struct nyas_drawlist *dl = nyas_arr_push(new_frame->draw_lists);
	nyas_draw_state_default(&dl->state);

	dl->state.target.bgcolor = (struct nyas_color){0.2f, 0.2f, 0.2f, 1.0f};
	dl->state.target.fb = g_fb;
	dl->state.ops.viewport = (struct nyas_rect){ 0, 0, fb_size.x, fb_size.y };

	dl->state.pipeline.shader = g_shaders.pbr;
	dl->state.pipeline.shared_data = nyas_mat_copy_shader(g_shaders.pbr);

	nyas_draw_op_enable(&dl->state.ops, NYAS_DRAW_CLEAR_COLOR);
	nyas_draw_op_enable(&dl->state.ops, NYAS_DRAW_CLEAR_DEPTH);
	nyas_draw_op_enable(&dl->state.ops, NYAS_DRAW_BLEND);
	nyas_draw_op_enable(&dl->state.ops, NYAS_DRAW_TEST_DEPTH);
	nyas_draw_op_enable(&dl->state.ops, NYAS_DRAW_WRITE_DEPTH);
	dl->state.ops.blend_src = NYAS_BLEND_ONE;
	dl->state.ops.blend_dst = NYAS_BLEND_ZERO;
	dl->state.ops.depth_fun = NYAS_DEPTH_LESS;
	dl->state.ops.cull_face = NYAS_CULL_BACK;

	dl->cmds = nyas_arr_create_a(struct nyas_draw_cmd, 16, renderalloc, NULL);
	for (int i = 0; i < nyas_arr_count(entities); ++i) {
		struct nyas_draw_cmd *cmd = nyas_arr_push(dl->cmds);
		mat4_assign(entities[i].mat.ptr, entities[i].transform);
		cmd->material = nyas_mat_copy(entities[i].mat);
		cmd->mesh = entities[i].mesh;
	}

	dl = nyas_arr_push(new_frame->draw_lists);
	nyas_draw_state_default(&dl->state);

	nyas_camera_static_vp(&camera, nyas_shader_data(g_shaders.skybox));
	dl->state.pipeline.shader = g_shaders.skybox;
	dl->state.pipeline.shared_data = nyas_mat_copy_shader(g_shaders.skybox);

	nyas_draw_op_disable(&dl->state.ops, NYAS_DRAW_CULL);
	dl->state.ops.depth_fun = NYAS_DEPTH_LEQUAL;

	dl->cmds = nyas_arr_create_a(struct nyas_draw_cmd, 2, renderalloc, NULL);
	struct nyas_draw_cmd *cmd = nyas_arr_push(dl->cmds);
	cmd->material.shader = g_shaders.skybox;
	cmd->mesh = CUBE_MESH;

	dl = nyas_arr_push(new_frame->draw_lists);
	nyas_draw_state_default(&dl->state);

	dl->state.target.bgcolor = (struct nyas_color){1.0f, 0.0f, 0.0f, 1.0f};
	dl->state.target.fb = NYAS_DEFAULT;
	dl->state.pipeline.shader = g_shaders.fullscreen_img;
	dl->state.pipeline.shared_data = nyas_mat_copy_shader(g_shaders.fullscreen_img);
	dl->state.ops.viewport = (struct nyas_rect){ 0, 0, vp->x, vp->y };
	nyas_draw_op_disable(&dl->state.ops, NYAS_DRAW_TEST_DEPTH);

	dl->cmds = nyas_arr_create_a(struct nyas_draw_cmd, 2, renderalloc, NULL);
	cmd = nyas_arr_push(dl->cmds);
	cmd->material.shader = g_shaders.fullscreen_img;
	cmd->mesh = QUAD_MESH;
}

int
main(int argc, char **argv)
{
	(void)argc, (void)argv;
	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS PBR Material Demo", (struct nyas_point){1920, 1080});
	nyas_io_poll();
	nyas_sched_init(9, 32);
	nyas_px_init();
	nyas_camera_init_default(&camera);
	nuklear_init();
	Init();
	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_io->window_closed) {
		float delta_time = nyas_time_sec(nyas_elapsed(frame_chrono));
		frame_chrono = nyas_time();

		struct nyas_frame_ctx frame;
		BuildFrame(&frame, delta_time);

		// Render
		nyas_frame_render(&frame);
		nuklear_draw();
		nyas_window_swap();
		// End render
	}

	return 0;
}
