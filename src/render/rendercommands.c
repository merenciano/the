#include "rendercommands.h"

#include "core/thefinitions.h"
#include "renderer.h" // TODO Move frame allocator to rendercommands and remove this include 
#include "internalresources.h"
#include "camera.h"
#include "material.h"
#include <mathc.h>

#include <string.h>
#include <assert.h>

#define THE_OPENGL // Hardcoded define until another render backend is implemented.
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

static GLint GetAttribSize(enum THE_VertexAttributes attr)
{
	switch(attr) {
		case A_POSITION:
		case A_NORMAL:
		case A_TANGENT:
		case A_BITANGENT:
			return 3;
		case A_UV:
			return 2;
		default:
			return -1;
	}
}

static GLsizei GetAttribStride(int32_t attr_flags)
{
	switch(attr_flags)
	{
		case (1 << A_POSITION):
			return 3 * sizeof(float);
		case (1 << A_POSITION) | (1 << A_NORMAL):
			return 6 * sizeof(float);
		case (1 << A_POSITION) | (1 << A_UV):
			return 5 * sizeof(float);
		case (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV):
			return 8 * sizeof(float);
		case (1 << A_POSITION) | (1 << A_UV) | (1 << A_TANGENT) | 
			(1 << A_BITANGENT):
			return 11 * sizeof(float);
		case (1 << A_POSITION) | (1 << A_NORMAL) | (1 <<  A_UV) | 
			(1 << A_TANGENT) | (1 << A_BITANGENT):
			return 14 * sizeof(float);
		default :
			THE_ASSERT(false, "Default case.");
			return 0;
	}
}

static void CreateMesh(THE_Mesh mesh, THE_Shader shader)
{
	THE_InternalMesh *m = meshes + mesh;
	THE_InternalShader *s = shaders + shader;

	glGenVertexArrays(1, (GLuint*)&m->internal_id);
	glBindVertexArray(m->internal_id);
	//glGenBuffers(2, m->internal_buffers_id);

	glGenBuffers(1, &m->internal_buffers_id[0]);
	glBindBuffer(GL_ARRAY_BUFFER, m->internal_buffers_id[0]);
	glBufferData(GL_ARRAY_BUFFER, m->vtx_size, m->vtx, GL_STATIC_DRAW);

	GLint attrib_pos = glGetAttribLocation(s->program_id, "a_position");
	if (attrib_pos >= 0) {
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE,
			8 * sizeof(float), (void*)0);
		glVertexAttribDivisor(attrib_pos, 0);
	}

	attrib_pos = glGetAttribLocation(s->program_id, "a_normal"); 
	if (attrib_pos >= 0) {
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE,
			8 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribDivisor(attrib_pos, 0);
	}

	attrib_pos = glGetAttribLocation(s->program_id, "a_uv");
	if (attrib_pos >= 0) {
		glEnableVertexAttribArray(attrib_pos);
		glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE,
			8 * sizeof(float), (void*)(6 * sizeof(float)));
		glVertexAttribDivisor(attrib_pos, 0);
	}

	glGenBuffers(1, &m->internal_buffers_id[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->internal_buffers_id[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->elements * sizeof(uint32_t),
		(const void*)m->idx, GL_STATIC_DRAW);

	/*GLsizei offset = 0;
	GLsizei s = GetAttribStride(m->attr_flags);
	GLuint a = 0; // Attribute index
	for (GLint i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
		if (!(m->attr_flags & (1 << i)))
			continue;

		GLint sz = GetAttribSize(i);
		glEnableVertexAttribArray(a);
		glVertexAttribPointer(a, sz, GL_FLOAT, GL_FALSE, s, (const void*)&offset);
		glVertexAttribDivisor(a, 0);
		++a;
		offset += sz * sizeof(float);
	}*/
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void CreateTexture(THE_Texture tex, bool release_from_ram)
{
	THE_TextureConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(t->cpu_version == 1, "Texture created before?");
	THE_ASSERT(tex < 62, "Max texture units"); // Tex unit is id + 1
	THE_ASSERT(t->internal_id == THE_UNINIT, "Texture already created on GPU");

	switch(t->type) {
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

	glGenTextures(1, (GLuint*)&(t->internal_id));
	t->texture_unit = tex + 1;
	glActiveTexture(GL_TEXTURE0 + t->texture_unit);
	glBindTexture(GL_TEXTURE_2D, t->internal_id);

	if (t->type == THE_TEX_RGB_F16) {
		if (!t->pix) {
			stbi_set_flip_vertically_on_load(1);
			int width, height, nchannels;
			t->pix = stbi_loadf(
				t->path, &width, &height, &nchannels, 0);
			t->width = width;
			t->height = height;
		}
		THE_ASSERT(t->pix, "The image couldn't be loaded");
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width, t->height, 0,
			config.format, config.type, t->pix);
		if (release_from_ram) {
			stbi_image_free(t->pix);
			t->pix = NULL;
		}
	} else if (*(t->path) != '\0') {
		if (!t->pix) {
			stbi_set_flip_vertically_on_load(1);
			int width, height, nchannels;
			t->pix = stbi_load(t->path, &width, &height, &nchannels, config.channels);
			t->width = width;
			t->height = height;
		}

		THE_ASSERT(t->pix, "The image couldn't be loaded");
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width, t->height, 0,
			config.format, config.type, t->pix);
		glGenerateMipmap(GL_TEXTURE_2D);
		if (release_from_ram) {
			stbi_image_free(t->pix);
			t->pix = NULL;
		}
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, config.internal_format, t->width, t->height, 0,
			config.format, config.type, NULL);
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

static void CreateCubemap(THE_Texture tex)
{
	THE_CubeConfig config;
	THE_InternalTexture *t = textures + tex;

	THE_ASSERT(t->cpu_version > t->gpu_version, "Texture not created on CPU or already created on GPU");
	THE_ASSERT(tex < 62, "Start thinking about the max textures");
	THE_ASSERT(t->internal_id == THE_UNINIT, "Texture already created in the gpu");

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

	glGenTextures(1, (GLuint*)&(t->internal_id));
	t->texture_unit = tex + 1;
	glActiveTexture(GL_TEXTURE0 + t->texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, t->internal_id);

	// The path for cubemaps will be the directory where the skyboxfaces are
	// and inside the directory the faces must have these names
	if (t->type == THE_TEX_SKYBOX) {
		char *faces[6] = {
			// MEGA TODO: Fix this soon
			/*
			strcat(t.path, "/right.jpg"),
			strcat(t.path, "/left.jpg"),
			strcat(t.path, "/up.jpg"),
			strcat(t.path, "/down.jpg"),
			strcat(t.path, "/front.jpg"),
			strcat(t.path, "/back.jpg"),*/
	
		};
	
		int width, height, nchannels;
		stbi_set_flip_vertically_on_load(0);
		for (int i = 0; i < 6; ++i) {
			uint8_t *img_data = stbi_load(faces[i], &width, &height, &nchannels, 0);
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
				config.internal_format, t->width,
				t->height, 0, config.format, config.type, 0);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, config.wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, config.min_filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, config.mag_filter);
	if (t->type == THE_TEX_PREFILTER_ENVIRONMENT) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	t->gpu_version = t->cpu_version;
}

static enum THE_ErrorCode LoadFile(const char *path, char *buffer, size_t buffsize)
{
	FILE* fp = fopen(path, "r");

	if (!fp) {
		THE_LOG_ERROR("File %s couldn't be opened.", path);
		return THE_EC_FILE;
	}

	fread((void*)buffer, 1, buffsize - 1, fp);
	fclose(fp);

	if (buffer[buffsize - 1] != '\0') {
		THE_LOG_ERROR("File %s bigger than buffer size.", path);
		return THE_EC_ALLOC;
	}

	return THE_EC_SUCCESS;
}

static enum THE_ErrorCode CreateShader(THE_InternalShader *shader)
{
	#define SHADER_BUFFSIZE 4096
	THE_ASSERT(shader->program_id == THE_UNINIT, "The material must be uninitialized.");
	THE_ASSERT(shader->shader_name, "Empty shader name");

	char vert[SHADER_BUFFSIZE] = {'\0'};
	char frag[SHADER_BUFFSIZE] = {'\0'};
	char vert_path[256] = {'\0'};
	char frag_path[256] = {'\0'};
	strcpy(frag_path, "assets/shaders/");
	strcat(frag_path, shader->shader_name);
	strcpy(vert_path, frag_path);
	strcat(vert_path, "-vert.glsl");
	strcat(frag_path, "-frag.glsl");

	if (LoadFile(vert_path, vert, SHADER_BUFFSIZE) != THE_EC_SUCCESS) {
		return THE_EC_FILE;
	}

	if (LoadFile(frag_path, frag, SHADER_BUFFSIZE) != THE_EC_SUCCESS) {
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
		THE_LOG_ERROR("%s vertex shader compilation failed:\n%s\n", shader->shader_name, output_log);
		return THE_EC_FAIL;
	}
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	shader_src = &frag[0];
	glShaderSource(frag_shader, 1, &shader_src, NULL);
	glCompileShader(frag_shader);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(frag_shader, 512, NULL, output_log);
		THE_LOG_ERROR("%s fragment shader compilation failed:\n%s\n", shader->shader_name, output_log);
		return THE_EC_FAIL;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &err);
	if (!err) {
		glGetProgramInfoLog(program, 512, NULL, output_log);
		THE_LOG_ERROR("%s program error:\n%s\n", shader->shader_name, output_log);
		return THE_EC_FAIL;
	}

	shader->program_id = program;
	// TODO: crear defines para uniforms de scena y enitdad y usarlos como indice.
	// TODO: Cambiar el nombre de los uniforms de scene quitando scene.
	shader->data_loc[0].data = glGetUniformLocation(shader->program_id, "u_scene_data");
	shader->data_loc[0].tex = glGetUniformLocation(shader->program_id, "u_scene_tex");
	shader->data_loc[0].cubemap = glGetUniformLocation(shader->program_id, "u_scene_cube");
	shader->data_loc[1].data = glGetUniformLocation(shader->program_id, "u_entity_data");
	shader->data_loc[1].tex = glGetUniformLocation(shader->program_id, "u_entity_tex");
	shader->data_loc[1].cubemap = glGetUniformLocation(shader->program_id, "u_entity_cube");

	return THE_EC_SUCCESS;
}

static void SetMaterialData(THE_Shader mat, THE_Material data, int32_t group)
{
	THE_InternalShader *m = shaders + mat;

	THE_ASSERT(m->program_id, "Shader uninit.");

	THE_ASSERT(data.tcount < 16, "So many textures, increase array size");
	// TODO: Tex units igual que el handle de textura (saltarse la textura 0 si hace falta)
	int32_t tex_units[16];
	for (int i = 0; i < data.tcount; ++i)
	{
		if (data.tex[i] != THE_UNINIT)
		{
			THE_ASSERT(data.tex[i] != -1, "Texture not created");
			THE_ASSERT(textures[data.tex[i]].cpu_version != -1, "Texture released");
			if (textures[data.tex[i]].gpu_version == 0)
			{
				CreateTexture(data.tex[i], true);
			}
		}
		tex_units[i] = textures[data.tex[i]].texture_unit;
	}

	glUniform4fv(m->data_loc[group].data, data.dcount / 4, data.data);
	glUniform1iv(m->data_loc[group].tex, data.cube_start, tex_units);
	glUniform1iv(m->data_loc[group].cubemap, data.tcount - data.cube_start,
		tex_units + data.cube_start);
}

void THE_UseShaderExecute(THE_CommandData *data)
{
	THE_InternalShader *m = shaders + data->use_shader;
	if (m->program_id == THE_UNINIT) {
		CreateShader(m);
	}
	glUseProgram(m->program_id);
	SetMaterialData(data->use_shader, m->common_data, 0);
}

void THE_ClearExecute(THE_CommandData *data)
{
	uint32_t mask = 0;
	if (data->clear.bcolor) {
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (data->clear.bdepth) {
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (data->clear.bstencil) {
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	glClearColor(data->clear.color[0], data->clear.color[1],
		data->clear.color[2], data->clear.color[3]);
	glClear(mask);
}

void THE_SkyboxExecute(THE_CommandData *data)
{
	THE_Material skymatdata = THE_MaterialDefault();
	skymatdata.data = THE_AllocateFrameResource(16 * sizeof(float));
	THE_CameraStaticViewProjection(skymatdata.data, &camera);
	skymatdata.dcount = 16;
	skymatdata.tex = THE_AllocateFrameResource(sizeof(THE_Texture));
	*(skymatdata.tex) = data->skybox.cubemap;
	skymatdata.tcount = 1;
	skymatdata.cube_start = 0;

	THE_ASSERT(meshes[CUBE_MESH].elements, "Uninit mesh");
	/*THE_ASSERT(CUBE_MESH.vertex != THE_UNINIT,
		"You are trying to draw with an uninitialized vertex buffer");

	THE_ASSERT(CUBE_MESH.index != THE_UNINIT,
		"You are trying to draw with an uninitialized index buffer");

	THE_ASSERT(buffers[CUBE_MESH.vertex].cpu_version > 0, "Vertex buffer without data");
	THE_ASSERT(buffers[CUBE_MESH.index].cpu_version > 0, "Index buffer without data");*/

	// Set the uniforms
	THE_Shader mat = 1; // skybox
	THE_CommandData usenewmatdata;
	usenewmatdata.use_shader = mat;
	shaders[mat].common_data = skymatdata;
	THE_UseShaderExecute(&usenewmatdata);

	if (meshes[CUBE_MESH].internal_id == THE_UNINIT) {
		CreateMesh(CUBE_MESH, mat);
	}
	glBindVertexArray(meshes[CUBE_MESH].internal_id);


	glDepthFunc(GL_LEQUAL);
	glDrawElements(GL_TRIANGLES, meshes[CUBE_MESH].elements, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
}

void THE_DrawExecute(THE_CommandData *data)
{
	THE_ASSERT(data->draw.inst_count > 0, "Set inst count");
	THE_Mesh mesh = data->draw.mesh;
	THE_InternalMesh *im = meshes + mesh;
	THE_InternalShader *s = shaders + data->draw.shader;
	THE_ASSERT(s->program_id, "Uninit shader");

	THE_ASSERT(im->elements, "Attempt to draw an uninitialized mesh");
	if (im->internal_id == THE_UNINIT) {
		CreateMesh(mesh, data->draw.shader);
	}
	glBindVertexArray(im->internal_id);
	/*
	glBindBuffer(GL_ARRAY_BUFFER, im->internal_buffers_id[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im->internal_buffers_id[1]);

	GLint attrib_pos = glGetAttribLocation(s->program_id, "a_position");
	glEnableVertexAttribArray(attrib_pos);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(float), (void*)0);
	glVertexAttribDivisor(attrib_pos, 0);

	// NORMAL
	attrib_pos = glGetAttribLocation(s->program_id, "a_normal"); 
	glEnableVertexAttribArray(attrib_pos);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE,
		8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribDivisor(attrib_pos, 0);

	// UV
	attrib_pos = glGetAttribLocation(s->program_id, "a_uv");
	glEnableVertexAttribArray(attrib_pos);
	glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE,
		8 * sizeof(float), (void*)(6 * sizeof(float)));
	glVertexAttribDivisor(attrib_pos, 0);
	*/

	SetMaterialData(data->draw.shader, data->draw.mat, 1);
	glDrawElements(GL_TRIANGLES, im->elements, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void THE_EquirectToCubeExecute(THE_CommandData *data)
{
	THE_Texture o_cube = data->eqr_cube.out_cube;
	THE_Texture o_pref = data->eqr_cube.out_prefilt;
	THE_Texture o_lut = data->eqr_cube.out_lut;
	const char *path = data->eqr_cube.in_path;
	THE_InternalTexture *icu = textures + o_cube;

	if (icu->cpu_version > icu->gpu_version) {
		CreateCubemap(o_cube);
	}

	//struct mat4 proj = smat4_perspective(to_radians(90.0f), 1.0f, 0.1f, 10.0f);
	float proj[16] = {0.0f};
	mat4_perspective(proj, to_radians(90.0f), 1.0f, 0.1f, 10.0f);

	struct mat4 views[] = {
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(1.0f, 0.0f, 0.0f), svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(-1.0f, 0.0f, 0.0f), svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 1.0f, 0.0f), svec3(0.0f, 0.0f, 1.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, -1.0f, 0.0f), svec3(0.0f, 0.0f, -1.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 0.0f, 1.0f), svec3(0.0f, -1.0f, 0.0f)),
		smat4_look_at(svec3(0.0f, 0.0f, 0.0f), svec3(0.0f, 0.0f, -1.0f), svec3(0.0f, -1.0f, 0.0f)),
	};

	GLuint fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0, 0, icu->width, icu->height);
	// Manually sync framebuffer here after changing textures
	THE_Texture equirec = THE_CreateTexture(path, THE_TEX_RGB_F16);
	CreateTexture(equirec, false);
	THE_CommandData ro;
	ro.renderops.cull_face = THE_CULLFACE_DISABLED;
	ro.renderops.changed_mask = THE_CULL_FACE_BIT;
	THE_RenderOptionsExecute(&ro);

	THE_CommandData dcd;
	dcd.draw.inst_count = 1U;
	dcd.draw.mesh = CUBE_MESH;
	for (int i = 0; i < 6; ++i) {
		float *view = (float*)&views[i];
		float vp[16];
		mat4_multiply(vp, proj, view);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, icu->internal_id, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		dcd.draw.shader = 2; // eq to cube
		dcd.draw.mat = THE_MaterialDefault();
		THE_MaterialSetFrameTexture(&(dcd.draw.mat), &equirec, 1, -1);
		THE_MaterialSetFrameData(&dcd.draw.mat, vp, 16);
		THE_CommandData cmdata;
		cmdata.use_shader = dcd.draw.shader;
		//cmdata.use_shader.material = dcd.draw.mat;
		THE_UseShaderExecute(&cmdata);
		THE_DrawExecute(&dcd);
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
			pref_data.roughness = i / 4.0f;  // mip / max mip levels - 1

			THE_CommandData draw_cd;
			draw_cd.draw.inst_count = 1U;
			draw_cd.draw.mesh = CUBE_MESH;
			for (int j = 0; j < 6; ++j) {
				float *view = (float*)&views[j];
                mat4_multiply(pref_data.vp, proj, view);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				    GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, ipref->internal_id, i);
				glClear(GL_COLOR_BUFFER_BIT);
				draw_cd.draw.shader = 3; // prefilter env
				draw_cd.draw.mat = THE_MaterialDefault();
                THE_MaterialSetFrameTexture(&draw_cd.draw.mat, &o_cube, 1, 0);
                THE_MaterialSetFrameData(&draw_cd.draw.mat, (float*)&pref_data, sizeof(struct THE_PrefilterEnvData) / 4);
				THE_CommandData cmdata;
				cmdata.use_shader = draw_cd.draw.shader;
				//cmdata.use_shader.material = draw_cd.draw.mat;
				THE_UseShaderExecute(&cmdata);
				THE_DrawExecute(&draw_cd);
			}
		}
	}

	if (o_lut != THE_UNINIT) {
		THE_InternalTexture *ilut = textures + o_lut;
		if (ilut->cpu_version > ilut->gpu_version) {
			CreateTexture(o_lut, false);
		}
		glViewport(0, 0, ilut->width, ilut->height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ilut->internal_id, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		THE_CommandData draw_cd;
		draw_cd.draw.mesh = QUAD_MESH;
		draw_cd.draw.inst_count = 1U;
		draw_cd.draw.shader = 4; // THE_MT_LUT_GEN;
		draw_cd.draw.mat = THE_MaterialDefault();
		THE_CommandData cmdata;
		cmdata.use_shader = draw_cd.draw.shader;
		//cmdata.use_shader.material = draw_cd.draw.mat;
		THE_UseShaderExecute(&cmdata);
		THE_DrawExecute(&draw_cd);
	}

	//glDeleteFramebuffers(1, &fb);
	//THE_ReleaseTexture(equirec);
}

void THE_RenderOptionsExecute(THE_CommandData *data)
{
	// Blend options
	if (data->renderops.changed_mask & THE_BLEND_FUNC_BIT) {
		GLuint sfac, dfac;
		switch(data->renderops.sfactor) {
		case THE_BLENDFUNC_ONE:
			sfac = GL_ONE;
			break;

		case THE_BLENDFUNC_SRC_ALPHA:
			sfac = GL_SRC_ALPHA;
			break;

		case THE_BLENDFUNC_ONE_MINUS_SRC_ALPHA:
			sfac = GL_ONE_MINUS_SRC_ALPHA;
			break;

		case THE_BLENDFUNC_ZERO:
			sfac = GL_ZERO;
			break;

		default:
			THE_ASSERT(false, "RenderOption invalid BlendCommand S value");
			return;
		}

		switch(data->renderops.dfactor) {
		case THE_BLENDFUNC_ONE:
			dfac = GL_ONE;
			break;

		case THE_BLENDFUNC_SRC_ALPHA:
			dfac = GL_SRC_ALPHA;
			break;

		case THE_BLENDFUNC_ONE_MINUS_SRC_ALPHA:
			dfac = GL_ONE_MINUS_SRC_ALPHA;
			break;

		case THE_BLENDFUNC_ZERO:
			dfac = GL_ZERO;
			break;

		default:
			THE_ASSERT(false, "RenderOption invalid BlendCommand D value");
			return;
		}

		glBlendFunc(sfac, dfac);
	}

	if (data->renderops.changed_mask & THE_ENABLE_BLEND_BIT) {
		if (data->renderops.blend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	// Cull options
	if (data->renderops.changed_mask & THE_CULL_FACE_BIT) {
		switch(data->renderops.cull_face) {
		case THE_CULLFACE_DISABLED:
			glDisable(GL_CULL_FACE);
			break;

		case THE_CULLFACE_FRONT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;

		case THE_CULLFACE_BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;

		case THE_CULLFACE_FRONT_AND_BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			break;

		default:
			THE_ASSERT(false, "RenderOption invalid CullFace value");
			break;
		}
	}

	// Depth options
	if (data->renderops.changed_mask & THE_WRITE_DEPTH_BIT) {
		// GL_TRUE is 1 and GL_FALSE 0 so this should work...
		glDepthMask(data->renderops.write_depth);
	}
	if (data->renderops.changed_mask & THE_DEPTH_TEST_BIT) {
		if (data->renderops.depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
}

void THE_UseFramebufferExecute(THE_CommandData *data)
{
	if (data->usefb == THE_DEFAULT) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	THE_InternalFramebuffer *ifb = framebuffers + data->usefb;
	GLsizei width;
	GLsizei height;

	if (ifb->gpu_version == 0) {
		THE_ASSERT(data->usefb >= 0 && ifb->cpu_version > 0, "Framebuffer not created");
		glGenFramebuffers(1, (GLuint*)&(ifb->internal_id));
		if (ifb->color_tex >= 0) {
			CreateTexture(ifb->color_tex, false);
		}
		if (ifb->depth_tex >= 0) {
			CreateTexture(ifb->depth_tex, false);
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
			THE_ASSERT(width == (GLsizei)textures[ifb->depth_tex].width &&
                height == (GLsizei)textures[ifb->depth_tex].height,
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
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				textures[ifb->color_tex].internal_id, 0);
		}
		if (ifb->depth_tex >= 0) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
				textures[ifb->depth_tex].internal_id, 0);
		}
		ifb->gpu_version = ifb->cpu_version;
	}
}

#endif // THE_OPENGL