#include "nyas.h"
#include "../helpersdemo.h"
#include "mathc.h"
#include <string.h>

typedef struct entt {
	nyas_entity *e;
	nyas_pbr_desc_unit upbr;
	nyas_mat scene_mat;
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
nyas_mat skyb_cmmn;
nyas_mat fs_mat;

void Init()
{
	fb = nyas_fb_create(nyas_window_width(), nyas_window_height(), true,
	                      true);
	e.light[0] = 1.0f;
	e.light[1] = -1.0f;
	e.light[2] = 1.0f;
	e.light[3] = 1.0f;
	shader = nyas_shader_create("pbr");
	skybox_sh = nyas_shader_create("skybox");
	fs_sh = nyas_shader_create("fullscreen-img");
	eqr_sh = nyas_shader_create("eqr-to-cube");
	pref_sh = nyas_shader_create("prefilter-env");
	lut_sh = nyas_shader_create("lut-gen");

	e.env = nyas_tex_load_img("assets/tex/env/helipad-env.hdr", NYAS_TEX_RGB_F16);
	e.envirr = nyas_tex_load_img("assets/tex/env/helipad-dif.hdr", NYAS_TEX_RGB_F16);

	e.albedo = nyas_tex_load_img("assets/tex/granite/granite_A.png", NYAS_TEX_SRGB);
	e.normal = nyas_tex_load_img("assets/tex/granite/granite_N.png", NYAS_TEX_RGB);
	e.roughness = nyas_tex_load_img("assets/tex/granite/granite_R.png", NYAS_TEX_R);
	e.metalness = nyas_tex_load_img("assets/tex/granite/granite_M.png", NYAS_TEX_R);

	fs_mat = nyas_mat_pers(fs_sh, 0, 1, 0);
	*(nyas_tex *)fs_mat.ptr = nyas_fb_color(fb);

	skybox_tex = nyas_tex_create(1024, 1024, NYAS_TEX_ENVIRONMENT);
	e.upbr.use_albedo_map = 1.0f;
	e.upbr.normal_map_intensity = 1.0f;
	e.upbr.use_pbr_maps = 1.0f;
	e.upbr.tiling_x = 1.0f;
	e.upbr.tiling_y = 1.0f;
	e.e = nyas_entity_create();
	float position[3] = {0.0f, 0.0f, 0.0f};
	mat4_translation(e.e->transform, e.e->transform, position);
	e.e->mesh = nyas_mesh_load_obj("assets/obj/matball-n.obj");
	e.e->mat = nyas_mat_pers(shader, sizeof(e.upbr) / sizeof(float), 4, 0);
	*(nyas_pbr_desc_unit*)e.e->mat.ptr = e.upbr;
	nyas_tex *t = (nyas_tex*)e.e->mat.ptr + (sizeof(e.upbr) / sizeof(float));
	t[0] = e.albedo;
	t[1] = e.metalness;
	t[2] = e.roughness;
	t[3] = e.normal;

	nyas_tex lut = nyas_tex_create(512, 512, NYAS_TEX_LUT);
	nyas_tex irr = nyas_tex_create(1024, 1024, NYAS_TEX_ENVIRONMENT);
	nyas_tex pref = nyas_tex_create(128, 128, NYAS_TEX_PREFILTER_ENVIRONMENT);

	int scene_dcount = sizeof(nyas_pbr_desc_scene) / sizeof(float);
	e.scene_mat = nyas_mat_pers(shader, scene_dcount, 1, 2);
	nyas_tex *pbr_scene_tex = nyas_mat_tex(&e.scene_mat);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irr;
	pbr_scene_tex[2] = pref;

	skyb_cmmn = nyas_mat_pers(skybox_sh, 16, 0, 1);
	*nyas_mat_tex(&skyb_cmmn) = skybox_tex;

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

	nyas_pbr_desc_scene *common_pbr = e.scene_mat.ptr;
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
	use_pbr->data.mat = e.scene_mat;
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

	nyas_camera_static_vp(skyb_cmmn.ptr, &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = skyb_cmmn;
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material = skyb_cmmn;
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
	usefullscreen->data.mat = fs_mat;
	usefullscreen->execute = nyas_setshader_fn;
	clear->next = usefullscreen;

	nyas_cmd *draw = nyas_cmd_alloc();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material = fs_mat;
	draw->execute = nyas_draw_fn;
	usefullscreen->next = draw;
	draw->next = NULL;

	nyas_cmd_add(rops);
}

void Render()
{
	nyas_px_render();
	nyas_imgui_draw();
	nyas_window_swap();
	nyas_frame_end();
}

int main(int argc, char **argv)
{
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
