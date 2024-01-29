#include "pixels_internal.h"

#include "core/io.h"
#include "core/log.h"
#include "core/mem.h"

#include <string.h>
#include <GLES2/gl2.h>

struct texcnfg {
	GLenum target;
	GLenum fmt;
	GLint wrap;
	GLint min;
	GLint mag;
};

/*
  Attribute's number of elements for each vertex.
  The array's position must match with the
  enum (nyas_VertexAttributes) value of the attribute.
*/
static const GLint attrib_sizes[VTXATTR_COUNT] = { 3, 3, 3, 3, 2 };

/*
  Attribute's layout name in the shader.
  The array's position must match with the attribute's
  value at enum nyas_VertexAttributes.
*/
static const char *attrib_names[VTXATTR_COUNT] = { "a_position", "a_normal",
	                                               "a_tangent", "a_bitangent",
	                                               "a_uv" };

static struct texcnfg
nypx__texcnfg(int flags)
{
	if (flags & TF_DEPTH) {
		struct texcnfg depthcnfg = {
			.target = GL_TEXTURE_2D,
			.fmt = GL_DEPTH_COMPONENT,
			.wrap = GL_CLAMP_TO_EDGE,
			.min = GL_LINEAR,
			.mag = GL_LINEAR,
		};

		return depthcnfg;
	}

	struct texcnfg cnfg = {
		.target = flags & TF_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D,
		.fmt = GL_RGB,
		.wrap = flags & TF_TILING ? GL_REPEAT : GL_CLAMP_TO_EDGE,
		.min = flags & TF_MIN_FILTER_LERP ? GL_LINEAR : GL_NEAREST,
		.mag = flags & TF_MAG_FILTER_LERP ? GL_LINEAR : GL_NEAREST
	};

	switch (flags & 0x3) {
	case 0:
		cnfg.fmt = GL_LUMINANCE;
		break;

	case 1:
		cnfg.fmt = GL_LUMINANCE_ALPHA;
		break;

	case 3:
		cnfg.fmt = GL_RGBA;
		break;
	}

	if ((flags & (TF_MIPMAP | TF_CUBE)) == (TF_MIPMAP | TF_CUBE)) {
		if (flags & (TF_MAG_FILTER_LERP | TF_MAG_MIP_FILTER_LERP)) {
			cnfg.min = GL_LINEAR_MIPMAP_LINEAR;
		} else if (flags & TF_MAG_MIP_FILTER_LERP) {
			cnfg.min = GL_NEAREST_MIPMAP_LINEAR;
		} else if (flags & TF_MAG_FILTER_LERP) {
			cnfg.min = GL_LINEAR_MIPMAP_NEAREST;
		} else {
			cnfg.min = GL_NEAREST_MIPMAP_NEAREST;
		}

		if (flags & (TF_MAG_FILTER_LERP | TF_MAG_MIP_FILTER_LERP)) {
			cnfg.mag = GL_LINEAR_MIPMAP_LINEAR;
		} else if (flags & TF_MAG_MIP_FILTER_LERP) {
			cnfg.mag = GL_NEAREST_MIPMAP_LINEAR;
		} else if (flags & TF_MAG_FILTER_LERP) {
			cnfg.mag = GL_LINEAR_MIPMAP_NEAREST;
		} else {
			cnfg.mag = GL_NEAREST_MIPMAP_NEAREST;
		}
	}

	return cnfg;
}

void
nypx_tex_create(uint32_t *id, int type)
{
	struct texcnfg cnfg = nypx__texcnfg(type);
	glGenTextures(1, id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(cnfg.target, *id);
	glTexParameteri(cnfg.target, GL_TEXTURE_MIN_FILTER, cnfg.min);
	glTexParameteri(cnfg.target, GL_TEXTURE_MAG_FILTER, cnfg.mag);
	glTexParameteri(cnfg.target, GL_TEXTURE_WRAP_S, cnfg.wrap);
	glTexParameteri(cnfg.target, GL_TEXTURE_WRAP_T, cnfg.wrap);
}

void
nypx_tex_set(uint32_t id, int type, int width, int height, void **pix)
{
	struct texcnfg cnfg = nypx__texcnfg(type);
	int sides = 1;
	GLenum target = cnfg.target;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(cnfg.target, id);

	if (type & TF_CUBE) {
		sides = 6;
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	}

	for (int i = 0; i < sides; ++i) {
		glTexImage2D(target + i, 0, cnfg.fmt, width, height, 0, cnfg.fmt,
		             GL_UNSIGNED_BYTE, pix[i]);
	}

	if (type & TF_MIPMAP) {
		glGenerateMipmap(cnfg.target);
	}
}

void
nypx_tex_release(uint32_t *id)
{
	glDeleteTextures(1, id);
}

void
nypx_mesh_create(uint32_t *id, uint32_t *vid, uint32_t *iid)
{
	(void)id;
	glGenBuffers(1, vid);
	glGenBuffers(1, iid);
}

static GLsizei
nypx__get_attrib_stride(int32_t attr_flags)
{
	GLsizei stride = 0;
	for (int i = 0; i < VTXATTR_COUNT; ++i) {
		if (attr_flags & (1 << i)) {
			stride += attrib_sizes[i];
		}
	}
	return stride * sizeof(float);
}

void
nypx_mesh_use(struct nyas_internal_mesh *m, struct nyas_internal_shader *s)
{
	if (!m || !s) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m->res_vb.id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->res_ib.id);
	GLint offset = 0;
	GLsizei stride = nypx__get_attrib_stride(m->attrib);
	for (int i = 0; i < VTXATTR_COUNT; ++i) {
		if (!(m->attrib & (1 << i))) {
			continue;
		}

		GLint size = attrib_sizes[i];
		GLint attrib_pos = glGetAttribLocation(s->res.id, attrib_names[i]);
		if (attrib_pos >= 0) {
			glEnableVertexAttribArray(attrib_pos);
			glVertexAttribPointer(attrib_pos, size, GL_FLOAT, GL_FALSE, stride,
			                      (void *)(offset * sizeof(float)));
		}
		offset += size;
	}
}

void
nypx_mesh_set(uint32_t id,
              uint32_t vid,
              uint32_t iid,
              uint32_t shader_id,
              int attrib,
              float *vtx,
              size_t vsize,
              nypx_index *idx,
              size_t elements)
{
	(void)id;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iid);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(nypx_index),
	             (const void *)idx, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vid);
	glBufferData(GL_ARRAY_BUFFER, vsize, vtx, GL_STATIC_DRAW);

	GLint offset = 0;
	GLsizei stride = nypx__get_attrib_stride(attrib);
	for (int i = 0; i < VTXATTR_COUNT; ++i) {
		if (!(attrib & (1 << i))) {
			continue;
		}

		GLint size = attrib_sizes[i];
		GLint attrib_pos = glGetAttribLocation(shader_id, attrib_names[i]);
		if (attrib_pos >= 0) {
			glEnableVertexAttribArray(attrib_pos);
			glVertexAttribPointer(attrib_pos, size, GL_FLOAT, GL_FALSE, stride,
			                      (void *)(offset * sizeof(float)));
		}
		offset += size;
	}
}

void
nypx_mesh_release(uint32_t *id, uint32_t *vid, uint32_t *iid)
{
	(void)id;
	glDeleteBuffers(1, vid);
	glDeleteBuffers(1, iid);
}

void
nypx_shader_create(uint32_t *id)
{
	*id = glCreateProgram();
}

void
nypx_shader_compile(uint32_t id, const char *name)
{
	// For shader hot-recompilations
	GLuint shaders[8];
	GLsizei attach_count;
	glGetAttachedShaders(id, 8, &attach_count, shaders);
	for (int i = 0; i < attach_count; ++i) {
		glDetachShader(id, shaders[i]);
	}

	size_t shsrc_size; // Shader source size in bytes
	char *shsrc;
	char vert_path[256] = { '\0' };
	char frag_path[256] = { '\0' };
	strcpy(frag_path, "assets/shaders/");
	strcat(frag_path, name);
	strcpy(vert_path, frag_path);
	strcat(vert_path, "-vert.glsl");
	strcat(frag_path, "-frag.glsl");

	GLint err;
	GLchar output_log[1024];

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

	if (nyas_file_read(vert_path, &shsrc, &shsrc_size) != NYAS_OK) {
		NYAS_ASSERT(!"Error loading shader vert file.");
	}

	glShaderSource(vert, 1, (const char *const *)&shsrc, NULL);
	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(vert, 1024, NULL, output_log);
		NYAS_LOG_ERR("%s vert:\n%s\n", name, output_log);
	}
	nyas_free(shsrc);

	if (nyas_file_read(frag_path, &shsrc, &shsrc_size) != NYAS_OK) {
		NYAS_ASSERT(!"Error loading shader frag file.");
	}

	glShaderSource(frag, 1, (const char *const *)&shsrc, NULL);
	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(frag, 1024, NULL, output_log);
		NYAS_LOG_ERR("%s frag:\n%s\n", name, output_log);
	}
	nyas_free(shsrc);

	glAttachShader(id, vert);
	glAttachShader(id, frag);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &err);
	if (!err) {
		glGetProgramInfoLog(id, 1024, NULL, output_log);
		NYAS_LOG_ERR("%s program:\n%s\n", name, output_log);
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}

void
nypx_shader_loc(uint32_t id, int *o_loc, const char **i_unif, int count)
{
	for (int i = 0; i < count; ++i) {
		o_loc[i] = glGetUniformLocation(id, i_unif[i]);
	}
}

void
nypx_shader_set_data(int loc, float *data, int v4count)
{
	glUniform4fv(loc, v4count, data);
}

static void
nypx__set_tex(int loc, int *t, int c, int unit, GLenum target)
{
	for (int i = 0; i < c; ++i) {
		glActiveTexture(GL_TEXTURE0 + unit + i);
		glBindTexture(target, t[i]);
		t[i] = unit + i;
	}
	glUniform1iv(loc, c, t);
}

void
nypx_shader_set_tex(int loc, int *tex, int count, int texunit_offset)
{
	nypx__set_tex(loc, tex, count, texunit_offset, GL_TEXTURE_2D);
}

void
nypx_shader_set_cube(int loc, int *tex, int count, int texunit_offset)
{
	nypx__set_tex(loc, tex, count, texunit_offset, GL_TEXTURE_CUBE_MAP);
}

void
nypx_shader_use(uint32_t id)
{
	glUseProgram(id);
}

void
nypx_shader_release(uint32_t id)
{
	glDeleteProgram(id);
}

void
nypx_fb_create(uint32_t *id)
{
	glGenFramebuffers(1, id);
}

int
nypx__fb_attach_gl(int slot)
{
	switch (slot) {
	case NYPX_SLOT_DEPTH:
		return GL_DEPTH_ATTACHMENT;
	case NYPX_SLOT_STENCIL:
		return GL_STENCIL_ATTACHMENT;
	case NYPX_SLOT_DEPTH_STENCIL:
		return GL_DEPTH_ATTACHMENT;
	default:
		return (GL_COLOR_ATTACHMENT0 - NYPX_SLOT_COLOR0) + slot;
	}
}

void
nypx_fb_set(uint32_t id, uint32_t texid, int slot, int level, int face)
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	GLenum target = face >= 0 ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + face :
								GL_TEXTURE_2D;
	slot = nypx__fb_attach_gl(slot);
	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, target, texid, level);
}

void
nypx_fb_use(uint32_t id)
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void
nypx_fb_release(uint32_t *id)
{
	glDeleteFramebuffers(1, id);
}

void
nypx_clear(int color, int depth, int stencil)
{
	GLbitfield mask = 0;
	mask |= (GL_COLOR_BUFFER_BIT * color);
	mask |= (GL_DEPTH_BUFFER_BIT * depth);
	mask |= (GL_STENCIL_BUFFER_BIT * stencil);
	glClear(mask);
}

void
nypx_draw(int elem_count, int index_type)
{
	glDrawElements(GL_TRIANGLES, elem_count,
	               index_type ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);
}

void
nypx_clear_color(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void
nypx_blend_enable(void)
{
	glEnable(GL_BLEND);
}

void
nypx_blend_disable(void)
{
	glDisable(GL_BLEND);
}

static GLenum
nypx__gl_blend(int blend_func)
{
	switch (blend_func) {
	case NYAS_BLEND_ONE:
		return GL_ONE;
	case NYAS_BLEND_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case NYAS_BLEND_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case NYAS_BLEND_ZERO:
		return GL_ZERO;
	default:
		return -1;
	}
}

void
nypx_blend_set(int blend_func_src, int blend_func_dst)
{
	glBlendFunc(nypx__gl_blend(blend_func_src),
	            nypx__gl_blend(blend_func_dst));
}

void
nypx_cull_enable(void)
{
	glEnable(GL_CULL_FACE);
}

void
nypx_cull_disable(void)
{
	glDisable(GL_CULL_FACE);
}

static GLenum
nypx__gl_cull(enum nyas_cull_face cull)
{
	switch (cull) {
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

void
nypx_cull_set(int cull_face)
{
	glCullFace(nypx__gl_cull(cull_face));
}

void
nypx_depth_enable_test(void)
{
	glEnable(GL_DEPTH_TEST);
}

void
nypx_depth_disable_test(void)
{
	glDisable(GL_DEPTH_TEST);
}

void
nypx_depth_enable_mask(void)
{
	glDepthMask(GL_TRUE);
}

void
nypx_depth_disable_mask(void)
{
	glDepthMask(GL_FALSE);
}

static GLenum
nypx__gl_depth(enum nyas_depth_func df)
{
	switch (df) {
	case NYPX_DEPTH_LEQUAL:
		return GL_LEQUAL;
	case NYPX_DEPTH_LESS:
		return GL_LESS;
	default:
		return -1;
	}
}

void
nypx_depth_set(int depth_func)
{
	glDepthFunc(nypx__gl_depth(depth_func));
}

void
nypx_viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}
