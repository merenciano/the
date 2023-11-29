#include "pixels_internal.h"

#include "core/io.h"
#include "core/log.h"
#include "core/mem.h"

#include <string.h>

#define NYAS_OPENGL // Hardcoded until another backend is implemented.
#ifdef NYAS_OPENGL
#include <glad/glad.h>

typedef struct {
	GLint internal_format;
	GLenum format;
	GLenum type;
	GLint wrap;
	GLint min_filter;
	GLint mag_filter;
} nyas_texcube_cnfg;

typedef struct {
	GLint internal_format;
	GLenum format;
	GLenum type;
	GLint wrap;
	GLint filter;
	int channels;
} nyas_tex_cnfg;

/*
  Attribute's number of elements for each vertex.
  The array's position must match with the
  enum (nyas_VertexAttributes) value of the attribute.
*/
static const GLint attrib_sizes[VERTEX_ATTRIBUTE_COUNT] = { 3, 3, 3, 3, 2 };

/*
  Attribute's layout name in the shader.
  The array's position must match with the attribute's
  value at enum nyas_VertexAttributes.
*/
static const char *attrib_names[VERTEX_ATTRIBUTE_COUNT] = {
	"a_position", "a_normal", "a_tangent", "a_bitangent", "a_uv"
};

static bool
nypx__resource_check(void *rsrc)
{
	return rsrc && ((r_resource *)rsrc)->id >= 0;
}

static bool
nyas__is_dirty(void *resource)
{
	r_resource *r = resource;
	return r->flags & RF_DIRTY;
}

static void
nyas__enable_render_opt(int opt)
{
	switch (opt) {
	case NYAS_BLEND:
		glEnable(GL_BLEND);
		break;
	case NYAS_CULL_FACE:
		glEnable(GL_CULL_FACE);
		break;
	case NYAS_DEPTH_TEST:
		glEnable(GL_DEPTH_TEST);
		break;
	case NYAS_DEPTH_WRITE:
		glDepthMask(GL_TRUE);
		break;
	default:
		break;
	}
}

static void
nyas__disable_render_opt(int opt)
{
	switch (opt) {
	case NYAS_BLEND:
		glDisable(GL_BLEND);
		break;
	case NYAS_CULL_FACE:
		glDisable(GL_CULL_FACE);
		break;
	case NYAS_DEPTH_TEST:
		glDisable(GL_DEPTH_TEST);
		break;
	case NYAS_DEPTH_WRITE:
		glDepthMask(GL_FALSE);
		break;
	default:
		break;
	}
}

static GLenum
nyas__gl_blend(enum nyas_blendfn_opt bf)
{
	switch (bf) {
	case NYAS_BLEND_FUNC_ONE:
		return GL_ONE;
	case NYAS_BLEND_FUNC_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case NYAS_BLEND_FUNC_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case NYAS_BLEND_FUNC_ZERO:
		return GL_ZERO;
	default:
		return -1;
	}
}

static GLenum
nyas__gl_cull(nyas_cullface_opt cull)
{
	switch (cull) {
	case NYAS_CULL_FACE_BACK:
		return GL_BACK;
	case NYAS_CULL_FACE_FRONT:
		return GL_FRONT;
	case NYAS_CULL_FACE_FRONT_AND_BACK:
		return GL_FRONT_AND_BACK;
	default:
		return -1;
	}
}

static GLenum
nyas__gl_depth(nyas_depthfn_opt df)
{
	switch (df) {
	case NYAS_DEPTH_FUNC_LEQUAL:
		return GL_LEQUAL;
	case NYAS_DEPTH_FUNC_LESS:
		return GL_LESS;
	default:
		return -1;
	}
}

static void
nyas__attach_to_fb(nyas_fbattach a)
{
	int id = ((r_tex *)nyas_arr_at(tex_pool, a.tex))->res.id;
	GLenum target = a.side < 0 ? GL_TEXTURE_2D :
								 GL_TEXTURE_CUBE_MAP_POSITIVE_X + a.side;

	GLenum slot = a.slot == NYAS_ATTACH_COLOR ? GL_COLOR_ATTACHMENT0 :
												GL_DEPTH_ATTACHMENT;

	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, target, id, a.level);
}

static void
nyas__set_viewport_from_fb(nyas_framebuffer fb)
{
	int w, h;
	nyas_fb_size(fb, &w, &h);
	glViewport(0, 0, w, h);
}

static void
nyas__set_viewport(int x, int y)
{
	if (x < 0) {
		nyas_v2i win_size = nyas_window_size();
		glViewport(0, 0, win_size.x, win_size.y);
	} else {
		glViewport(0, 0, x, y);
	}
}

static GLsizei
nyas__get_attrib_stride(int32_t attr_flags)
{
	GLsizei stride = 0;
	for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
		if (attr_flags & (1 << i)) {
			stride += attrib_sizes[i];
		}
	}
	return stride * sizeof(float);
}

static void
nyas__create_mesh(nyas_mesh mesh, nyas_shader shader)
{
	r_mesh *m = nyas_arr_at(mesh_pool, mesh);
	r_shader *s = nyas_arr_at(shader_pool, shader);

	glGenVertexArrays(1, (GLuint *)&m->res.id);
	glBindVertexArray(m->res.id);
	glGenBuffers(2, m->internal_buffers_id);

	glBindBuffer(GL_ARRAY_BUFFER, m->internal_buffers_id[0]);
	glBufferData(GL_ARRAY_BUFFER, m->vtx_size, m->vtx, GL_STATIC_DRAW);

	GLint offset = 0;
	GLsizei stride = nyas__get_attrib_stride(m->attr_flags);
	for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
		if (!(m->attr_flags & (1 << i))) {
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->internal_buffers_id[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->elements * sizeof(IDX_T),
	             (const void *)m->idx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (m->res.flags & RF_FREE_AFTER_LOAD) {
		nyas_free(m->vtx);
		m->vtx = NULL;
		nyas_free(m->idx);
		m->idx = NULL;
	}
}

static void
nyas__create_cubemap(nyas_tex tex)
{
	nyas_texcube_cnfg config;
	r_tex *t = nyas_arr_at(tex_pool, tex);

	NYAS_ASSERT(tex < 62 && "Start thinking about the max textures");
	NYAS_ASSERT(t->res.id == NYAS_UNINIT &&
	            "Texture already created in the gpu");

	switch (t->type) {
	case NYAS_TEX_SKYBOX:
		config.format = GL_RGB;
		config.internal_format = GL_SRGB;
		config.type = GL_UNSIGNED_BYTE;
		config.min_filter = GL_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	case NYAS_TEX_ENVIRONMENT:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.min_filter = GL_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	case NYAS_TEX_PREFILTER_ENVIRONMENT:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.min_filter = GL_LINEAR_MIPMAP_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	default:
		NYAS_LOG_ERR("Trying to create a cubemap with an invalid format");
		return;
	}

	glGenTextures(1, (GLuint *)&(t->res.id));
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, t->res.id);

	if (t->type == NYAS_TEX_SKYBOX) {
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			             config.internal_format, t->width, t->height, 0,
			             config.format, config.type, t->pix[i]);

			if (t->res.flags & RF_FREE_AFTER_LOAD) {
				nyas_free(t->pix[i]);
				t->pix[i] = NULL;
			}
		}
	} else {
		NYAS_ASSERT(t->width > 0 && t->height > 0 && "Invalid size");
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			             config.internal_format, t->width, t->height, 0,
			             config.format, config.type, 0);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
	                config.min_filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
	                config.mag_filter);
	if (t->type == NYAS_TEX_PREFILTER_ENVIRONMENT) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	t->res.flags = 0;
}

static void
nyas__create_texture(nyas_tex tex)
{
	nyas_tex_cnfg config;
	r_tex *t = nyas_arr_at(tex_pool, tex);

	NYAS_ASSERT(tex < 62 && "Max texture units"); // Tex unit is id + 1
	NYAS_ASSERT(t->res.id == NYAS_UNINIT && "Texture already created on GPU");

	switch (t->type) {
	case NYAS_TEX_R:
		config.format = GL_RED;
		config.internal_format = GL_R8;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 1;
		break;

	case NYAS_TEX_LUT:
		config.format = GL_RG;
		config.internal_format = GL_RG16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 2;
		break;

	case NYAS_TEX_RGB:
		config.format = GL_RGB;
		config.internal_format = GL_RGB;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 3;
		break;

	case NYAS_TEX_SRGB:
		config.format = GL_RGB;
		config.internal_format = GL_SRGB;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 3;
		break;

	case NYAS_TEX_RGBA_F16:
		config.format = GL_RGBA;
		config.internal_format = GL_RGBA16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 4;
		break;

	case NYAS_TEX_DEPTH:
		config.format = GL_DEPTH_COMPONENT;
		config.internal_format = GL_DEPTH_COMPONENT;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_BORDER;
		config.channels = 1;
		break;

	case NYAS_TEX_RGB_F16:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 3;
		break;

	case NYAS_TEX_SKYBOX:
	case NYAS_TEX_ENVIRONMENT:
	case NYAS_TEX_PREFILTER_ENVIRONMENT:
		nyas__create_cubemap(tex);
		return;

	default:
		NYAS_LOG_ERR("Invalid format");
		config.format = GL_INVALID_ENUM;
		config.internal_format = GL_INVALID_ENUM;
		config.type = GL_INVALID_ENUM;
		config.filter = GL_INVALID_ENUM;
		config.wrap = GL_INVALID_ENUM;
		config.channels = 0;
		break;
	}

	glGenTextures(1, (GLuint *)&(t->res.id));
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, t->res.id);
	glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width, t->height,
	             0, config.format, config.type, t->pix[0]);
	if (t->pix[0]) {
		if (t->type == GL_UNSIGNED_BYTE) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (t->res.flags & RF_FREE_AFTER_LOAD) {
			nyas_free(t->pix[0]);
			t->pix[0] = NULL;
		}
	}
	// No shadows outside shadow maps
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap);
	t->res.flags = 0;
}

static enum nyas_defs
nyas__load_text_file(const char *path, char *buffer, size_t buffsize)
{
	FILE *fp = fopen(path, "r");

	if (!fp) {
		NYAS_LOG_ERR("File %s couldn't be opened.", path);
		return NYAS_ERR_FILE;
	}

	fread((void *)buffer, 1, buffsize - 1, fp);
	fclose(fp);

	if (buffer[buffsize - 1] != '\0') {
		NYAS_LOG_ERR("File %s bigger than buffer size.", path);
		return NYAS_ERR_ALLOC;
	}

	return NYAS_OK;
}

static enum nyas_defs
nyas__compile_program(r_shader *shader)
{
#define SHADER_BUFFSIZE 8192
	char vert[SHADER_BUFFSIZE] = { '\0' };
	char frag[SHADER_BUFFSIZE] = { '\0' };
	char vert_path[256] = { '\0' };
	char frag_path[256] = { '\0' };
	strcpy(frag_path, "assets/shaders/");
	strcat(frag_path, shader->shader_name);
	strcpy(vert_path, frag_path);
	strcat(vert_path, "-vert.glsl");
	strcat(frag_path, "-frag.glsl");

	if (nyas__load_text_file(vert_path, vert, SHADER_BUFFSIZE) != NYAS_OK) {
		return NYAS_ERR_FILE;
	}

	if (nyas__load_text_file(frag_path, frag, SHADER_BUFFSIZE) != NYAS_OK) {
		if (frag[SHADER_BUFFSIZE - 1] != '\0') {
			NYAS_LOG_ERR("File %s bigger than buffer size.", frag_path);
		}
		return NYAS_ERR_FILE;
	}
#undef SHADER_BUFFSIZE

	GLint err;
	GLchar output_log[512];
	const GLchar *shader_src = &vert[0];
	glShaderSource(shader->vert, 1, &shader_src, NULL);
	glCompileShader(shader->vert);
	glGetShaderiv(shader->vert, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(shader->vert, 512, NULL, output_log);
		NYAS_LOG_ERR("%s vertex shader compilation failed:\n%s\n",
		             shader->shader_name, output_log);
		return NYAS_EC_FAIL;
	}
	shader_src = &frag[0];
	glShaderSource(shader->frag, 1, &shader_src, NULL);
	glCompileShader(shader->frag);
	glGetShaderiv(shader->frag, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(shader->frag, 512, NULL, output_log);
		NYAS_LOG_ERR("%s fragment shader compilation failed:\n%s\n",
		             shader->shader_name, output_log);
		return NYAS_EC_FAIL;
	}
	glAttachShader(shader->res.id, shader->vert);
	glAttachShader(shader->res.id, shader->frag);
	glLinkProgram(shader->res.id);
	glGetProgramiv(shader->res.id, GL_LINK_STATUS, &err);
	if (!err) {
		glGetProgramInfoLog(shader->res.id, 512, NULL, output_log);
		NYAS_LOG_ERR("%s program error:\n%s\n", shader->shader_name,
		             output_log);
		return NYAS_EC_FAIL;
	}

	shader->data_loc[SHADATA_COMMON].data =
	  glGetUniformLocation(shader->res.id, "u_common_data");
	shader->data_loc[SHADATA_COMMON].tex =
	  glGetUniformLocation(shader->res.id, "u_common_tex");
	shader->data_loc[SHADATA_COMMON].cubemap =
	  glGetUniformLocation(shader->res.id, "u_common_cube");
	shader->data_loc[SHADATA_UNIT].data = glGetUniformLocation(shader->res.id,
	                                                           "u_data");
	shader->data_loc[SHADATA_UNIT].tex = glGetUniformLocation(shader->res.id,
	                                                          "u_tex");
	shader->data_loc[SHADATA_UNIT].cubemap =
	  glGetUniformLocation(shader->res.id, "u_cube");

	return NYAS_OK;
}

static enum nyas_defs
nyas__create_shader(r_shader *shader)
{
	NYAS_ASSERT(shader->res.id == NYAS_UNINIT &&
	            "Shader must be uninitialized.");
	NYAS_ASSERT(shader->vert == NYAS_UNINIT &&
	            "Vert shader must be uninitialized.");
	NYAS_ASSERT(shader->frag == NYAS_UNINIT &&
	            "Frag shader must be uninitialized.");
	NYAS_ASSERT(shader->shader_name && "Empty shader name");

	shader->vert = glCreateShader(GL_VERTEX_SHADER);
	shader->frag = glCreateShader(GL_FRAGMENT_SHADER);
	shader->res.id = glCreateProgram();

	nypx__resource_check(shader);
	nypx__resource_check(&shader->vert);
	nypx__resource_check(&shader->frag);

	return nyas__compile_program(shader);
}

static void
nyas__sync_gpu_mesh(nyas_mesh m, nyas_shader s)
{
	r_mesh *im = nyas_arr_at(mesh_pool, m);
	if (im->res.id == NYAS_UNINIT) {
		nyas__create_mesh(m, s);
	}
	im->res.flags = 0;
}

static r_tex *
nyas__sync_gpu_tex(nyas_tex tex)
{
	CHECK_HANDLE(tex, tex);
	r_tex *mutated = NULL;
	r_tex *itex = nyas_arr_at(tex_pool, tex);
	if (itex->res.id == NYAS_UNINIT) {
		nyas__create_texture(tex);
		mutated = itex;
	}
	NYAS_ASSERT(nypx__resource_check(itex) && "Invalid internal resource.");
	return mutated;
}

static void
nyas__sync_gpu_shader(r_shader *is)
{
	if (is->res.id == NYAS_UNINIT) {
		nyas__create_shader(is);
	} else if (is->res.flags & RF_DIRTY) {
		nyas__compile_program(is);
	}
	is->res.flags = 0;
	NYAS_ASSERT(nypx__resource_check(is) && "Error creating internal shader.");
}

static void
nyas__sync_gpu_fb(nyas_framebuffer fb, const nyas_fbattach *atta)
{
	NYAS_ASSERT(atta && "Null ptr arg.");
	r_fb *ifb = nyas_arr_at(framebuffer_pool, fb);
	if (ifb->res.id == NYAS_UNINIT) {
		glGenFramebuffers(1, (GLuint *)&(ifb->res.id));
	}
	NYAS_ASSERT(nypx__resource_check(ifb) && "Invalid internal resource.");
	glBindFramebuffer(GL_FRAMEBUFFER, ifb->res.id);
	if (atta->slot == NYAS_ATTACH_DEPTH) {
		// Change depth texture
		CHECK_HANDLE(tex, atta->tex);
		ifb->depth_tex = atta->tex;
		nyas__sync_gpu_tex(ifb->depth_tex);
		nyas__attach_to_fb(*atta);
	} else if (atta->slot == NYAS_ATTACH_COLOR) {
		// Change color texture
		CHECK_HANDLE(tex, atta->tex);
		ifb->color_tex = atta->tex;
		nyas__sync_gpu_tex(ifb->color_tex);
		nyas__attach_to_fb(*atta);
	} else {
		if (ifb->depth_tex != NYAS_INACTIVE) {
			nyas__sync_gpu_tex(ifb->depth_tex);
			nyas_fbattach a = { .tex = ifb->depth_tex,
				                .slot = NYAS_ATTACH_DEPTH,
				                .side = -1,
				                .level = 0 };
			nyas__attach_to_fb(a);
		}

		if (ifb->color_tex != NYAS_INACTIVE) {
			nyas__sync_gpu_tex(ifb->color_tex);
			nyas_fbattach a = { .tex = ifb->color_tex,
				                .slot = NYAS_ATTACH_COLOR,
				                .side = -1,
				                .level = 0 };
			nyas__attach_to_fb(a);
		}
	}
	ifb->res.flags = 0;
}

static void
nyas__set_shader_data(nyas_mat m, enum shadata_type group)
{
	r_shader *s = nyas_arr_at(shader_pool, m.shader);
	NYAS_ASSERT(nypx__resource_check(s) && "Invalid internal shader.");
	nyas_tex *t = (nyas_tex *)((float *)m.ptr) + m.data_count;
	for (int i = 0; i < m.tex_count + m.cube_count; ++i) {
		r_tex *itx = nyas_arr_at(tex_pool, t[i]);
		if (nyas__is_dirty(itx)) {
			nyas__sync_gpu_tex(t[i]);
		}
	}

	glUniform4fv(s->data_loc[group].data, m.data_count / 4, m.ptr);
	glUniform1iv(s->data_loc[group].tex, m.tex_count, t);
	glUniform1iv(s->data_loc[group].cubemap, m.cube_count, t + m.tex_count);
}

void
nyas_setshader_fn(nyas_cmdata *data)
{
	CHECK_HANDLE(shader, data->mat.shader);
	r_shader *s = nyas_arr_at(shader_pool, data->mat.shader);
	if (nyas__is_dirty(s)) {
		nyas__sync_gpu_shader(s);
	}
	glUseProgram(s->res.id);
	nyas__set_shader_data(data->mat, SHADATA_COMMON);
}

void
nyas_clear_fn(nyas_cmdata *data)
{
	GLbitfield mask = 0;
	if (data->clear.color_buffer) {
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (data->clear.depth_buffer) {
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (data->clear.stencil_buffer) {
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	glClearColor(data->clear.color[0], data->clear.color[1],
	             data->clear.color[2], data->clear.color[3]);
	glClear(mask);
}

void
nyas_draw_fn(nyas_cmdata *data)
{
	nyas_mesh mesh = data->draw.mesh;
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	CHECK_HANDLE(mesh, mesh);
	NYAS_ASSERT(imsh->elements && "Attempt to draw an uninitialized mesh");

	if (nyas__is_dirty(imsh)) {
		nyas__sync_gpu_mesh(mesh, data->draw.material.shader);
	}

	glBindVertexArray(imsh->res.id);
	nyas__set_shader_data(data->draw.material, SHADATA_UNIT);
	glDrawElements(GL_TRIANGLES, imsh->elements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void
nyas_rops_fn(nyas_cmdata *data)
{
	int enable = data->rend_opts.enable_flags;
	int disable = data->rend_opts.disable_flags;
	for (int i = 0; i < NYAS_REND_OPTS_COUNT; ++i) {
		/* Prioritizing disable for bad configs (both on). */
		if (disable & (1 << i)) {
			nyas__disable_render_opt(1 << i);
		} else if (enable & (1 << i)) {
			nyas__enable_render_opt(1 << i);
		}
	}

	nyas_blend_fn blend = data->rend_opts.blend_func;
	/* Ignore unless both have a value assigned. */
	if (blend.src && blend.dst) {
		glBlendFunc(nyas__gl_blend(blend.src), nyas__gl_blend(blend.dst));
	}

	nyas_cullface_opt cull = data->rend_opts.cull_face;
	if (cull) {
		glCullFace(nyas__gl_cull(cull));
	}

	nyas_depthfn_opt depth = data->rend_opts.depth_func;
	if (depth) {
		glDepthFunc(nyas__gl_depth(depth));
	}
}

void
nyas_setfb_fn(nyas_cmdata *data)
{
	NYAS_ASSERT(data && "Null param");
	const nyas_set_fb_cmdata *d = &data->set_fb;
	if (d->fb == NYAS_DEFAULT) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		nyas__set_viewport(-1, -1);
		return;
	}

	r_fb *ifb = nyas_arr_at(framebuffer_pool, d->fb);
	if (d->attachment.slot != NYAS_IGNORE) {
		ifb->res.flags |= RF_DIRTY;
	}

	CHECK_HANDLE(framebuffer, d->fb);
	if (nyas__is_dirty(ifb)) {
		nyas__sync_gpu_fb(d->fb, &d->attachment);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, ifb->res.id);
	}
	if (d->vp_x > 0) {
		nyas__set_viewport(d->vp_x, d->vp_y);
	} else {
		nyas__set_viewport_from_fb(d->fb);
	}
}

#endif // NYAS_OPENGL
