#include "nyas.h"
#include "mathc.h"

typedef struct entt {
	nyas_entity *e;
	nyas_pbr_desc_unit upbr;
	nyas_mat scene_mat;
	nyas_tex albedo;
	nyas_tex normal;
	nyas_tex roughness;
	nyas_tex metalness;
	nyas_tex env;
	nyas_tex skybox;
	float light[4] = {1.0f, -1.0f, 1.0f, 1.0f};
} entt;

entt e;
nyas_shader shader;
nyas_shader skybox_sh;
nyas_mat skyb_cmmn;

void Init()
{
	shader = nyas_shader_create("pbr");
	skybox_sh = nyas_shader_create("skybox");
	e.skybox = nyas_tex_create(1024, 1024, NYAS_TEX_ENVIRONMENT);
	e.upbr.use_albedo_map = 1.0f;
	e.upbr.normal_map_intensity = 1.0f;
	e.upbr.use_pbr_maps = 1.0f;
	e.e = nyas_entity_create();
	float position[3] = {0.0f, 0.0f, 0.0f};
	mat4_translation(e.e->transform, e.e->transform, position);
	e.e->mat = nyas_mat_pers("pbr", sizeof(e.upbr) / sizeof(float), 4, 0);
	*(nyas_pbr_desc_unit*)e.e->mat.ptr = e.upbr;
	nyas_tex *t = (nyas_tex*)e.e->mat.ptr + (sizeof(e.upbr) / sizeof(float));
	t[0] = e.albedo;
	t[1] = e.normal;
	t[2] = e.roughness;
	t[3] = e.metalness;

	nyas_tex lut = nyas_tex_create(512, 512, NYAS_TEX_LUT);
	nyas_tex irr = nyas_tex_create(1024, 1024, NYAS_TEX_ENVIRONMENT);
	nyas_tex pref = nyas_tex_create(128, 128, NYAS_TEX_PREFILTER_ENVIRONMENT);

	int scene_dcount = sizeof(nyas_pbr_desc_scene) / sizeof(float);
	e.scene_mat = nyas_mat_pers("pbr", scene_dcount, 1, 2);
	nyas_tex *pbr_scene_tex = nyas_mat_tex(&e.scene_mat);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irr;
	pbr_scene_tex[2] = pref;

	skyb_cmmn = nyas_mat_pers(skybox_sh, 16, 0, 1);
	*nyas_mat_tex(&skyb_cmmn) = e.skybox;
}

void Update(nyas_chrono *dt)
{
	float delta = nyas_time_seconds(nyas_elapsed(*dt));
	*dt = nyas_time();
	nyas_io_poll();
	nyas_input_read();
	nyas_camera_control(&camera, delta); 

	nyas_pbr_desc_scene *common_pbr = e.scene_mat.ptr;
	mat4_multiply(common_pbr->view_projection, camera.proj, camera.view);
	nyas_camera_pos(common_pbr->camera_position, &camera);
	vec4_assign(common_pbr->sunlight, g_sunlight);
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