#include "rendercommands.h"

#include "core/io.h"
#include "core/mem.h"
#include "internalresources.h"
#include "renderer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <mathc.h>
#define THE_OPENGL // Hardcoded until another backend is implemented.
#ifdef THE_OPENGL
#include <glad/glad.h>

typedef struct {
	GLenum internal_format;
	GLenum format;
	GLenum type;
	GLenum wrap;
	GLenum min_filter;
	GLenum mag_filter;
} THE_CubeConfig;

typedef struct {
	GLenum internal_format;
	GLenum format;
	GLenum type;
	GLenum wrap;
	GLenum filter;
	int32_t channels;
} THE_TextureConfig;

/*
  Attribute's number of elements for each vertex.
  The array's position must match with the
  enum (THE_VertexAttributes) value of the attribute.
*/
static const GLint attrib_sizes[VERTEX_ATTRIBUTE_COUNT] = { 3, 3, 3, 3, 2 };

/*
  Attribute's layout name in the shader.
  The array's position must match with the attribute's
  value at enum THE_VertexAttributes.
*/
static const char *attrib_names[VERTEX_ATTRIBUTE_COUNT] = {
	"a_position", "a_normal", "a_tangent", "a_bitangent", "a_uv"
};

static bool
the__is_dirty(void *resource)
{
	THE_InternalResource *r = resource;
	return r->flags & RF_DIRTY;
}

static void
the__enable_render_opt(int opt)
{
	switch (opt) {
	case THE_BLEND:
		glEnable(GL_BLEND);
		break;
	case THE_CULL_FACE:
		glEnable(GL_CULL_FACE);
		break;
	case THE_DEPTH_TEST:
		glEnable(GL_DEPTH_TEST);
		break;
	case THE_DEPTH_WRITE:
		glDepthMask(GL_TRUE);
		break;
	default:
		break;
	}
}

static void
the__disable_render_opt(int opt)
{
	switch (opt) {
	case THE_BLEND:
		glDisable(GL_BLEND);
		break;
	case THE_CULL_FACE:
		glDisable(GL_CULL_FACE);
		break;
	case THE_DEPTH_TEST:
		glDisable(GL_DEPTH_TEST);
		break;
	case THE_DEPTH_WRITE:
		glDepthMask(GL_FALSE);
		break;
	default:
		break;
	}
}

static GLenum
the__gl_blend(enum THE_BlendFuncOpt bf)
{
	switch (bf) {
	case THE_BLEND_FUNC_ONE:
		return GL_ONE;
	case THE_BLEND_FUNC_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case THE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case THE_BLEND_FUNC_ZERO:
		return GL_ZERO;
	default:
		return -1;
	}
}

static GLenum
the__gl_cull(THE_CullFace cull)
{
	switch (cull) {
	case THE_CULL_FACE_BACK:
		return GL_BACK;
	case THE_CULL_FACE_FRONT:
		return GL_FRONT;
	case THE_CULL_FACE_FRONT_AND_BACK:
		return GL_FRONT_AND_BACK;
	default:
		return -1;
	}
}

static GLenum
the__gl_depth(THE_DepthFunc df)
{
	switch (df) {
	case THE_DEPTH_FUNC_LEQUAL:
		return GL_LEQUAL;
	case THE_DEPTH_FUNC_LESS:
		return GL_LESS;
	default:
		return -1;
	}
}

static void
the__attach_to_fb(THE_FBAttachment a)
{
	int id = textures[a.tex].res.id;
	GLenum target = a.side < 0 ? GL_TEXTURE_2D :
								 GL_TEXTURE_CUBE_MAP_POSITIVE_X + a.side;

	GLenum slot = a.slot == THE_ATTACH_COLOR ? GL_COLOR_ATTACHMENT0 :
											   GL_DEPTH_ATTACHMENT;

	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, target, id, a.level);
}

static void
the__set_viewport_from_fb(THE_Framebuffer fb)
{
	int w, h;
	THE_FbDimensions(fb, &w, &h);
	glViewport(0, 0, w, h);
}

static void the__set_viewport(int x, int y)
{
	if (x < 0) {
		int win_size[2];
		THE_WindowSize(win_size);
		glViewport(0, 0, win_size[0], win_size[1]);
	} else {
		glViewport(0, 0, x, y);
	}
}

static GLsizei
the__get_attrib_stride(int32_t attr_flags)
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
the__create_mesh(THE_Mesh mesh, THE_Shader shader)
{
	THE_InternalMesh *m = meshes + mesh;
	THE_InternalShader *s = shaders + shader;

	glGenVertexArrays(1, (GLuint *)&m->res.id);
	glBindVertexArray(m->res.id);
	glGenBuffers(2, m->internal_buffers_id);

	glBindBuffer(GL_ARRAY_BUFFER, m->internal_buffers_id[0]);
	glBufferData(GL_ARRAY_BUFFER, m->vtx_size, m->vtx, GL_STATIC_DRAW);

	GLint offset = 0;
	GLsizei stride = the__get_attrib_stride(m->attr_flags);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->elements * sizeof(uint32_t),
	             (const void *)m->idx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (m->res.flags & RF_FREE_AFTER_LOAD) {
		THE_Free(m->vtx);
		m->vtx = NULL;
		THE_Free(m->idx);
		m->idx = NULL;
	}
}

static void
the__create_cubemap(THE_Texture tex)
{
	THE_CubeConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(tex < 62, "Start thinking about the max textures");
	THE_ASSERT(t->res.id == THE_UNINIT, "Texture already created in the gpu");

	switch (t->type) {
	case THE_TEX_SKYBOX:
		config.format = GL_RGB;
		config.internal_format = GL_SRGB;
		config.type = GL_UNSIGNED_BYTE;
		config.min_filter = GL_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	case THE_TEX_ENVIRONMENT:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.min_filter = GL_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	case THE_TEX_PREFILTER_ENVIRONMENT:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.min_filter = GL_LINEAR_MIPMAP_LINEAR;
		config.mag_filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		break;

	default:
		THE_SLOG_ERROR("Trying to create a cubemap with an invalid format");
		return;
	}

	glGenTextures(1, (GLuint *)&(t->res.id));
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, t->res.id);

	if (t->type == THE_TEX_SKYBOX) {
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			             config.internal_format, t->width, t->height, 0,
			             config.format, config.type, t->pix[i]);

			if (t->res.flags & RF_FREE_AFTER_LOAD) {
				THE_Free(t->pix[i]);
				t->pix[i] = NULL;
			}
		}
	} else {
		THE_ASSERT(t->width > 0 && t->height > 0,
		           "The texture have to have size for the empty environment");
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
	if (t->type == THE_TEX_PREFILTER_ENVIRONMENT) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	t->res.flags = 0;
}

static void
the__create_texture(THE_Texture tex)
{
	THE_TextureConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(tex < 62, "Max texture units"); // Tex unit is id + 1
	THE_ASSERT(t->res.id == THE_UNINIT, "Texture already created on GPU");

	switch (t->type) {
	case THE_TEX_R:
		config.format = GL_RED;
		config.internal_format = GL_R8;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 1;
		break;

	case THE_TEX_LUT:
		config.format = GL_RG;
		config.internal_format = GL_RG16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 2;
		break;

	case THE_TEX_RGB:
		config.format = GL_RGB;
		config.internal_format = GL_RGB;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 3;
		break;

	case THE_TEX_SRGB:
		config.format = GL_RGB;
		config.internal_format = GL_SRGB;
		config.type = GL_UNSIGNED_BYTE;
		config.filter = GL_LINEAR;
		config.wrap = GL_REPEAT;
		config.channels = 3;
		break;

	case THE_TEX_RGBA_F16:
		config.format = GL_RGBA;
		config.internal_format = GL_RGBA16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 4;
		break;

	case THE_TEX_DEPTH:
		config.format = GL_DEPTH_COMPONENT;
		config.internal_format = GL_DEPTH_COMPONENT;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_BORDER;
		config.channels = 1;
		break;

	case THE_TEX_RGB_F16:
		config.format = GL_RGB;
		config.internal_format = GL_RGB16F;
		config.type = GL_FLOAT;
		config.filter = GL_LINEAR;
		config.wrap = GL_CLAMP_TO_EDGE;
		config.channels = 3;
		break;

	case THE_TEX_SKYBOX:
	case THE_TEX_ENVIRONMENT:
	case THE_TEX_PREFILTER_ENVIRONMENT:
		the__create_cubemap(tex);
		return;
		break;

	default:
		THE_SLOG_ERROR("Invalid format");
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
			THE_Free(t->pix[0]);
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

static enum THE_ErrorCode
the__load_text_file(const char *path, char *buffer, size_t buffsize)
{
	FILE *fp = fopen(path, "r");

	if (!fp) {
		THE_LOG_ERROR("File %s couldn't be opened.", path);
		return THE_EC_FILE;
	}

	fread((void *)buffer, 1, buffsize - 1, fp);
	fclose(fp);

	if (buffer[buffsize - 1] != '\0') {
		THE_LOG_ERROR("File %s bigger than buffer size.", path);
		return THE_EC_ALLOC;
	}

	return THE_EC_SUCCESS;
}

static enum THE_ErrorCode
the__create_shader(THE_InternalShader *shader)
{
#define SHADER_BUFFSIZE 8192
	THE_ASSERT(shader->res.id == THE_UNINIT,
	           "The material must be uninitialized.");
	THE_ASSERT(shader->shader_name, "Empty shader name");

	char vert[SHADER_BUFFSIZE] = { '\0' };
	char frag[SHADER_BUFFSIZE] = { '\0' };
	char vert_path[256] = { '\0' };
	char frag_path[256] = { '\0' };
	strcpy(frag_path, "assets/shaders/");
	strcat(frag_path, shader->shader_name);
	strcpy(vert_path, frag_path);
	strcat(vert_path, "-vert.glsl");
	strcat(frag_path, "-frag.glsl");

	if (the__load_text_file(vert_path, vert, SHADER_BUFFSIZE) !=
	    THE_EC_SUCCESS) {
		return THE_EC_FILE;
	}

	if (the__load_text_file(frag_path, frag, SHADER_BUFFSIZE) !=
	    THE_EC_SUCCESS) {
		if (frag[SHADER_BUFFSIZE - 1] != '\0') {
			THE_LOG_ERROR("File %s bigger than buffer size.", frag_path);
		}
		return THE_EC_FILE;
	}
#undef SHADER_BUFFSIZE

	GLint err;
	GLchar output_log[512];
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *shader_src = &vert[0];
	glShaderSource(vert_shader, 1, &shader_src, NULL);
	glCompileShader(vert_shader);
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(vert_shader, 512, NULL, output_log);
		THE_LOG_ERROR("%s vertex shader compilation failed:\n%s\n",
		              shader->shader_name, output_log);
		return THE_EC_FAIL;
	}
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	shader_src = &frag[0];
	glShaderSource(frag_shader, 1, &shader_src, NULL);
	glCompileShader(frag_shader);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(frag_shader, 512, NULL, output_log);
		THE_LOG_ERROR("%s fragment shader compilation failed:\n%s\n",
		              shader->shader_name, output_log);
		return THE_EC_FAIL;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &err);
	if (!err) {
		glGetProgramInfoLog(program, 512, NULL, output_log);
		THE_LOG_ERROR("%s program error:\n%s\n", shader->shader_name,
		              output_log);
		return THE_EC_FAIL;
	}

	shader->res.id = program;
	// TODO: Cambiar el nombre de los uniforms de scene quitando scene.
	shader->data_loc[THE_SHADER_COMMON_DATA].data =
	  glGetUniformLocation(shader->res.id, "u_scene_data");
	shader->data_loc[THE_SHADER_COMMON_DATA].tex =
	  glGetUniformLocation(shader->res.id, "u_scene_tex");
	shader->data_loc[THE_SHADER_COMMON_DATA].cubemap =
	  glGetUniformLocation(shader->res.id, "u_scene_cube");
	shader->data_loc[THE_SHADER_DATA].data =
	  glGetUniformLocation(shader->res.id, "u_entity_data");
	shader->data_loc[THE_SHADER_DATA].tex =
	  glGetUniformLocation(shader->res.id, "u_entity_tex");
	shader->data_loc[THE_SHADER_DATA].cubemap =
	  glGetUniformLocation(shader->res.id, "u_entity_cube");

	return THE_EC_SUCCESS;
}

static void
the__sync_gpu_mesh(THE_Mesh m, THE_Shader s)
{
	THE_InternalMesh *im = meshes + m;
	if (im->res.id == THE_UNINIT) {
		the__create_mesh(m, s);
	}
	im->res.flags = 0;
}

static THE_InternalTexture *
the__sync_gpu_tex(THE_Texture tex)
{
	THE__CHECK_HANDLE(texture, tex);
	THE_InternalTexture *mutated = NULL;
	THE_InternalTexture *itex = textures + tex;
	if (itex->res.id == THE_UNINIT) {
		the__create_texture(tex);
		mutated = itex;
	}
	THE_ASSERT(the__resource_check(itex), "Invalid internal resource.");
	return mutated;
}

static void
the__sync_gpu_shader(THE_InternalShader *is)
{
	if (is->res.id == THE_UNINIT) {
		the__create_shader(is);
	}
	is->res.flags = 0;
	THE_ASSERT(the__resource_check(is), "Error creating internal shader.");
}

static void
the__sync_gpu_fb(THE_Framebuffer fb, const THE_FBAttachment *atta)
{
	THE_ASSERT(atta, "Null ptr arg.");
	THE_InternalFramebuffer *ifb = framebuffers + fb;
	if (ifb->res.id == THE_UNINIT) {
		glGenFramebuffers(1, (GLuint *)&(ifb->res.id));
	}
	THE_ASSERT(the__resource_check(ifb), "Invalid internal resource.");
	glBindFramebuffer(GL_FRAMEBUFFER, ifb->res.id);
	if (atta->slot == THE_ATTACH_DEPTH) {
		// Change depth texture
		THE__CHECK_HANDLE(texture, atta->tex);
		ifb->depth_tex = atta->tex;
		the__sync_gpu_tex(ifb->depth_tex);
		the__attach_to_fb(*atta);
	} else if (atta->slot == THE_ATTACH_COLOR) {
		// Change color texture
		THE__CHECK_HANDLE(texture, atta->tex);
		ifb->color_tex = atta->tex;
		the__sync_gpu_tex(ifb->color_tex);
		the__attach_to_fb(*atta);
	} else {
		if (ifb->depth_tex != THE_INACTIVE) {
			the__sync_gpu_tex(ifb->depth_tex);
			THE_FBAttachment a = { .tex = ifb->depth_tex,
				                   .slot = THE_ATTACH_DEPTH,
				                   .side = -1,
				                   .level = 0 };
			the__attach_to_fb(a);
		}

		if (ifb->color_tex != THE_INACTIVE) {
			the__sync_gpu_tex(ifb->color_tex);
			THE_FBAttachment a = { .tex = ifb->color_tex,
				                   .slot = THE_ATTACH_COLOR,
				                   .side = -1,
				                   .level = 0 };
			the__attach_to_fb(a);
		}
	}
	ifb->res.flags = 0;
}

static void
the__set_shader_data(THE_Material m, enum THE_DataGroup group)
{
	THE_InternalShader *s = shaders + m.shader;
	THE_ASSERT(the__resource_check(s), "Invalid internal shader.");
	THE_Texture *t = (THE_Texture *)m.ptr + m.data_count;
	for (int i = 0; i < m.tex_count + m.cube_count; ++i) {
		if (the__is_dirty(textures + t[i])) {
			the__sync_gpu_tex(t[i]);
		}
	}

	glUniform4fv(s->data_loc[group].data, m.data_count / 4, m.ptr);
	glUniform1iv(s->data_loc[group].tex, m.tex_count, t);
	glUniform1iv(s->data_loc[group].cubemap, m.cube_count, t + m.tex_count);
}

void
THE_UseShaderExecute(THE_CommandData *data)
{
	THE__CHECK_HANDLE(shader, data->mat.shader);
	THE_InternalShader *s = shaders + data->mat.shader;
	if (the__is_dirty(s)) {
		the__sync_gpu_shader(s);
	}
	glUseProgram(s->res.id);
	the__set_shader_data(data->mat, THE_SHADER_COMMON_DATA);
}

void
THE_ClearExecute(THE_CommandData *data)
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
THE_DrawExecute(THE_CommandData *data)
{
	THE_Mesh mesh = data->draw.mesh;
	THE__CHECK_HANDLE(mesh, mesh);
	THE_ASSERT(meshes[mesh].elements, "Attempt to draw an uninitialized mesh");

	if (the__is_dirty(meshes + mesh)) {
		the__sync_gpu_mesh(mesh, data->draw.material.shader);
	}

	glBindVertexArray(meshes[mesh].res.id);
	the__set_shader_data(data->draw.material, THE_SHADER_DATA);
	glDrawElements(GL_TRIANGLES, meshes[mesh].elements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void
THE_RenderOptionsExecute(THE_CommandData *data)
{
	int enable = data->rend_opts.enable_flags;
	int disable = data->rend_opts.disable_flags;
	for (int i = 0; i < THE_REND_OPTS_COUNT; ++i) {
		/* Prioritizing disable for bad configs (both on). */
		if (disable & (1 << i)) {
			the__disable_render_opt(1 << i);
		} else if (enable & (1 << i)) {
			the__enable_render_opt(1 << i);
		}
	}

	THE_BlendFunc blend = data->rend_opts.blend_func;
	/* Ignore unless both have a value assigned. */
	if (blend.src && blend.dst) {
		glBlendFunc(the__gl_blend(blend.src), the__gl_blend(blend.dst));
	}

	THE_CullFace cull = data->rend_opts.cull_face;
	if (cull) {
		glCullFace(the__gl_cull(cull));
	}

	THE_DepthFunc depth = data->rend_opts.depth_func;
	if (depth) {
		glDepthFunc(the__gl_depth(depth));
	}
}

void
THE_SetFramebufferExecute(THE_CommandData *data)
{
	THE_ASSERT(data, "Null param");
	const THE_SetFramebufferData *d = &data->set_fb;
	if (d->fb == THE_DEFAULT) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		the__set_viewport(-1, -1);
		return;
	}

	if (d->attachment.slot != THE_IGNORE) {
		framebuffers[d->fb].res.flags |= RF_DIRTY;
	}

	THE__CHECK_HANDLE(framebuffer, d->fb);
	if (the__is_dirty(framebuffers + d->fb)) {
		the__sync_gpu_fb(d->fb, &d->attachment);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[d->fb].res.id);
	}
	if (d->vp_x > 0) {
		the__set_viewport(d->vp_x, d->vp_y);
	} else {
		the__set_viewport_from_fb(d->fb);
	}
}

#endif // THE_OPENGL
