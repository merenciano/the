#include "rendercommands.h"

#include "core/io.h"
#include "core/thefinitions.h"
#include "internalresources.h"
#include "renderer.h"
#include <mathc.h>

#include <assert.h>
#include <string.h>

#define THE__IS_DIRTY(RESOURCE) ((RESOURCE)->res.flags & RF_DIRTY)

#define THE_OPENGL // Hardcoded define until another render backend is
                   // implemented.
#ifdef THE_OPENGL
#include "glad/glad.h"
#include "stb_image.h"

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

	glGenVertexArrays(1, (GLuint *)&m->internal_id);
	glBindVertexArray(m->internal_id);
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
		GLint attrib_pos = glGetAttribLocation(s->program_id, attrib_names[i]);
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
}

static void
CreateCubemap(THE_Texture tex)
{
	THE_CubeConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(t->cpu_version > t->gpu_version,
	           "Texture not created on CPU or already created on GPU");
	THE_ASSERT(tex < 62, "Start thinking about the max textures");
	THE_ASSERT(t->internal_id == THE_UNINIT,
	           "Texture already created in the gpu");

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

	glGenTextures(1, (GLuint *)&(t->internal_id));
	t->texture_unit = tex + 1;
	glActiveTexture(GL_TEXTURE0 + t->texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, t->internal_id);

	// The path for cubemaps will be the directory where the skyboxfaces
	// are and inside the directory the faces must have these names
	if (t->type == THE_TEX_SKYBOX) {
		const char *cube_prefix = "RLUDFB";
		int width, height, nchannels;
		stbi_set_flip_vertically_on_load(0);
		for (int i = 0; i < 6; ++i) {
			t->path[13] = cube_prefix[i];
			uint8_t *img_data = stbi_load(t->path, &width, &height, &nchannels,
			                              0);
			THE_ASSERT(img_data, "Couldn't load the image to the cubemap");
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			             config.internal_format, width, height, 0,
			             config.format, config.type, img_data);
			stbi_image_free(img_data);
		}
		t->width = width;
		t->height = height;
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
	t->gpu_version = t->cpu_version;
}

static void
the__create_texture(THE_Texture tex, bool release_from_ram)
{
	THE_TextureConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(t->cpu_version == 1, "Texture created before?");
	THE_ASSERT(tex < 62, "Max texture units"); // Tex unit is id + 1
	THE_ASSERT(t->internal_id == THE_UNINIT, "Texture already created on GPU");

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
		CreateCubemap(tex);
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

	glGenTextures(1, (GLuint *)&(t->internal_id));
	t->texture_unit = tex + 1;
	glActiveTexture(GL_TEXTURE0 + t->texture_unit);
	glBindTexture(GL_TEXTURE_2D, t->internal_id);

	if (t->type == THE_TEX_RGB_F16) {
		if (!t->pix) {
			stbi_set_flip_vertically_on_load(1);
			int width, height, nchannels;
			t->pix = stbi_loadf(t->path, &width, &height, &nchannels, 0);
			t->width = width;
			t->height = height;
		}
		THE_ASSERT(t->pix, "The image couldn't be loaded");
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width,
		             t->height, 0, config.format, config.type, t->pix);
		if (release_from_ram) {
			stbi_image_free(t->pix);
			t->pix = NULL;
		}
	} else if (*(t->path) != '\0') {
		if (!t->pix) {
			stbi_set_flip_vertically_on_load(1);
			int width, height, nchannels;
			t->pix = stbi_load(t->path, &width, &height, &nchannels,
			                   config.channels);
			t->width = width;
			t->height = height;
		}

		THE_ASSERT(t->pix, "The image couldn't be loaded");
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width,
		             t->height, 0, config.format, config.type, t->pix);
		glGenerateMipmap(GL_TEXTURE_2D);
		if (release_from_ram) {
			stbi_image_free(t->pix);
			t->pix = NULL;
		}
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width,
		             t->height, 0, config.format, config.type, NULL);
	}
	// No shadows outside shadow maps
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap);
	t->gpu_version = t->cpu_version;
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
	THE_ASSERT(shader->program_id == THE_UNINIT,
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

	shader->program_id = program;
	// TODO: Cambiar el nombre de los uniforms de scene quitando scene.
	shader->data_loc[THE_SHADER_COMMON_DATA].data =
	  glGetUniformLocation(shader->program_id, "u_scene_data");
	shader->data_loc[THE_SHADER_COMMON_DATA].tex =
	  glGetUniformLocation(shader->program_id, "u_scene_tex");
	shader->data_loc[THE_SHADER_COMMON_DATA].cubemap =
	  glGetUniformLocation(shader->program_id, "u_scene_cube");
	shader->data_loc[THE_SHADER_DATA].data =
	  glGetUniformLocation(shader->program_id, "u_entity_data");
	shader->data_loc[THE_SHADER_DATA].tex =
	  glGetUniformLocation(shader->program_id, "u_entity_tex");
	shader->data_loc[THE_SHADER_DATA].cubemap =
	  glGetUniformLocation(shader->program_id, "u_entity_cube");

	return THE_EC_SUCCESS;
}

static void
the__set_shader_data(THE_Mat m, enum THE_DataGroup group)
{
	THE_InternalShader *s = shaders + m.shader;
	THE_ASSERT(s->program_id, "Shader uninit.");
	THE_ASSERT(m.tex_count < 16, "So many textures, increase array size");
	// TODO: Tex units igual que el handle de textura (saltarse la textura
	// 0 si hace falta)
	int tex_units[16];
	for (int i = 0; i < m.tex_count + m.cube_count; ++i) {
		THE_Texture t = ((THE_Texture *)m.ptr)[m.data_count + i];
		THE_ASSERT(t != THE_UNINIT, "Texture not created");
		THE_ASSERT(textures[t].cpu_version != -1, "Texture released");
		if (textures[t].gpu_version == 0) {
			the__create_texture(t, true);
		}
		tex_units[i] = textures[t].texture_unit;
	}

	glUniform4fv(s->data_loc[group].data, m.data_count / 4, m.ptr);
	glUniform1iv(s->data_loc[group].tex, m.tex_count, tex_units);
	glUniform1iv(s->data_loc[group].cubemap, m.cube_count,
	             tex_units + m.tex_count);
}

void
THE_UseShaderExecute(THE_CommandData *data)
{
	THE_InternalShader *s = shaders + data->mat.shader;
	if (s->program_id == THE_UNINIT) {
		the__create_shader(s);
	}
	glUseProgram(s->program_id);
	the__set_shader_data(data->mat, THE_SHADER_COMMON_DATA);
}

void
THE_ClearExecute(THE_CommandData *data)
{
	uint32_t mask = 0;
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
	THE_ASSERT(meshes[mesh].elements, "Attempt to draw an uninitialized mesh");

	if (meshes[mesh].internal_id == THE_UNINIT) {
		the__create_mesh(mesh, data->draw.material.shader);
	}

	glBindVertexArray(meshes[mesh].internal_id);
	the__set_shader_data(data->draw.material, THE_SHADER_DATA);
	glDrawElements(GL_TRIANGLES, meshes[mesh].elements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void
THE_EquirectToCubeExecute(THE_CommandData *data)
{
	THE_Texture o_cube = data->eqr_cube.out_cube;
	THE_Texture o_pref = data->eqr_cube.out_prefilt;
	THE_Texture o_lut = data->eqr_cube.out_lut;
	const char *path = data->eqr_cube.in_path;
	THE_InternalTexture *icu = textures + o_cube;

	if (icu->cpu_version > icu->gpu_version) {
		CreateCubemap(o_cube);
	}

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

	GLuint fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0, 0, icu->width, icu->height);
	// Manually sync framebuffer here after changing textures
	THE_Texture equirec = THE_CreateTexture(path, THE_TEX_RGB_F16);
	the__create_texture(equirec, false);
	THE_CommandData ro;
	ro.rend_opts.disable_flags = THE_CULL_FACE;
	THE_RenderOptionsExecute(&ro);

	THE_CommandData use_tocube = { .mat.shader = 2,
		                           .mat.data_count = 0,
		                           .mat.tex_count = 1,
		                           .mat.cube_count = 0 };
	THE_Texture *eqtex = THE_MatAllocFrame(&use_tocube.mat);
	*eqtex = equirec;
	THE_UseShaderExecute(&use_tocube);

	THE_Mat mat_tocube = {
		.shader = 2, .data_count = 16 * 6, .tex_count = 0, .cube_count = 0
	};
	float *vp = THE_MatAllocFrame(&mat_tocube);

	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
		                       icu->internal_id, 0);

		float *view = (float *)&views[i];
		mat4_multiply(vp + 16 * i, proj, view);
		THE_DrawData dd = { .mesh = CUBE_MESH, .material = mat_tocube };
		dd.material.data_count = 16;
		dd.material.ptr = (float *)mat_tocube.ptr + 16 * i;
		THE_DrawExecute((THE_CommandData *)&dd);
	}

	if (o_pref != THE_UNINIT) {
		THE_InternalTexture *ipref = textures + o_pref;
		if (ipref->cpu_version > ipref->gpu_version) {
			CreateCubemap(o_pref);
		}

		struct THE_PrefilterEnvData pref_data;
		for (int i = 0; i < 5; ++i) {
			// mip size
			int32_t s = (float)ipref->width * powf(0.5f, (float)i);
			glViewport(0, 0, s, s);
			pref_data.roughness = i / 4.0f; // mip / max mip levels - 1

			THE_CommandData draw_cd;
			draw_cd.draw.mesh = CUBE_MESH;
			for (int j = 0; j < 6; ++j) {
				float *view = (float *)&views[j];
				mat4_multiply(pref_data.vp, proj, view);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
				                       ipref->internal_id, i);
				glClear(GL_COLOR_BUFFER_BIT);
				draw_cd.draw.material.shader = 3; // prefilter
				                                  // env
				draw_cd.draw.material.data_count =
				  sizeof(struct THE_PrefilterEnvData) / 4;
				draw_cd.draw.material.tex_count = 0;
				draw_cd.draw.material.cube_count = 1;
				struct THE_PrefilterEnvData *d =
				  THE_MatAllocFrame(&draw_cd.draw.material);
				*d = pref_data;
				((THE_Texture *)d)[draw_cd.draw.material.data_count] = o_cube;

				THE_CommandData cmdata;
				cmdata.mat = draw_cd.draw.material;
				THE_UseShaderExecute(&cmdata);
				THE_DrawExecute(&draw_cd);
			}
		}
	}

	if (o_lut != THE_UNINIT) {
		THE_InternalTexture *ilut = textures + o_lut;
		if (ilut->cpu_version > ilut->gpu_version) {
			the__create_texture(o_lut, false);
		}
		glViewport(0, 0, ilut->width, ilut->height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		                       GL_TEXTURE_2D, ilut->internal_id, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		THE_CommandData draw_cd;
		draw_cd.draw.mesh = QUAD_MESH;
		draw_cd.draw.material = THE_MatDefault();
		draw_cd.draw.material.shader = 4; // THE_MT_LUT_GEN;
		THE_CommandData cmdata;
		cmdata.mat = draw_cd.draw.material;
		THE_UseShaderExecute(&cmdata);
		THE_DrawExecute(&draw_cd);
	}

	glDeleteFramebuffers(1, &fb);
	THE_FreeTextureData(equirec);
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
	}
	return;
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
	return;
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

static THE_InternalTexture *
the__sync_gpu_tex(THE_Texture tex)
{
	THE__CHECK_HANDLE(texture, tex);
	THE_InternalTexture *mutated = NULL;
	THE_InternalTexture *itex = textures + tex;
	if (itex->internal_id == THE_UNINIT) {
		the__create_texture(tex, false);
		mutated = itex;
	}
	THE__CHECK_INTERNAL(itex);
	return mutated;
}

static void
the__attach_to_fb(THE_FBAttachment a)
{
	int id = textures[a.tex].internal_id;
	GLenum target = a.side < 0 ? GL_TEXTURE_2D :
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + a.side;

	glFramebufferTexture2D(GL_FRAMEBUFFER, a.slot, target, id, a.level);
}

static void
the__sync_gpu_fb(THE_Framebuffer fb, const THE_FBAttachment *atta)
{
	THE_ASSERT(atta, "Null ptr arg.");
	THE_InternalFramebuffer *ifb = framebuffers + fb;
	if (ifb->res.id == THE_UNINIT) {
		glGenFramebuffers(1, (GLuint *)&(ifb->res.id));
	}
	THE__CHECK_INTERNAL(ifb);
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
			THE_FBAttachment a = {
				.tex = ifb->depth_tex,
				.slot = THE_ATTACH_DEPTH,
				.side = -1,
				.level = 0
			};
			the__attach_to_fb(a);
		}

		if (ifb->color_tex != THE_INACTIVE) {
			the__sync_gpu_tex(ifb->color_tex);
			THE_FBAttachment a = {
			  .tex = ifb->color_tex,
			  .slot = THE_ATTACH_COLOR,
			  .side = -1,
			  .level = 0
			};
			the__attach_to_fb(a);
		}
	}
	ifb->res.flags = 0;
}

void
the__set_viewport(THE_Framebuffer fb)
{
	int w, h;
	THE_FbDimensions(fb, &w, &h);
	glViewport(0, 0, w, h);
}

void
THE_SetFramebufferExecute(THE_CommandData *data)
{
	THE_ASSERT(data, "Null param");
	const THE_SetFramebufferData *d = &data->set_fb;
	if (d->fb == THE_DEFAULT) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, THE_WindowGetWidth(), THE_WindowGetHeight());
		return;
	}

	THE__CHECK_HANDLE(framebuffer, d->fb);
	if (THE__IS_DIRTY(framebuffers + d->fb)) {
		the__sync_gpu_fb(d->fb, &d->attachment);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[d->fb].res.id);
	}
	the__set_viewport(d->fb);
}

void
THE_UseFramebufferExecute(THE_CommandData *data)
{
	if (data->usefb == THE_DEFAULT) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	THE_InternalFramebuffer *ifb = framebuffers + data->usefb;
	GLsizei width;
	GLsizei height;

	if (ifb->gpu_version == 0) {
		THE_ASSERT(data->usefb >= 0 && ifb->cpu_version > 0,
		           "Framebuffer not created");
		glGenFramebuffers(1, (GLuint *)&(ifb->internal_id));
		if (ifb->color_tex >= 0) {
			the__create_texture(ifb->color_tex, false);
		}
		if (ifb->depth_tex >= 0) {
			the__create_texture(ifb->depth_tex, false);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, ifb->internal_id);

	// Set viewport
	if (ifb->color_tex >= 0) {
		width = textures[ifb->color_tex].width;
		height = textures[ifb->color_tex].height;
		glViewport(0, 0, width, height);
	}

	if (ifb->depth_tex >= 0) {
		if (ifb->color_tex >= 0) {
			THE_ASSERT(
			  width == textures[ifb->depth_tex].width &&
				height == textures[ifb->depth_tex].height,
			  "Color and depth texture sizes of framebuffer not matching");
		} else {
			width = textures[ifb->depth_tex].width;
			height = textures[ifb->depth_tex].height;
			glViewport(0, 0, width, height);
		}
	}

	// Update framebuffer if the textures have been changed
	if (ifb->gpu_version < ifb->cpu_version) {
		if (ifb->color_tex >= 0) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			                       GL_TEXTURE_2D,
			                       textures[ifb->color_tex].internal_id, 0);
		}
		if (ifb->depth_tex >= 0) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			                       GL_TEXTURE_2D,
			                       textures[ifb->depth_tex].internal_id, 0);
		}
		ifb->gpu_version = ifb->cpu_version;
	}
}

#endif // THE_OPENGL
