#include "pbr.h"
#include "pbr_types.h"
#include "gui.h"
#include "mathc.h"
#include "nyas.h"
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

void
Init(void)
{
	e.light[0] = 1.0f;
	e.light[1] = -1.0f;
	e.light[2] = 1.0f;
	e.light[3] = 1.0f;

	shader = nyas_shader_create(g_shader_descriptors.pbr);
	skybox_sh = nyas_shader_create(g_shader_descriptors.sky);
	fs_sh = nyas_shader_create(g_shader_descriptors.fullscreen_img);
	eqr_sh = nyas_shader_create(g_shader_descriptors.cubemap_from_equirect);
	pref_sh = nyas_shader_create(g_shader_descriptors.prefilter);
	lut_sh = nyas_shader_create(g_shader_descriptors.lut);

	e.env = nyas_tex_load("assets/tex/env/helipad-env.hdr", 1,
	                      g_tex_flags.env);
	e.envirr = nyas_tex_load("assets/tex/env/helipad-dif.hdr", 1,
	                         g_tex_flags.env);

	e.albedo = nyas_tex_load("../pbr/assets/tex/peeling/peeling_A.png", 1,
	                         g_tex_flags.albedo);
	e.normal = nyas_tex_load("../pbr/assets/tex/peeling/peeling_N.png", 1,
	                         g_tex_flags.normal);
	e.roughness = nyas_tex_load("../pbr/assets/tex/peeling/peeling_R.png", 1,
	                            g_tex_flags.pbr_map);
	e.metalness = nyas_tex_load("../pbr/assets/tex/peeling/peeling_M.png", 1,
	                            g_tex_flags.pbr_map);

	skybox_tex = nyas_tex_empty(1024, 1024, g_tex_flags.sky);
	e.upbr.use_albedo_map = 1.0f;
	e.upbr.normal_map_intensity = 1.0f;
	e.upbr.use_pbr_maps = 1.0f;
	e.upbr.tiling_x = 1.0f;
	e.upbr.tiling_y = 1.0f;
	e.upbr.reflectance = 0.5f;
	e.e = nyas_entity_create();
	float position[3] = { 0.0f, 0.0f, 0.0f };
	mat4_translation(e.e->transform, e.e->transform, position);
	e.e->mesh = nyas_mesh_load_file("assets/obj/matball.obj");
	e.e->mat = nyas_mat_pers(shader);
	*(nyas_pbr_desc_unit *)e.e->mat.ptr = e.upbr;
	nyas_tex *t = nyas_mat_tex(e.e->mat);
	t[0] = e.albedo;
	t[1] = e.metalness;
	t[2] = e.roughness;
	t[3] = e.normal;

	nyas_tex lut = nyas_tex_empty(512, 512, g_tex_flags.lut);
	nyas_tex irr = nyas_tex_empty(1024, 1024, g_tex_flags.irr);
	nyas_tex pref = nyas_tex_empty(256, 256, g_tex_flags.prefilter);

	vec4_assign(((nyas_pbr_desc_scene *)nyas_shader_data(shader))->sunlight,
	            e.light);
	nyas_tex *pbr_scene_tex = nyas_shader_tex(shader);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irr;
	pbr_scene_tex[2] = pref;

	*nyas_shader_tex(skybox_sh) = skybox_tex;

	GenEnv env = { .eqr_sh = eqr_sh,
		           .lut_sh = lut_sh,
		           .pref_sh = pref_sh,
		           .eqr_in = e.env,
		           .eqr_out = skybox_tex,
		           .irr_in = e.envirr,
		           .irr_out = irr,
		           .pref_out = pref,
		           .lut = lut };

	GeneratePbrEnv(&env);
	fb = nyas_fb_create();
	nyas_tex fb_tex = InitMainFramebuffer(fb);
	*nyas_shader_tex(fs_sh) = fb_tex;
}

void
Update(nyas_chrono *dt)
{
	float delta = nyas_time_sec(nyas_elapsed(*dt));
	*dt = nyas_time();
	nyas_io_poll();
	nyas_input_read();
	nyas_v2i vp = nyas_window_size();
	nyas_camera_control(&camera, delta);

	nyas_pbr_desc_scene *common_pbr = nyas_shader_data(shader);
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);

	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = fb;
	fbuff->data.set_fb.vp_x = vp.x;
	fbuff->data.set_fb.vp_y = vp.y;
	fbuff->data.set_fb.attach.type = NYAS_IGNORE;
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
	use_pbr->data.mat = nyas_mat_copy_shader(shader);
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
	use_sky_shader->data.mat = nyas_mat_copy_shader(skybox_sh);
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material.shader = skybox_sh;
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
	usefullscreen->data.mat = nyas_mat_copy_shader(fs_sh);
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
	nyas_io_init("NYAS Asset Inspector", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);
	nuklear_init();
	Init();

	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_window_closed()) {
		Update(&frame_chrono);
		Render();
	}

	return 0;
}
