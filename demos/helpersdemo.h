#include "nyas.h"
#include "mathc.h"

typedef struct GenEnv {
	nyas_shader eqr_sh;
	nyas_shader lut_sh;
	nyas_shader pref_sh;
	nyas_tex eqr_in;
	nyas_tex eqr_out;
	nyas_tex irr_in;
	nyas_tex irr_out;
	nyas_tex pref_out;
	nyas_tex lut;
} GenEnv;

static nyas_mat
GenerateRenderToCubeMat(nyas_shader eqr_sh)
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

	nyas_mat mat_tocube = nyas_mat_tmp(eqr_sh, 16 * 6, 0, 0);
	float *vp = mat_tocube.ptr;

	for (int i = 0; i < 6; ++i) {
		float *view = (float *)&views[i];
		mat4_multiply(vp + 16 * i, proj, view);
	}

	return mat_tocube;
}

static nyas_cmd *
ConvertEqrToCubeCommandList(nyas_shader eqr_sh, nyas_tex tex_in,
                            nyas_tex tex_out,
                            nyas_framebuffer fb,
                            nyas_cmd *last_command)
{
	nyas_cmd *use_tocube = nyas_cmd_alloc();
	use_tocube->data.mat = nyas_mat_tmp(eqr_sh, 0, 1, 0);
	*(nyas_tex *)use_tocube->data.mat.ptr = tex_in;
	use_tocube->execute = nyas_setshader_fn;
	last_command->next = use_tocube;

	nyas_fbattach atta = { .slot = NYAS_ATTACH_COLOR,
		                   .tex = tex_out,
		                   .level = 0 };

	nyas_cmd *last = use_tocube;
	nyas_mat draw_tocube = GenerateRenderToCubeMat(eqr_sh);
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
GeneratePrefilterCommandList(GenEnv *env,
                             nyas_framebuffer fb,
                             nyas_cmd *last_command)
{
	nyas_cmd *use_pref = nyas_cmd_alloc();
	use_pref->data.mat = nyas_mat_tmp(env->pref_sh, 0, 0, 1);
	*(nyas_tex *)use_pref->data.mat.ptr = env->eqr_out;
	use_pref->execute = nyas_setshader_fn;
	last_command->next = use_pref;
	last_command = use_pref;

	nyas_mat mat = GenerateRenderToCubeMat(env->eqr_sh);
	int tex_size[2];
	float tex_width = (float)nyas_tex_size(env->pref_out, tex_size)[0];
	for (int i = 0; i < 5; ++i) {
		int16_t vp_size = (int16_t)(tex_width * powf(0.5f, (float)i));
		float roughness = (float)i / 4.0f;

		for (int side = 0; side < 6; ++side) {
			nyas_cmd *fb_comm = nyas_cmd_alloc();
			fb_comm->data.set_fb.fb = fb;
			fb_comm->data.set_fb.vp_x = vp_size;
			fb_comm->data.set_fb.vp_y = vp_size;
			fb_comm->data.set_fb.attachment.tex = env->pref_out;
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
			draw_comm->data.draw.material =
			  nyas_mat_tmp(env->pref_sh,
			               1 + sizeof(struct mat4) / sizeof(float), 0, 0);
			float *vp = draw_comm->data.draw.material.ptr;
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
GenerateLutCommandlist(nyas_framebuffer fb, nyas_cmd *last_command, GenEnv *env)
{
	nyas_cmd *set_init_fb = nyas_cmd_alloc();
	set_init_fb->data.set_fb.fb = fb;
	set_init_fb->data.set_fb.vp_x = NYAS_IGNORE;
	set_init_fb->data.set_fb.attachment.slot = NYAS_ATTACH_COLOR;
	set_init_fb->data.set_fb.attachment.tex = env->lut;
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

	nyas_mat lutmat = nyas_mat_dft(env->lut_sh);
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
GeneratePbrEnv(GenEnv *env)
{
	nyas_framebuffer auxfb = nyas_fb_create(1, 1, false, false);

	nyas_cmd *rendops = nyas_cmd_alloc();
	rendops->data.rend_opts.enable_flags = NYAS_DEPTH_TEST | NYAS_DEPTH_WRITE |
	  NYAS_BLEND;
	rendops->data.rend_opts.disable_flags = NYAS_CULL_FACE;
	rendops->data.rend_opts.cull_face = NYAS_CULL_FACE_BACK;
	rendops->data.rend_opts.blend_func.src = NYAS_BLEND_FUNC_ONE;
	rendops->data.rend_opts.blend_func.dst = NYAS_BLEND_FUNC_ZERO;
	rendops->execute = nyas_rops_fn;

	nyas_cmd *last_tocube = ConvertEqrToCubeCommandList(env->eqr_sh, env->eqr_in, env->eqr_out,
	                                                    auxfb, rendops);

	last_tocube = ConvertEqrToCubeCommandList(env->eqr_sh, env->irr_in, env->irr_out, auxfb,
	                                          last_tocube);

	nyas_cmd *pref = GeneratePrefilterCommandList(env, auxfb, last_tocube);

	GenerateLutCommandlist(auxfb, pref, env);
	nyas_cmd_add(rendops);
}
