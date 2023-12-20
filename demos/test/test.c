#include "nyas.h"
#include "../helpersdemo.h"
#include "mathc.h"
#include <string.h>

typedef struct entt {
	nyas_entity *e;
	nyas_pbr_desc_unit upbr;
	nyas_tex albedo;
	nyas_tex normal;
	nyas_tex roughness;
	nyas_tex metalness;
	nyas_tex env;
	nyas_tex envirr;
	float light[4];
} entt;

entt e;
nyas_framebuffer fb;
nyas_shader shader;
nyas_shader skybox_sh;
nyas_shader fs_sh;
nyas_shader eqr_sh;
nyas_shader lut_sh;
nyas_shader pref_sh;
nyas_tex skybox_tex;

void Init(void)
{
	fb = nyas_fb_create(nyas_window_width(), nyas_window_height(), true,
	                      true);
	e.light[0] = 1.0f;
	e.light[1] = -1.0f;
	e.light[2] = 1.0f;
	e.light[3] = 1.0f;
	nyas_shader_desc pbr_descriptor = {
		.name = "pbr",
		.data_count = 7 * 4, // 7 vec4
		.tex_count = 4,
		.cubemap_count = 0,
		.common_data_count = 6 * 4, // 6 vec4
		.common_tex_count = 1,
		.common_cubemap_count = 2
	};
	shader = nyas_shader_create(&pbr_descriptor);

	nyas_shader_desc sky_descriptor = {
	  .name = "skybox",
	  .data_count = 0,
	  .tex_count = 0,
	  .cubemap_count = 0,
	  .common_data_count = 4 * 4, // mat4
	  .common_tex_count = 0,
	  .common_cubemap_count = 1
	};
	skybox_sh = nyas_shader_create(&sky_descriptor);

	nyas_shader_desc img_descriptor = {
	  .name = "fullscreen-img", // fullscreen quad with texture
	  .data_count = 0,
	  .tex_count = 0,
	  .cubemap_count = 0,
	  .common_data_count = 0,
	  .common_tex_count = 1,
	  .common_cubemap_count = 0
	};
	fs_sh = nyas_shader_create(&img_descriptor);

	nyas_shader_desc eqr_descriptor = {
	  .name = "eqr-to-cube", // environment image to cubemap
	  .data_count = 4 * 4,
	  .tex_count = 0,
	  .cubemap_count = 0,
	  .common_data_count = 0,
	  .common_tex_count = 1,
	  .common_cubemap_count = 0
	};
	eqr_sh = nyas_shader_create(&eqr_descriptor);

	nyas_shader_desc pref_descriptor = {
	  .name = "prefilter-env", // environment prefilter
	  .data_count = 5 * 4,
	  .tex_count = 0,
	  .cubemap_count = 0,
	  .common_data_count = 0,
	  .common_tex_count = 0,
	  .common_cubemap_count = 1
	};
	pref_sh = nyas_shader_create(&pref_descriptor);

	nyas_shader_desc lut_descriptor = {
	  .name = "lut-gen", // look-up table
	  .data_count = 0,
	  .tex_count = 0,
	  .cubemap_count = 0,
	  .common_data_count = 0,
	  .common_tex_count = 0,
	  .common_cubemap_count = 0
	};
	lut_sh = nyas_shader_create(&lut_descriptor);

	int envflags = nyas_tex_flags(3, true, true, false, false, false, false);
	e.env = nyas_tex_load("assets/tex/env/helipad-env.hdr", 1, envflags);
	e.envirr = nyas_tex_load("assets/tex/env/helipad-dif.hdr", 1, envflags);

	int alb = nyas_tex_flags(3, false, false, false, false, true, true);
	e.albedo = nyas_tex_load("../pbr/assets/tex/cliff/cliff_A.png", 1, alb);
	int n = nyas_tex_flags(3, false, true, false, false, true, true);
	e.normal = nyas_tex_load("../pbr/assets/tex/cliff/cliff_N.png", 1, n);
	int r_m = nyas_tex_flags(1, false, true, false, false, true, true);
	e.roughness = nyas_tex_load("../pbr/assets/tex/cliff/cliff_R.png", 1, r_m);
	e.metalness = nyas_tex_load("../pbr/assets/tex/cliff/cliff_M.png", 1, r_m);

	*nyas_shader_tex(fs_sh) = nyas_fb_color(fb);

	int sky_flags = nyas_tex_flags(3, true, true, true, false, false, false);
	skybox_tex = nyas_tex_empty(1024, 1024, sky_flags);
	e.upbr.use_albedo_map = 1.0f;
	e.upbr.normal_map_intensity = 1.0f;
	e.upbr.use_pbr_maps = 1.0f;
	e.upbr.tiling_x = 4.0f;
	e.upbr.tiling_y = 4.0f;
	e.e = nyas_entity_create();
	float position[3] = {0.0f, 0.0f, 0.0f};
	mat4_translation(e.e->transform, e.e->transform, position);
	e.e->mesh = nyas_mesh_create();
	nyas_mesh_load_obj(e.e->mesh, "assets/obj/matball-n.obj");
	e.e->mat = nyas_mat_pers(shader);
	*(nyas_pbr_desc_unit*)e.e->mat.ptr = e.upbr;
	nyas_tex *t = nyas_mat_tex(&e.e->mat);
	t[0] = e.albedo;
	t[1] = e.metalness;
	t[2] = e.roughness;
	t[3] = e.normal;

	int lutflgs = nyas_tex_flags(2, true, true, false, false, false, false);
	nyas_tex lut = nyas_tex_empty(512, 512, lutflgs);
	int irrflgs = nyas_tex_flags(3, true, true, true, false, false, false);
	nyas_tex irr = nyas_tex_empty(1024, 1024, irrflgs);
	int preflgs = nyas_tex_flags(3, true, true, true, false, false, true);
	nyas_tex pref = nyas_tex_empty(128, 128, preflgs);

	nyas_tex *pbr_scene_tex = nyas_shader_tex(shader);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irr;
	pbr_scene_tex[2] = pref;

	*nyas_shader_tex(skybox_sh) = skybox_tex;

	GenEnv env = {
		.eqr_sh = eqr_sh,
		.lut_sh = lut_sh,
		.pref_sh = pref_sh,
		.eqr_in = e.env,
		.eqr_out = skybox_tex,
		.irr_in = e.envirr,
		.irr_out = irr,
		.pref_out = pref,
		.lut = lut
	};

	GeneratePbrEnv(&env);
}

void Update(nyas_chrono *dt)
{
	float delta = nyas_time_sec(nyas_elapsed(*dt));
	*dt = nyas_time();
	nyas_io_poll();
	nyas_input_read();
	nyas_camera_control(&camera, delta); 

	nyas_pbr_desc_scene *common_pbr = nyas_shader_data(shader);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);
	vec4_assign(common_pbr->sunlight, e.light);

	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = fb;
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
	use_pbr->data.mat = nyas_mat_from_shader(shader);
	use_pbr->execute = nyas_setshader_fn;
	clear->next = use_pbr;
	use_pbr->next = NULL;

	nyas_cmd_add(fbuff);

	nyas_entity_draw(e.e, 1);

	rops = nyas_cmd_alloc();
	memset(&rops->data.rend_opts, 0, sizeof(nyas_rops_cmdata));
	rops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LEQUAL;
	rops->execute = nyas_rops_fn;

	nyas_camera_static_vp(nyas_shader_data(skybox_sh), &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = nyas_mat_from_shader(skybox_sh);
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material.shader = skybox_sh;
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
	usefullscreen->data.mat = nyas_mat_from_shader(fs_sh);
	usefullscreen->execute = nyas_setshader_fn;
	clear->next = usefullscreen;

	nyas_cmd *draw = nyas_cmd_alloc();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material.shader = fs_sh;
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

int main(int argc, char **argv)
{
	(void)argc; (void)argv;
	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS Asset Inspector", 1920, 1080, true);
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
