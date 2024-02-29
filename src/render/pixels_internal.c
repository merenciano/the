#include "pixels_internal.h"

#include "core/io.h"
#include "core/mem.h" // free --shader source buffer

#include <string.h> // shader path manipulation
#include <glad/glad.h>

static const GLint attrib_sizes[NYAS_VA_COUNT] = { 3, 3, 3, 3, 2 };
static const char *attrib_names[NYAS_VA_COUNT] = {
	"a_position", "a_normal", "a_tangent", "a_bitangent", "a_uv"
};

typedef struct gl_tdesc_t {
	GLenum target;
	GLint min_f;
	GLint mag_f;
	GLint ws;
	GLint wt;
	GLint wr;
	float border[4];
} gl_tdesc_t;

static GLenum
glttarget(nyas_texture_type type)
{
	switch (type) {
	case NYAS_TEX_2D: return GL_TEXTURE_2D;
	case NYAS_TEX_CUBEMAP: return GL_TEXTURE_CUBE_MAP;
	case NYAS_TEX_ARRAY_2D: return GL_TEXTURE_2D_ARRAY;
	case NYAS_TEX_ARRAY_CUBEMAP: return GL_TEXTURE_CUBE_MAP_ARRAY;
	default: return 0;
	}
}

static GLint
gltfilter(nyas_texture_filter f)
{
	switch (f) {
	case NYAS_TEX_FLTR_LINEAR: return GL_LINEAR;
	case NYAS_TEX_FLTR_LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
	case NYAS_TEX_FLTR_LINEAR_MIPMAP_NEAR: return GL_LINEAR_MIPMAP_NEAREST;
	case NYAS_TEX_FLTR_NEAR: return GL_NEAREST;
	case NYAS_TEX_FLTR_NEAR_MIPMAP_NEAR: return GL_NEAREST_MIPMAP_NEAREST;
	case NYAS_TEX_FLTR_NEAR_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
	default: return 0;
	}
}

static GLint
gltwrap(nyas_texture_wrap w)
{
	switch (w) {
	case NYAS_TEX_WRAP_REPEAT: return GL_REPEAT;
	case NYAS_TEX_WRAP_CLAMP: return GL_CLAMP_TO_EDGE;
	case NYAS_TEX_WRAP_MIRROR: return GL_MIRRORED_REPEAT;
	case NYAS_TEX_WRAP_BORDER: return GL_CLAMP_TO_BORDER;
	default: return 0;
	}
}

static gl_tdesc_t
gltdesc(struct nyas_texture_desc *d)
{
	gl_tdesc_t gldesc = {
		.target = glttarget(d->type),
		.min_f = gltfilter(d->min_filter),
		.mag_f = gltfilter(d->mag_filter),
		.ws = gltwrap(d->wrap_s),
		.wt = gltwrap(d->wrap_t),
		.wr = gltwrap(d->wrap_r),
		.border = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
	memcpy(gldesc.border, d->border_color, sizeof(gldesc.border));
	return gldesc;
}

struct gltfmt_result {
	GLint ifmt;
	GLenum fmt;
	GLenum type;
};

static struct gltfmt_result
gltfmt(nyas_texture_format fmt)
{
	switch (fmt) {
	case NYAS_TEX_FMT_R8: return (struct gltfmt_result){ GL_R8, GL_RED, GL_UNSIGNED_BYTE };
	case NYAS_TEX_FMT_RG8: return (struct gltfmt_result){ GL_RG8, GL_RG, GL_UNSIGNED_BYTE };
	case NYAS_TEX_FMT_RGB8: return (struct gltfmt_result){ GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
	case NYAS_TEX_FMT_RGBA8: return (struct gltfmt_result){ GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
	case NYAS_TEX_FMT_SRGB: return (struct gltfmt_result){ GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE };
	case NYAS_TEX_FMT_R16F: return (struct gltfmt_result){ GL_R16F, GL_RED, GL_HALF_FLOAT };
	case NYAS_TEX_FMT_RG16F: return (struct gltfmt_result){ GL_RG16F, GL_RG, GL_HALF_FLOAT };
	case NYAS_TEX_FMT_RGB16F: return (struct gltfmt_result){ GL_RGB16F, GL_RGB, GL_HALF_FLOAT };
	case NYAS_TEX_FMT_RGBA16F: return (struct gltfmt_result){ GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT };
	case NYAS_TEX_FMT_RGB32F: return (struct gltfmt_result){ GL_RGB32F, GL_RGB, GL_FLOAT };
	case NYAS_TEX_FMT_DEPTH:
		return (struct gltfmt_result){ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT };
	default:
		NYAS_LOG_ERR("Unrecognized texture format: (%d).", fmt);
		return (struct gltfmt_result){ 0, 0, 0 };
	}
}

void
nypx_tex_create(struct nyas_texture_internal *t)
{
	gl_tdesc_t d = gltdesc(&t->data);
	glGenTextures(1, &t->res.id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(d.target, t->res.id);
	if (d.min_f) {
		glTexParameteri(d.target, GL_TEXTURE_MIN_FILTER, d.min_f);
	}
	if (d.mag_f) {
		glTexParameteri(d.target, GL_TEXTURE_MAG_FILTER, d.mag_f);
	}
	if (d.ws) {
		glTexParameteri(d.target, GL_TEXTURE_WRAP_S, d.ws);
	}
	if (d.wt) {
		glTexParameteri(d.target, GL_TEXTURE_WRAP_T, d.wt);
	}
	if (d.wr) {
		glTexParameteri(d.target, GL_TEXTURE_WRAP_R, d.wr);
	}
	if (d.border[3] > 0.0f) {
		glTexParameterfv(d.target, GL_TEXTURE_BORDER_COLOR, d.border);
	}
}

void
nypx_tex_set(struct nyas_texture_internal *t)
{
	GLenum type = glttarget(t->data.type);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(type, t->res.id);
	struct gltfmt_result fmt = gltfmt(t->data.fmt);
	for (int i = 0; i < t->img->count; ++i) {
		GLint target = (t->data.type == NYAS_TEX_2D) ?
		  GL_TEXTURE_2D :
		  GL_TEXTURE_CUBE_MAP_POSITIVE_X + t->img->at[i].face;
		int w = t->data.width >> t->img->at[i].lod;
		int h = t->data.height >> t->img->at[i].lod;
		glTexImage2D(target, t->img->at[i].lod, fmt.ifmt, w, h, 0, fmt.fmt, fmt.type, t->img->at[i].pix);
	}

	if (t->data.flags & NYAS_TEX_FLAG_GENERATE_MIPMAPS) {
		glGenerateMipmap(type);
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
nypx_mesh_use(struct nyas_mesh_internal *m, struct nyas_shader_internal *s)
{
	(void)s;
	if (m) {
		glBindVertexArray(m->res.id);
	} else {
		glBindVertexArray(0);
	}
}

static GLsizei
nypx__get_attrib_stride(int32_t attr_flags)
{
	GLsizei stride = 0;
	for (int i = 0; i < NYAS_VA_COUNT; ++i) {
		if (attr_flags & (1 << i)) {
			stride += attrib_sizes[i];
		}
	}
	return stride * sizeof(float);
}

void
nypx_mesh_set(struct nyas_mesh_internal *mesh, uint32_t shader_id)
{
	glBindVertexArray(mesh->res.id);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->res_vb.id);
	glBufferData(GL_ARRAY_BUFFER, mesh->vtx_size, mesh->vtx, GL_STATIC_DRAW);

	GLint offset = 0;
	GLsizei stride = nypx__get_attrib_stride(mesh->attrib);
	for (int i = 0; i < NYAS_VA_COUNT; ++i) {
		if (!(mesh->attrib & (1 << i))) {
			continue;
		}

		GLint size = attrib_sizes[i];
		GLint attrib_pos = glGetAttribLocation(shader_id, attrib_names[i]);
		if (attrib_pos >= 0) {
			glEnableVertexAttribArray(attrib_pos);
			glVertexAttribPointer(
			  attrib_pos, size, GL_FLOAT, GL_FALSE, stride, (void *)(offset * sizeof(float)));
		}
		offset += size;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->res_ib.id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->elem_count * sizeof(nypx_index),
	             (const void *)mesh->idx, GL_STATIC_DRAW);

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
nypx_fb_create(struct nyas_framebuffer_internal *fb)
{
	glGenFramebuffers(1, &fb->res.id);
}

int
nypx__fb_attach_gl(nyas_framebuffer_attach a)
{
	switch (a) {
	case NYAS_ATTACH_DEPTH: return GL_DEPTH_ATTACHMENT;
	case NYAS_ATTACH_STENCIL: return GL_STENCIL_ATTACHMENT;
	case NYAS_ATTACH_DEPTH_STENCIL: return GL_DEPTH_STENCIL_ATTACHMENT;
	default: return GL_COLOR_ATTACHMENT0 + a;
	}
}

void
nypx_fb_set(uint32_t fb_id, uint32_t tex_id, struct nyas_texture_target *tt)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
	GLenum face = tt->face == NYAS_FACE_2D ?
	  GL_TEXTURE_2D :
	  GL_TEXTURE_CUBE_MAP_POSITIVE_X + tt->face;
	GLint slot = nypx__fb_attach_gl(tt->attach);
	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, face, tex_id, tt->lod_level);
}

void
nypx_fb_use(uint32_t id)
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void
nypx_fb_release(struct nyas_framebuffer_internal *fb)
{
	glDeleteFramebuffers(1, &fb->res.id);
}

void
nypx_clear(bool color, bool depth, bool stencil)
{
	GLbitfield mask = 0;
	mask |= (GL_COLOR_BUFFER_BIT * color);
	mask |= (GL_DEPTH_BUFFER_BIT * depth);
	mask |= (GL_STENCIL_BUFFER_BIT * stencil);
	if (mask) {
		glClear(mask);
	}
}

void
nypx_draw(int elem_count, int index_type)
{
	glDrawElements(GL_TRIANGLES, elem_count, index_type ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);
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
nypx__gl_blend(int blend_func)
{
	switch (blend_func) {
	case NYAS_BLEND_CURRENT: return 0xFFFF;
	case NYAS_BLEND_ONE: return GL_ONE;
	case NYAS_BLEND_SRC_ALPHA: return GL_SRC_ALPHA;
	case NYAS_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
	case NYAS_BLEND_ZERO: return GL_ZERO;
	default: NYAS_ASSERT(false);
	}
}

void
nypx_blend_set(int blend_func_src, int blend_func_dst)
{
	GLenum gl_src = nypx__gl_blend(blend_func_src);
	if (gl_src != 0xFFFF) {
		glBlendFunc(gl_src, nypx__gl_blend(blend_func_dst));
	}
}

static GLenum
nypx__gl_cull(enum nyas_cull_face cull)
{
	switch (cull) {
	case NYAS_CULL_BACK: return GL_BACK;
	case NYAS_CULL_FRONT: return GL_FRONT;
	case NYAS_CULL_FRONT_AND_BACK: return GL_FRONT_AND_BACK;
	case NYAS_CULL_CURRENT: return 0;
	default: return NYAS_ERR_SWITCH_DEFAULT;
	}
}

void
nypx_cull_set(int cull_face)
{
	GLenum gl_value = nypx__gl_cull(cull_face);
	if (gl_value) {
		glCullFace(nypx__gl_cull(cull_face));
	}
}

static GLenum
nypx__gl_depth(enum nyas_depth_func df)
{
	switch (df) {
	case NYAS_DEPTH_LEQUAL: return GL_LEQUAL;
	case NYAS_DEPTH_LESS: return GL_LESS;
	case NYAS_DEPTH_CURRENT: return 0;
	default: return NYAS_ERR_SWITCH_DEFAULT;
	}
}

void
nypx_depth_set(int depth_func)
{
	GLenum gl_value = nypx__gl_depth(depth_func);
	if (gl_value) {
		glDepthFunc(gl_value);
	}
}

void
nypx_stencil_enable_test(void)
{
	glEnable(GL_STENCIL);
}

void
nypx_stencil_disable_test(void)
{
	glDisable(GL_STENCIL);
}

void
nypx_stencil_enable_mask(void)
{
	glStencilMask(GL_TRUE);
}

void
nypx_stencil_disable_mask(void)
{
	glStencilMask(GL_FALSE);
}

void
nypx_scissor_enable(void)
{
	glEnable(GL_SCISSOR_TEST);
}

void
nypx_scissor_disable(void)
{
	glDisable(GL_SCISSOR_TEST);
}

void
nypx_viewport(struct nyas_rect rect)
{
	if (rect.w) {
		glViewport(rect.x, rect.y, rect.w, rect.h);
	}
}

void
nypx_scissor(struct nyas_rect rect)
{
	if (rect.w) {
		glScissor(rect.x, rect.y, rect.w, rect.h);
	}
}
