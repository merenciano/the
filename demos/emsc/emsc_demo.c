#include "nyas.h"
#include <mathc.h>
#include <emscripten.h>

typedef struct {
	float model[16];
	float vp[16];
	float color[4];
} HelloMatData;

typedef struct HelloCtx {
	HelloMatData hello_mat;
	nyas_shader hellomat;
	nyas_shader skybox;
	nyas_tex skycube;
	nyas_entity *e;
} HelloCtx;

HelloCtx g_ctx;

void
Init(void *context)
{
	HelloCtx *ctx = context;

	nyas_shader_desc hello_desc = { .name = "hello",
		                            .data_count = sizeof(HelloMatData) / 4,
		                            .tex_count = 0,
		                            .cubemap_count = 0,
		                            .shared_data_count = 0,
		                            .common_tex_count = 0,
		                            .common_cubemap_count = 0 };
	ctx->hellomat = nyas_shader_create(&hello_desc);

	nyas_shader_desc skybox_desc = { .name = "skybox",
		                             .data_count = 0,
		                             .tex_count = 0,
		                             .cubemap_count = 0,
		                             .shared_data_count = 16,
		                             .common_tex_count = 0,
		                             .common_cubemap_count = 1 };
	ctx->skybox = nyas_shader_create(&skybox_desc);

	int sky_flags = nyas_tex_flags(3, false, false, true, false, false, false);
	ctx->skycube = nyas_tex_load("assets/tex/%ccave.png", 0, sky_flags);
	*nyas_shader_tex(ctx->skybox) = ctx->skycube;

	nyas_set_color(ctx->hello_mat.color, 1.0f, 0.0f, 1.0f, 1.0f);

	ctx->e = nyas_entity_create();
	float pos[3] = { 0.0f, 0.0f, -4.0f };
	mat4_translation(ctx->e->transform, mat4_identity(ctx->e->transform), pos);
	ctx->e->mesh = SPHERE_MESH;
	ctx->e->mat = nyas_mat_create(ctx->hellomat);
	*(HelloMatData *)ctx->e->mat.ptr = ctx->hello_mat;
}

void
Update(void)
{
	HelloCtx *ctx = &g_ctx;
	float dt = 0.016f;

	nyas_io_poll();
	nyas_input_read();
	nyas_camera_control(&camera, dt);

	nyas_cmd *rops = nyas_cmd_alloc();
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

	nyas_cmd_add(rops);

	nyas_entity_draw(nyas_entities(), nyas_entity_count());

	rops = nyas_cmd_alloc();
	rops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rops->data.rend_opts.depth_func = NYAS_DEPTH_FUNC_LEQUAL;
	rops->execute = nyas_rops_fn;

	nyas_camera_static_vp(nyas_shader_data(ctx->skybox), &camera);
	nyas_cmd *use_sky_shader = nyas_cmd_alloc();
	use_sky_shader->data.mat = nyas_mat_copy_shader(ctx->skybox);
	use_sky_shader->execute = nyas_setshader_fn;
	rops->next = use_sky_shader;

	nyas_cmd *draw_sky = nyas_cmd_alloc();
	draw_sky->data.draw.mesh = CUBE_MESH;
	draw_sky->data.draw.material.shader = ctx->skybox;
	draw_sky->execute = nyas_draw_fn;
	use_sky_shader->next = draw_sky;

	nyas_cmd_add(rops);
}

void
Render(void)
{
	nyas_px_render();
	nyas_window_swap();
	nyas_frame_end();
}

static void MainLoopForEmscripten(void)
{
	Update();
	Render();
}

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	nyas_mem_init(NYAS_GB(1));
	nyas_io_init("NYAS Demo", 1920, 1080, true);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, 1280, 720);

	Init(&g_ctx);
	emscripten_set_main_loop(MainLoopForEmscripten, 0, true);

	return 0;
}
