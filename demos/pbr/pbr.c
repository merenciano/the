#include "nyas.h"
#include <mathc.h>
#include <string.h>

struct Materials {
	nyas_shader fullscreen_img;
	nyas_shader skybox;
	nyas_shader eqr_to_cube;
	nyas_shader prefilter_env;
	nyas_shader lut_gen;
	nyas_shader pbr;
};

struct Materials g_mats;
static nyas_resourcemap g_resources;
nyas_framebuffer g_fb;

nyas_mat pbr_common;
nyas_mat sky_common;
nyas_mat fulls;

static float g_sunlight[4] = { 0.0f, -1.0f, -0.1f, 1.0f };

static nyas_mat
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

	nyas_mat mat_tocube = { .shader = g_mats.eqr_to_cube,
		                    .data_count = 16 * 6,
		                    .tex_count = 0,
		                    .cube_count = 0 };

	float *vp = nyas_mat_alloc_frame(&mat_tocube);
	for (int i = 0; i < 6; ++i) {
		float *view = (float *)&views[i];
		mat4_multiply(vp + 16 * i, proj, view);
	}

	return mat_tocube;
}

static nyas_cmd *
ConvertEqrToCubeCommandList(nyas_tex tex_in,
                            nyas_tex tex_out,
                            nyas_framebuffer fb,
                            nyas_cmd *last_command)
{
	nyas_cmd *use_tocube = nyas_cmd_alloc();
	use_tocube->data.mat.shader = g_mats.eqr_to_cube;
	use_tocube->data.mat.data_count = 0;
	use_tocube->data.mat.tex_count = 1;
	use_tocube->data.mat.cube_count = 0;
	nyas_tex *eqr = nyas_mat_alloc_frame(&use_tocube->data.mat);
	*eqr = tex_in;
	use_tocube->execute = nyas_setshader_fn;
	last_command->next = use_tocube;

	nyas_fbattach atta = { .slot = NYAS_ATTACH_COLOR,
		                   .tex = tex_out,
		                   .level = 0 };

	nyas_cmd *last = use_tocube;
	nyas_mat draw_tocube = GenerateRenderToCubeMat();
	for (int i = 0; i < 6; ++i) {
		nyas_cmd *cube_fb = nyas_cmd_alloc();
		cube_fb->data.set_fb.fb = fb;
		cube_fb->data.set_fb.vp_x = NYAS_IGNORE;
		cube_fb->data.set_fb.attachment = atta;
		cube_fb->data.set_fb.attachment.side = i;
		cube_fb->execute = nyas_setfb_fn;
		last->next = cube_fb;

		nyas_cmd *draw_cube = nyas_cmd_alloc();
		draw_cube->data.draw.mesh = CUBE_MESH;
		draw_cube->data.draw.material = draw_tocube;
		draw_cube->data.draw.material.data_count = 16;
		draw_cube->data.draw.material.ptr = (float *)draw_tocube.ptr + 16 * i;
		draw_cube->execute = nyas_draw_fn;
		cube_fb->next = draw_cube;
		last = draw_cube;
	}

	return last;
}

static nyas_cmd *
GeneratePrefilterCommandList(nyas_tex tex_in,
                             nyas_tex tex_out,
                             nyas_framebuffer fb,
                             nyas_cmd *last_command)
{
	nyas_cmd *use_pref = nyas_cmd_alloc();
	use_pref->data.mat.shader = g_mats.prefilter_env;
	use_pref->data.mat.data_count = 0;
	use_pref->data.mat.tex_count = 0;
	use_pref->data.mat.cube_count = 1;
	nyas_tex *sky_tex = nyas_mat_alloc_frame(&use_pref->data.mat);
	*sky_tex = tex_in;
	use_pref->execute = nyas_setshader_fn;
	last_command->next = use_pref;
	last_command = use_pref;

	nyas_mat mat = GenerateRenderToCubeMat();
	int tex_size[2];
	float tex_width = (float)nyas_tex_size(tex_out, tex_size)[0];
	for (int i = 0; i < 5; ++i) {
		int16_t vp_size = (int16_t)(tex_width * powf(0.5f, (float)i));
		float roughness = (float)i / 4.0f;

		for (int side = 0; side < 6; ++side) {
			nyas_cmd *fb_comm = nyas_cmd_alloc();
			fb_comm->data.set_fb.fb = fb;
			fb_comm->data.set_fb.vp_x = vp_size;
			fb_comm->data.set_fb.vp_y = vp_size;
			fb_comm->data.set_fb.attachment.tex = tex_out;
			fb_comm->data.set_fb.attachment.slot = NYAS_ATTACH_COLOR;
			fb_comm->data.set_fb.attachment.side = side;
			fb_comm->data.set_fb.attachment.level = i;
			fb_comm->execute = nyas_setfb_fn;
			last_command->next = fb_comm;

			nyas_cmd *clear_comm = nyas_cmd_alloc();
			clear_comm->data.clear.color_buffer = true;
			clear_comm->data.clear.depth_buffer = false;
			clear_comm->data.clear.stencil_buffer = false;
			clear_comm->execute = nyas_clear_fn;
			fb_comm->next = clear_comm;

			nyas_cmd *draw_comm = nyas_cmd_alloc();
			draw_comm->data.draw.mesh = CUBE_MESH;
			draw_comm->data.draw.material.shader = g_mats.prefilter_env;
			draw_comm->data.draw.material.data_count = sizeof(struct mat4) /
			  sizeof(float);
			draw_comm->data.draw.material.tex_count = 0;
			draw_comm->data.draw.material.cube_count = 0;
			float *vp = nyas_mat_alloc_frame(&draw_comm->data.draw.material);
			mat4_assign(vp, (float *)mat.ptr + 16 * side);
			vp[16] = roughness;
			draw_comm->execute = nyas_draw_fn;
			clear_comm->next = draw_comm;
			draw_comm->next = NULL;
			last_command = draw_comm;
		}
	}

	return last_command;
}

static nyas_cmd *
GenerateLutCommandlist(nyas_framebuffer fb, nyas_cmd *last_command)
{
	nyas_tex lut_tex = nyas_resourcemap_tex(&g_resources, "LutMap");
	nyas_cmd *set_init_fb = nyas_cmd_alloc();
	set_init_fb->data.set_fb.fb = fb;
	set_init_fb->data.set_fb.vp_x = NYAS_IGNORE;
	set_init_fb->data.set_fb.attachment.slot = NYAS_ATTACH_COLOR;
	set_init_fb->data.set_fb.attachment.tex = lut_tex;
	set_init_fb->data.set_fb.attachment.side = -1;
	set_init_fb->data.set_fb.attachment.level = 0;
	set_init_fb->execute = nyas_setfb_fn;
	last_command->next = set_init_fb;

	nyas_cmd *clear_comm = nyas_cmd_alloc();
	clear_comm->data.clear.color_buffer = true;
	clear_comm->data.clear.depth_buffer = false;
	clear_comm->data.clear.stencil_buffer = false;
	clear_comm->execute = nyas_clear_fn;
	set_init_fb->next = clear_comm;

	nyas_mat lutmat = nyas_mat_default();
	lutmat.shader = g_mats.lut_gen;

	nyas_cmd *set_lut = nyas_cmd_alloc();
	set_lut->data.mat = lutmat;
	set_lut->execute = nyas_setshader_fn;
	clear_comm->next = set_lut;

	nyas_cmd *draw_lut = nyas_cmd_alloc();
	draw_lut->data.draw.mesh = QUAD_MESH;
	draw_lut->data.draw.material = lutmat;
	draw_lut->execute = nyas_draw_fn;
	set_lut->next = draw_lut;
	draw_lut->next = NULL;

	return draw_lut;
}

static void
GeneratePbrEnv(void)
{
	nyas_resourcemap *rm = &g_resources;
	nyas_framebuffer auxfb = nyas_fb_create(1, 1, false, false);

	nyas_cmd *rendops = nyas_cmd_alloc();
	rendops->data.rend_opts.enable_flags = NYAS_DEPTH_TEST | NYAS_DEPTH_WRITE |
	  NYAS_BLEND;
	rendops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rendops->data.rend_opts.cull_face = NYAS_CULL_FACE_BACK;
	rendops->data.rend_opts.blend_func.src = NYAS_BLEND_FUNC_ONE;
	rendops->data.rend_opts.blend_func.dst = NYAS_BLEND_FUNC_ZERO;
	rendops->execute = nyas_rops_fn;

	nyas_tex eqr_in = nyas_tex_load_img("assets/tex/env/helipad-env.hdr",
	                                    NYAS_TEX_RGB_F16);
	nyas_tex eqr_cube_out = nyas_resourcemap_tex(rm, "Skybox");

	nyas_cmd *last_tocube = ConvertEqrToCubeCommandList(eqr_in, eqr_cube_out,
	                                                    auxfb, rendops);

	nyas_tex irrad_in = nyas_tex_load_img("assets/tex/env/helipad-dif.hdr",
	                                      NYAS_TEX_RGB_F16);
	nyas_tex irrad_out = nyas_resourcemap_tex(rm, "Irradian");

	last_tocube = ConvertEqrToCubeCommandList(irrad_in, irrad_out, auxfb,
	                                          last_tocube);

	nyas_tex pref_out = nyas_resourcemap_tex(rm, "Prefilte");
	nyas_cmd *pref = GeneratePrefilterCommandList(eqr_cube_out, pref_out,
	                                              auxfb, last_tocube);

	GenerateLutCommandlist(auxfb, pref);
	nyas_cmd_add(rendops);
}

void
Init(void)
{
	g_fb = nyas_fb_create(nyas_window_width(), nyas_window_height(), true,
	                      true);
	g_resources.meshes = nyas_hmap_create(8, sizeof(nyas_mesh));
	g_resources.textures = nyas_hmap_create(64, sizeof(nyas_tex));

	g_mats.fullscreen_img = nyas_shader_create("fullscreen-img");
	g_mats.skybox = nyas_shader_create("skybox");
	g_mats.eqr_to_cube = nyas_shader_create("eqr-to-cube");
	g_mats.prefilter_env = nyas_shader_create("prefilter-env");
	g_mats.lut_gen = nyas_shader_create("lut-gen");
	g_mats.pbr = nyas_shader_create("pbr");

	fulls.shader = g_mats.fullscreen_img;
	fulls.data_count = 0;
	fulls.tex_count = 1;
	fulls.cube_count = 0;
	nyas_tex *fst = nyas_mat_alloc(&fulls);
	*fst = nyas_fb_color(g_fb);

	nyas_resourcemap *rm = &g_resources;

	nyas_resourcemap_mesh_file(rm, "MatBall", "assets/obj/matball-n.obj");
	nyas_resourcemap_tex_add(rm, "Skybox", 1024, 1024, NYAS_TEX_ENVIRONMENT);
	nyas_resourcemap_tex_add(rm, "Irradian", 1024, 1024, NYAS_TEX_ENVIRONMENT);
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

	// CelticGold
	{
		nyas_entity *e = nyas_entity_create();
		mat4_translation(e->transform, e->transform, position);
		e->mesh = nyas_resourcemap_mesh(rm, "MatBall");
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
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
		e->mat.data_count = sizeof(struct nyas_pbr_desc_unit) / 4;
		e->mat.tex_count = 4;
		e->mat.cube_count = 0;
		e->mat.shader = g_mats.pbr;
		struct nyas_pbr_desc_unit *d = nyas_mat_alloc(&e->mat);
		*d = pbr;
		nyas_tex *t = (nyas_tex *)d + e->mat.data_count;
		t[0] = nyas_resourcemap_tex(rm, "Foam_A");
		t[1] = nyas_resourcemap_tex(rm, "Foam_M");
		t[2] = nyas_resourcemap_tex(rm, "Foam_R");
		t[3] = nyas_resourcemap_tex(rm, "Foam_N");
	}

	GeneratePbrEnv();

	pbr_common.data_count = sizeof(struct nyas_pbr_desc_scene) / sizeof(float);
	pbr_common.tex_count = 1;
	pbr_common.cube_count = 2;
	pbr_common.shader = g_mats.pbr;
	nyas_tex *pbr_scene_tex = nyas_mat_alloc(&pbr_common);
	pbr_scene_tex += pbr_common.data_count;
	pbr_scene_tex[0] = nyas_resourcemap_tex(rm, "LutMap");
	pbr_scene_tex[1] = nyas_resourcemap_tex(rm, "Irradian");
	pbr_scene_tex[2] = nyas_resourcemap_tex(rm, "Prefilte");

	sky_common.data_count = 16;
	sky_common.tex_count = 0;
	sky_common.cube_count = 1;
	sky_common.shader = g_mats.skybox;
	nyas_tex *skytex = nyas_mat_alloc(&sky_common);
	skytex[sky_common.data_count] = nyas_resourcemap_tex(rm, "Skybox");
}

void
Update(nyas_chrono chrono)
{
	float dt = nyas_time_sec(nyas_elapsed(chrono));
	chrono = nyas_time();

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
	clear->data.clear.color[0] = 0.2f;
	clear->data.clear.color[1] = 0.2f;
	clear->data.clear.color[2] = 0.2f;
	clear->data.clear.color[3] = 1.0f;
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
	clear->data.clear.color[0] = 1.0f;
	clear->data.clear.color[1] = 0.0f;
	clear->data.clear.color[2] = 0.0f;
	clear->data.clear.color[3] = 1.0f;
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

void Render(void)
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
	nyas_io_init("NYAS Material", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);
	nyas_imgui_init();
	Init();

	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_window_closed()) {
		Update(frame_chrono);
		Render();
	}

	return 0;
}
