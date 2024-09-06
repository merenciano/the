#include <the.h>
#include <render/pixels_internal.h>

#include <mathc.h>
#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

struct header {
	size_t skybox;
	size_t irradiance;
	size_t prefilter[6];
	size_t lut;
};

static const struct the_shader_desc eqr_to_cubemap_shader_desc = {
	.name = "eqr-to-cube", // environment image to cubemap
	.data_count = 4 * 4,
	.tex_count = 0,
	.cubemap_count = 0,
	.shared_data_count = 0,
	.common_tex_count = 1,
	.common_cubemap_count = 0
};

static const struct the_shader_desc prefilter_shader_desc = {
	.name = "prefilter-env", // environment prefilter
	.data_count = 5 * 4,
	.tex_count = 0,
	.cubemap_count = 0,
	.shared_data_count = 0,
	.common_tex_count = 0,
	.common_cubemap_count = 1
};

static const struct the_shader_desc lut_shader_desc = {
	.name = "env-dfg", // look-up table
	.data_count = 0,
	.tex_count = 0,
	.cubemap_count = 0,
	.shared_data_count = 0,
	.common_tex_count = 0,
	.common_cubemap_count = 0
};

struct ShaderDescriptors {
	const struct the_shader_desc *prefilter;
	const struct the_shader_desc *cubemap_from_equirect;
	const struct the_shader_desc *lut;
};

static const struct ShaderDescriptors g_shader_descriptors = {
	.cubemap_from_equirect = &eqr_to_cubemap_shader_desc,
	.prefilter = &prefilter_shader_desc,
	.lut = &lut_shader_desc
};


struct pbr_tex_desc {
	struct the_texture_desc env;
	struct the_texture_desc sky;
	struct the_texture_desc lut;
	struct the_texture_desc irr;
	struct the_texture_desc prefilter;
};

static struct pbr_tex_desc g_tex_flags;

typedef struct GenEnv {
	the_shader eqr_sh;
	the_shader lut_sh;
	the_shader pref_sh;
	the_tex eqr_in;
	the_tex eqr_out;
	the_tex irr_in;
	the_tex irr_out;
	the_tex pref_out;
	the_tex lut;
} GenEnv;

static float *
GenerateRenderToCubeVP(void)
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

	float *vp = the_alloc_frame(16 * 6 * sizeof(float));
	for (int i = 0; i < 6; ++i) {
		float *view = (float *)&views[i];
		mat4_multiply(vp + 16 * i, proj, view);
	}

	return vp;
}

static struct the_cmd *
ConvertEqrToCubeCommandList(
  the_shader eqr_sh,
  the_tex tex_in,
  the_tex tex_out,
  the_framebuffer fb,
  struct the_draw_cmd *last_command)
{
	struct the_draw_cmd *use_tocube = the_cmd_alloc();
	*the_shader_tex(eqr_sh) = tex_in;
	use_tocube->data.mat = the_mat_copy_shader(eqr_sh);
	use_tocube->execute = the_setshader_fn;
	last_command->next = use_tocube;

	the_cmd *last = use_tocube;
	float *vp = GenerateRenderToCubeVP();
	struct the_vec2i vp_size = the_tex_size(tex_out);
	for (int i = 0; i < 6; ++i) {
		the_cmd *cube_fb = the_cmd_alloc();
		cube_fb->data.set_fb.fb = fb;
		cube_fb->data.set_fb.vp_x = vp_size.x;
		cube_fb->data.set_fb.vp_y = vp_size.y;
		cube_fb->data.set_fb.attach.tex = tex_out;
		cube_fb->data.set_fb.attach.type = THE_SLOT_COLOR0;
		cube_fb->data.set_fb.attach.mip_level = 0;
		cube_fb->data.set_fb.attach.face = i;
		cube_fb->execute = the_setfb_fn;
		last->next = cube_fb;

		the_cmd *draw_cube = the_cmd_alloc();
		draw_cube->data.draw.mesh = CUBE_MESH;
		draw_cube->data.draw.material = the_mat_tmp(eqr_sh);
		mat4_assign(draw_cube->data.draw.material.ptr, vp + 16 * i);
		draw_cube->execute = the_draw_fn;
		cube_fb->next = draw_cube;
		last = draw_cube;
	}

	return last;
}

static the_cmd *
GeneratePrefilterCommandList(
  GenEnv *env,
  the_framebuffer fb,
  the_cmd *last_command)
{
	the_cmd *use_pref = the_cmd_alloc();
	*the_shader_cubemap(env->pref_sh) = env->eqr_out;
	use_pref->data.mat = the_mat_copy_shader(env->pref_sh);
	use_pref->execute = the_setshader_fn;
	last_command->next = use_pref;
	last_command = use_pref;
	float clearcolor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

	float *vp = GenerateRenderToCubeVP();
	struct the_vec2i tex_size = the_tex_size(env->pref_out);
	for (int i = 0; i < 9; ++i) {
		float roughness = (float)i / 6.0f;

		for (int side = 0; side < 6; ++side) {
			the_cmd *fb_comm = the_cmd_alloc();
			fb_comm->data.set_fb.fb = fb;
			fb_comm->data.set_fb.vp_x = tex_size.x >> i;
			fb_comm->data.set_fb.vp_y = tex_size.y >> i;
			fb_comm->data.set_fb.attach.tex = env->pref_out;
			fb_comm->data.set_fb.attach.type = THE_SLOT_COLOR0;
			fb_comm->data.set_fb.attach.face = side;
			fb_comm->data.set_fb.attach.mip_level = i;
			fb_comm->execute = the_setfb_fn;
			last_command->next = fb_comm;

			the_cmd *clear_comm = the_cmd_alloc();
			clear_comm->data.clear.color[0] = clearcolor[0];
			clear_comm->data.clear.color[1] = clearcolor[1];
			clear_comm->data.clear.color[2] = clearcolor[2];
			clear_comm->data.clear.color[3] = clearcolor[3];
			clear_comm->data.clear.color_buffer = true;
			clear_comm->data.clear.depth_buffer = false;
			clear_comm->data.clear.stencil_buffer = false;
			clear_comm->execute = the_clear_fn;
			fb_comm->next = clear_comm;

			the_cmd *draw_comm = the_cmd_alloc();
			draw_comm->data.draw.mesh = CUBE_MESH;
			draw_comm->data.draw.material = the_mat_tmp(env->pref_sh);
			mat4_assign(draw_comm->data.draw.material.ptr, vp + 16 * side);
			((float *)draw_comm->data.draw.material.ptr)[16] = roughness;
			((float *)draw_comm->data.draw.material.ptr)[17] = roughness;
			((float *)draw_comm->data.draw.material.ptr)[18] = roughness;
			((float *)draw_comm->data.draw.material.ptr)[19] = roughness;

			draw_comm->execute = the_draw_fn;
			clear_comm->next = draw_comm;
			draw_comm->next = NULL;
			last_command = draw_comm;
		}
	}

	return last_command;
}

static the_cmd *
GenerateLutCommandlist(
  the_framebuffer fb,
  the_cmd *last_command,
  GenEnv *env)
{
	struct the_vec2i vp_size = the_tex_size(env->lut);
	the_cmd *set_init_fb = the_cmd_alloc();
	set_init_fb->data.set_fb.fb = fb;
	set_init_fb->data.set_fb.vp_x = vp_size.x;
	set_init_fb->data.set_fb.vp_y = vp_size.y;
	set_init_fb->data.set_fb.attach.type = THE_SLOT_COLOR0;
	set_init_fb->data.set_fb.attach.tex = env->lut;
	set_init_fb->data.set_fb.attach.face = -1;
	set_init_fb->data.set_fb.attach.mip_level = 0;
	set_init_fb->execute = the_setfb_fn;
	last_command->next = set_init_fb;

	the_cmd *clear_comm = the_cmd_alloc();
	clear_comm->data.clear.color_buffer = true;
	clear_comm->data.clear.depth_buffer = false;
	clear_comm->data.clear.stencil_buffer = false;
	clear_comm->execute = the_clear_fn;
	set_init_fb->next = clear_comm;

	the_cmd *set_lut = the_cmd_alloc();
	set_lut->data.mat.shader = env->lut_sh;
	set_lut->execute = the_setshader_fn;
	clear_comm->next = set_lut;

	the_cmd *draw_lut = the_cmd_alloc();
	draw_lut->data.draw.mesh = QUAD_MESH;
	draw_lut->data.draw.material.shader = env->lut_sh;
	draw_lut->execute = the_draw_fn;
	set_lut->next = draw_lut;
	draw_lut->next = NULL;

	return draw_lut;
}

static void
GeneratePbrEnv(GenEnv *env)
{
	the_framebuffer auxfb = the_fb_create();

	the_cmd *rendops = the_cmd_alloc();
	rendops->data.rend_opts.enable_flags = THE_DEPTH_TEST | THE_DEPTH_WRITE |
	  THE_BLEND;
	rendops->data.rend_opts.disable_flags = THE_CULL_FACE;
	rendops->data.rend_opts.cull_face = THE_CULL_BACK;
	rendops->data.rend_opts.blend_src = THE_BLEND_ONE;
	rendops->data.rend_opts.blend_dst = THE_BLEND_ZERO;
	rendops->execute = the_rops_fn;

	the_cmd *last_tocube = ConvertEqrToCubeCommandList(
	  env->eqr_sh, env->eqr_in, env->eqr_out, auxfb, rendops);

	last_tocube = ConvertEqrToCubeCommandList(
	  env->eqr_sh, env->irr_in, env->irr_out, auxfb, last_tocube);

	the_cmd *pref = GeneratePrefilterCommandList(env, auxfb, last_tocube);

	GenerateLutCommandlist(auxfb, pref, env);
	the_cmd_add(rendops);
}

static void write_cubemap_data(FILE *f, unsigned int texid, const size_t side_size, int lod_level)
{
	char buffer[side_size];
	glBindTexture(GL_TEXTURE_CUBE_MAP, texid);
	for (int i = 0; i < 6; ++i) {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, lod_level, GL_RGB, GL_HALF_FLOAT, buffer);
		fwrite(buffer, side_size, 1, f);
	}
}

int
main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Usage: genenv <envfile>.hdr <diffile>.hdr <output>.env\n");
		return 1;
	}

	the_mem_init(THE_GB(1));
	the_io_init("Environment file generation tool", 600, 400, true);
	the_px_init();

	g_tex_flags.env = the_tex_default_desc(THE_TEX_2D);
	g_tex_flags.env.min_filter = THE_TEX_FLTR_LINEAR;
	g_tex_flags.env.desired_channels = 3;

	the_shader eqr_to_cube =
	  the_shader_create(g_shader_descriptors.cubemap_from_equirect);
	the_shader prefilter_env =
	  the_shader_create(g_shader_descriptors.prefilter);
	the_shader lut_gen = the_shader_create(g_shader_descriptors.lut);

	g_tex_flags.sky = the_tex_defined_desc(THE_TEX_CUBEMAP, THE_TEX_FMT_RGB16F, 1024, 1024);
	g_tex_flags.sky.min_filter = THE_TEX_FLTR_LINEAR;
	g_tex_flags.prefilter = the_tex_defined_desc(THE_TEX_CUBEMAP, THE_TEX_FMT_RGB16F, 256, 256);
	g_tex_flags.prefilter.min_filter = THE_TEX_FLTR_LINEAR_MIPMAP_LINEAR;
	g_tex_flags.prefilter.mag_filter = THE_TEX_FLTR_LINEAR;
	g_tex_flags.prefilter.flags = THE_TEX_FLAG_GENERATE_MIPMAPS;
	g_tex_flags.lut = the_tex_defined_desc(THE_TEX_2D, THE_TEX_FMT_RG16F, 512, 512);
	the_tex sky = the_tex_empty(&g_tex_flags.sky);
	the_tex irradiance = the_tex_empty(&g_tex_flags.sky);
	the_tex prefilter = the_tex_empty(&g_tex_flags.prefilter);
	the_tex lut = the_tex_empty(&g_tex_flags.lut);

	the_tex eqr_in_tex =
	  the_tex_loadf(&g_tex_flags.env, argv[1], true);
	the_tex irr_in_tex =
	  the_tex_loadf(&g_tex_flags.env, argv[2], true);

	GenEnv env = {
		.eqr_sh = eqr_to_cube,
		.lut_sh = lut_gen,
		.pref_sh = prefilter_env,
		.eqr_in = eqr_in_tex,
		.eqr_out = sky,
		.irr_in = irr_in_tex,
		.irr_out = irradiance,
		.pref_out = prefilter,
		.lut = lut
	};

	GeneratePbrEnv(&env);

	the_px_render();
	the_window_swap();
	the_frame_end();

	the_px_render();
	the_window_swap();
	the_frame_end();

	FILE *f = fopen(argv[3], "w");
	const char *hdr = "NYAS_ENV";
	fwrite(hdr, 8, 1, f);

	{
		const size_t size = 1024 * 1024 * 3 * 2;
		write_cubemap_data(f, tex_pool[sky].res.id, size, 0);
		write_cubemap_data(f, tex_pool[irradiance].res.id, size, 0);
	}

	{
		size_t size = 256 * 256 * 3 * 2;
		for (int level = 0; level < 9; ++level) {
			write_cubemap_data(f, tex_pool[prefilter].res.id, size, level);
			size /= 4;
		}
	}

	{
		const size_t size = 512 * 512 * 2 * 2;
		char buffer[size];
		glBindTexture(GL_TEXTURE_2D, tex_pool[lut].res.id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_HALF_FLOAT, buffer);
		fwrite(buffer, size, 1, f);
	}

	const char *eof = "_END_ENV";
	fwrite(eof, 8, 1, f);
	fclose(f);

	return 0;
}
