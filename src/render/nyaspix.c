#include "nyaspix.h"

#include <glad/glad.h>

void nypx_tex_create(nypx_tex *t)
{
	glGenTextures(1, (GLuint *)&(t->res.id));
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, t->res.id);
}

void nypx_tex_set(nypx_tex *t)
{

}

void nypx_tex_release(nypx_tex *t)
{

}

void nypx_mesh_create(nypx_mesh *m)
{

}

void nypx_mesh_use(nypx_mesh *m)
{

}

void nypx_mesh_set(nypx_mesh *m)
{

}

void nypx_mesh_release(nypx_mesh *m)
{

}

void nypx_shader_create(nypx_shader *s)
{
	s->res_vert.id = glCreateShader(GL_VERTEX_SHADER);
	s->res_frag.id = glCreateShader(GL_FRAGMENT_SHADER);
	s->res.id = glCreateProgram();
}

void nypx_shader_compile(nypx_shader *s)
{

}

void nypx_shader_use(nypx_shader *s)
{
	glUseProgram(s->res.id);
}

void nypx_shader_set(nypx_shader *s, float *data, int *tex, int *cubemap)
{
	glUniform4fv(s->loc_data, s->data_count / 4, data);
	glUniform1iv(s->loc_tex, s->tex_count, tex);
	glUniform1iv(s->loc_cubemap, s->cubemap_count, cubemap);
}

void nypx_shader_set_common(nypx_shader *s, float *data, int *tex, int *cubemap)
{
	glUniform4fv(s->loc_common_data, s->data_count / 4, data);
	glUniform1iv(s->loc_common_tex, s->tex_count, tex);
	glUniform1iv(s->loc_common_cubemap, s->cubemap_count, cubemap);
}

void nypx_shader_release(nypx_shader *s)
{

}

void nypx_fb_create(nypx_fb *fb)
{

}

void nypx_fb_use(nypx_fb *fb)
{

}

void nypx_fb_set(nypx_fb *fb)
{

}

void nypx_fb_release(nypx_fb *fb)
{

}

void nypx_clear(int color, int depth, int stencil)
{
	GLbitfield mask = 0;
	mask |= (GL_COLOR_BUFFER_BIT * color);
	mask |= (GL_DEPTH_BUFFER_BIT * depth);
	mask |= (GL_STENCIL_BUFFER_BIT * stencil);
	glClear(mask);
}

void nypx_draw(int elem_count, int half_type)
{

}

void nypx_clear_color(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void nypx_blend_enable(void)
{
	glEnable(GL_BLEND);
}

void nypx_blend_disable(void)
{
	glDisable(GL_BLEND);
}

static GLenum
nypx__gl_blend(enum nypx_blend_func bf)
{
	switch (bf)
	{
		case NYPX_BLEND_ONE:
			return GL_ONE;
		case NYPX_BLEND_SRC_ALPHA:
			return GL_SRC_ALPHA;
		case NYPX_BLEND_ONE_MINUS_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case NYPX_BLEND_ZERO:
			return GL_ZERO;
		default:
			return -1;
	}
}

void nypx_blend_set(enum nypx_blend_func src, enum nypx_blend_func dst)
{
	glBlendFunc(nypx__gl_blend(src), nypx__gl_blend(dst));
}

void nypx_cull_enable(void)
{
	glEnable(GL_CULL_FACE);
}

void nypx_cull_disable(void)
{
	glDisable(GL_CULL_FACE);
}

static GLenum
nypx__gl_cull(enum nypx_cull_face cull)
{
	switch (cull)
	{
		case NYPX_CULL_BACK:
			return GL_BACK;
		case NYPX_CULL_FRONT:
			return GL_FRONT;
		case NYPX_CULL_FRONT_AND_BACK:
			return GL_FRONT_AND_BACK;
		default:
			return -1;
	}
}

void nypx_cull_set(enum nypx_cull_face cull)
{
	glCullFace(nypx__gl_cull(cull));
}

void nypx_depth_enable_test(void)
{
	glEnable(GL_DEPTH_TEST);
}

void nypx_depth_disable_test(void)
{
	glDisable(GL_DEPTH_TEST);
}

void nypx_depth_enable_mask(void)
{
	glDepthMask(GL_TRUE);
}

void nypx_depth_disable_mask(void)
{
	glDepthMask(GL_FALSE);
}

static GLenum
nypx__gl_depth(enum nypx_depth_func df)
{
	switch (df)
	{
		case NYPX_DEPTH_LEQUAL:
			return GL_LEQUAL;
		case NYPX_DEPTH_LESS:
			return GL_LESS;
		default:
			return -1;
	}
}

void nypx_depth_set(enum nypx_depth_func depth)
{
	glDepthFunc(nypx__gl_depth(depth));
}

void nypx_viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}
