#include "nyaspix.h"

#include "core/io.h" // file_read --shader source
#include "core/log.h"
#include "core/mem.h" // free --shader source buffer

#include <glad/glad.h>
#include <string.h> // shader path manipulation

struct texcnfg {
	GLenum target;
	GLint ifmt;
	GLenum fmt;
	GLenum type;
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
			.ifmt = GL_DEPTH_COMPONENT,
			.fmt = GL_DEPTH_COMPONENT,
			.type = GL_FLOAT,
			.wrap = GL_CLAMP_TO_BORDER,
			.min = GL_LINEAR,
			.mag = GL_LINEAR,
		};

		return depthcnfg;
	}

	struct texcnfg cnfg = {
		.target = flags & TF_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D,
		.ifmt = flags & TF_LINEAR_COLOR ? GL_RGB8 : GL_SRGB8,
		.fmt = GL_RGB,
		.type = GL_UNSIGNED_BYTE,
		.wrap = flags & TF_TILING ? GL_REPEAT : GL_CLAMP_TO_EDGE,
		.min = flags & TF_MIN_FILTER_LERP ? GL_LINEAR : GL_NEAREST,
		.mag = flags & TF_MAG_FILTER_LERP ? GL_LINEAR : GL_NEAREST
	};

	switch (flags & 0x3) {
	case 0:
		cnfg.ifmt = GL_R8;
		cnfg.fmt = GL_RED;
		break;

	case 1:
		cnfg.ifmt = GL_RG8;
		cnfg.fmt = GL_RG;
		break;

	case 3:
		cnfg.ifmt = GL_RGBA8;
		cnfg.fmt = GL_RGBA;
		break;
	}

	if (flags & TF_FLOAT) {
		cnfg.type = GL_FLOAT;
		switch (flags & 0x3) {
		case 0:
			cnfg.ifmt = GL_R16F;
			break;

		case 1:
			cnfg.ifmt = GL_RG16F;
			break;

		case 2:
			cnfg.ifmt = GL_RGB16F;
			break;

		case 3:
			cnfg.ifmt = GL_RGBA16F;
			break;
		}
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

	if (type & TF_CUBE) { // TODO: Check if WrapR can be set for Tex2D
		glTexParameteri(cnfg.target, GL_TEXTURE_WRAP_R, cnfg.wrap);
	} else if (type & TF_DEPTH) {
		float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(cnfg.target, GL_TEXTURE_BORDER_COLOR, border);
	}
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
		glTexImage2D(target + i, 0, cnfg.ifmt, width, height, 0, cnfg.fmt,
		             cnfg.type, pix[i]);
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
	glGenVertexArrays(1, id);
	glGenBuffers(1, vid);
	glGenBuffers(1, iid);
}

void
nypx_mesh_use(uint32_t id)
{
	glBindVertexArray(id);
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
	glBindVertexArray(id);
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iid);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements * sizeof(nypx_index),
	             (const void *)idx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
nypx_mesh_release(uint32_t *id, uint32_t *vid, uint32_t *iid)
{
	glDeleteVertexArrays(1, id);
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
	/*s->loc[0].data = glGetUniformLocation(id, "u_data");
	s->loc[0].tex = glGetUniformLocation(id, "u_tex");
	s->loc[0].cubemap = glGetUniformLocation(id, "u_cube");
	s->loc[1].data = glGetUniformLocation(id, "u_common_data");
	s->loc[1].tex = glGetUniformLocation(id, "u_common_tex");
	s->loc[1].cubemap = glGetUniformLocation(id, "u_common_cube");*/
}

void
nypx_shader_set_data(int loc, float *data, int v4count)
{
	glUniform4fv(loc, v4count, data);
}

void
nypx_shader_set_tex(int loc, int *tex, int count, int texunit_offset)
{
	for (int i = 0; i < count; ++i) {
		glActiveTexture(GL_TEXTURE0 + texunit_offset + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		tex[i] = texunit_offset + i;
	}

	glUniform1iv(loc, count, tex);
}

void
nypx_shader_set_cube(int loc, int *tex, int count, int texunit_offset)
{
	for (int i = 0; i < count; ++i) {
		glActiveTexture(GL_TEXTURE0 + texunit_offset + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[i]);
		tex[i] = texunit_offset + i;
	}

	glUniform1iv(loc, count, tex);
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
		return GL_DEPTH_STENCIL_ATTACHMENT;
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
nypx_draw(int elem_count, int half_type)
{
	glDrawElements(GL_TRIANGLES, elem_count,
	               half_type ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 0);
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
nypx__gl_blend(enum nypx_blend_func bf)
{
	switch (bf) {
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

void
nypx_blend_set(enum nypx_blend_func src, enum nypx_blend_func dst)
{
	glBlendFunc(nypx__gl_blend(src), nypx__gl_blend(dst));
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
nypx__gl_cull(enum nypx_cull_face cull)
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
nypx_cull_set(enum nypx_cull_face cull)
{
	glCullFace(nypx__gl_cull(cull));
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
nypx__gl_depth(enum nypx_depth_func df)
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
nypx_depth_set(enum nypx_depth_func depth)
{
	glDepthFunc(nypx__gl_depth(depth));
}

void
nypx_viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}
