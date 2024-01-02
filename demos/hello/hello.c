#include "nyas.h"
#include <mathc.h>
#include <string.h>

typedef struct {
	float model[16];
	float vp[16];
	float color[4];
} HelloMatData;

typedef struct HelloCtx {
	HelloMatData hello_mat;
	nyas_shader hellomat;
	nyas_shader fs_img;
	nyas_shader skybox;
	nyas_framebuffer fb;
	nyas_tex skycube;
	nyas_entity *e;
	nyas_chrono chrono;
} HelloCtx;

void
Init(void *context)
{
	HelloCtx *ctx = context;
	ctx->chrono = nyas_time();
	nyas_v2i sz = nyas_window_size();
	ctx->fb = nyas_fb_create(sz.x, sz.y, true, true);

	nyas_shader_desc hello_desc = { .name = "hello",
		                            .data_count = sizeof(HelloMatData) / 4,
		                            .tex_count = 0,
		                            .cubemap_count = 0,
		                            .common_data_count = 0,
		                            .common_tex_count = 0,
		                            .common_cubemap_count = 0 };
	ctx->hellomat = nyas_shader_create(&hello_desc);

	nyas_shader_desc fs_desc = { .name = "fullscreen-img",
		                         .data_count = 0,
		                         .tex_count = 0,
		                         .cubemap_count = 0,
		                         .common_data_count = 0,
		                         .common_tex_count = 1,
		                         .common_cubemap_count = 0 };
	ctx->fs_img = nyas_shader_create(&fs_desc);

	nyas_shader_desc skybox_desc = { .name = "skybox",
		                             .data_count = 0,
		                             .tex_count = 0,
		                             .cubemap_count = 0,
		                             .common_data_count = 16,
		                             .common_tex_count = 0,
		                             .common_cubemap_count = 1 };
	ctx->skybox = nyas_shader_create(&skybox_desc);

	*nyas_shader_tex(ctx->fs_img) = nyas_fb_color(ctx->fb);

	int sky_flags = nyas_tex_flags(3, false, false, true, false, false, false);
	ctx->skycube = nyas_tex_load("./assets/tex/%ccave.png", 0, sky_flags);
	*nyas_shader_tex(ctx->skybox) = ctx->skycube;

	nyas_set_color(ctx->hello_mat.color, 1.0f, 0.0f, 1.0f, 1.0f);

	ctx->e = nyas_entity_create();
	float pos[3] = { 0.0f, 0.0f, -4.0f };
	mat4_translation(ctx->e->transform, mat4_identity(ctx->e->transform), pos);
	ctx->e->mesh = SPHERE_MESH;
	ctx->e->mat = nyas_mat_pers(ctx->hellomat);
	*(HelloMatData *)ctx->e->mat.ptr = ctx->hello_mat;
}

void
Update(void *context)
{
	HelloCtx *ctx = context;
	float dt = nyas_time_sec(nyas_elapsed(ctx->chrono));
	ctx->chrono = nyas_time();

	nyas_io_poll();
	nyas_input_read();
	nyas_camera_control(&camera, dt);

	nyas_cmd *fbuff = nyas_cmd_alloc();
	fbuff->data.set_fb.fb = ctx->fb;
	fbuff->data.set_fb.attachment.slot = NYAS_IGNORE;
	fbuff->execute = nyas_setfb_fn;

	nyas_cmd *rops = nyas_cmd_alloc();
	fbuff->next = rops;
	rops->data.rend_opts.enable_flags = NYAS_BLEND | NYAS_DEPTH_TEST |
	  NYAS_DEPTH_WRITE | NYAS_CULL_FACE;
	rops->data.rend_opts.blend_func.src = NYAS_BLEND_FUNC_ONE;
	rops->data.rend_opts.blend_func.dst = NYAS_BLEND_FUNC_ZERO;
	rops->data.rend_opts.cull_face = NYAS_CULL_FACE_BACK;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LESS;
	rops->execute = nyas_rops_fn;

	mat4_multiply(((HelloMatData *)ctx->e->mat.ptr)->vp, camera.proj,
	              camera.view);

	nyas_cmd *clear = nyas_cmd_alloc();
	rops->next = clear;
	clear->data.clear.color_buffer = true;
	clear->data.clear.depth_buffer = true;
	clear->data.clear.stencil_buffer = false;
	nyas_set_color(clear->data.clear.color, 0.2f, 0.2f, 0.2f, 1.0f);
	clear->execute = nyas_clear_fn;

	nyas_cmd *usemat = nyas_cmd_alloc();
	usemat->data.mat.shader = ctx->hellomat;
	usemat->execute = nyas_setshader_fn;
	clear->next = usemat;
	usemat->next = NULL;

	nyas_cmd_add(fbuff);

	nyas_entity_draw(nyas_entities(), nyas_entity_count());

	rops = nyas_cmd_alloc();
	rops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LEQUAL;
	rops->execute = nyas_rops_fn;

	nyas_camera_static_vp(nyas_shader_data(ctx->skybox), &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = nyas_mat_from_shader(ctx->skybox);
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material.shader = ctx->skybox;
	draw_sky->execute = nyas_draw_fn;
	use_sky_shader->next = draw_sky;

	nyas_cmd *fbuff2 = nyas_cmd_alloc();
	fbuff2->data.set_fb.fb = NYAS_DEFAULT;
	fbuff2->data.set_fb.attachment.slot = NYAS_IGNORE;
	fbuff2->execute = nyas_setfb_fn;
	draw_sky->next = fbuff2;

	nyas_cmd *rops2 = nyas_cmd_alloc();
	fbuff2->next = rops2;
	rops2->data.rend_opts.disable_flags = NYAS_DEPTH_TEST;
	rops2->execute = nyas_rops_fn;

	nyas_cmd *clear2 = nyas_cmd_alloc();
	clear2->data.clear.color_buffer = true;
	clear2->data.clear.depth_buffer = false;
	clear2->data.clear.stencil_buffer = false;
	nyas_set_color(clear2->data.clear.color, 0.0f, 1.0f, 1.0f, 1.0f);
	clear2->execute = nyas_clear_fn;
	rops2->next = clear2;

	nyas_cmd *usefullscreen = nyas_cmd_alloc();
	usefullscreen->data.mat = nyas_mat_from_shader(ctx->fs_img);
	usefullscreen->execute = nyas_setshader_fn;
	clear2->next = usefullscreen;

	nyas_cmd *draw = nyas_cmd_alloc();
	draw->data.draw.mesh = QUAD_MESH;
	draw->data.draw.material.shader = ctx->fs_img;
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
	(void)argc;
	(void)argv;

	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS HelloWorld", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);
	nyas_imgui_init();

	HelloCtx ctx;
	Init(&ctx);

	while (!nyas_window_closed()) {
		Update(&ctx);
		Render();
	}

	return 0;
}
