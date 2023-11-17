#include "core/io.h"
#include "core/mem.h"
#include "internalresources.h"
#include "rendercommands.h"
#include "renderer.h"

#include <math.h>
#include <mathc.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC nyas_alloc
#define STBI_REALLOC nyas_realloc
#define STBI_FREE nyas_free
#endif
#include "stb_image.h"

#ifndef TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_MALLOC nyas_alloc
#define TINYOBJ_REALLOC nyas_realloc
#define TINYOBJ_CALLOC nyas_calloc
#define TINYOBJ_FREE nyas_free
#endif
#include "tinyobj_loader_c.h"

nyas_mesh SPHERE_MESH;
nyas_mesh CUBE_MESH;
nyas_mesh QUAD_MESH;

nyas_cmd_queue render_queue;

typedef struct nyas_AvailableNode {
	struct nyas_AvailableNode *next;
	int value;
} nyas_AvailableNode;

static nyas_cmd *curr_pool;
static nyas_cmd *curr_pool_tail;
static nyas_cmd *next_pool;
static nyas_cmd *next_pool_tail;

static int8_t *frame_pool[2];
static int8_t *frame_pool_last;
static int frame_switch;

static void
nyas__file_reader(void *ctx,
                  const char *path,
                  int is_mtl,
                  const char *obj_path,
                  char **buf,
                  size_t *size)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		return;
	}

	fseek(f, 0L, SEEK_END);
	*size = ftell(f);
	rewind(f);

	*buf = nyas_alloc(*size + 1);
	NYAS_ASSERT(*buf && "Allocation failed.");

	if (fread(*buf, *size, 1, f) != 1) {
		NYAS_ASSERT(false && "Read failed.");
	}

	fclose(f);
}

static nyas_mesh
nyas__create_mesh_handle()
{
	NYAS_ASSERT(mesh_count < NYAS_MAX_MESHES && "Max meshes reached");
	return mesh_count++;
}

static nyas_tex
nyas__create_tex_handle()
{
	NYAS_ASSERT(texture_count < NYAS_MAX_TEXTURES && "Max textures reached");
	return texture_count++;
}

static nyas_framebuffer
nyas__create_fb_handle()
{
	NYAS_ASSERT(framebuffer_count < NYAS_MAX_FRAMEBUFFERS &&
	            "Max framebuffers reached");
	return framebuffer_count++;
}

static nyas_shader
nyas__create_shader_handle()
{
	NYAS_ASSERT(shader_count < NYAS_MAX_SHADERS && "Max shaders reached");
	return shader_count++;
}

static nyas_tex
nyas__new_texture()
{
	nyas_tex tex = nyas__create_tex_handle();
	NYAS__CHECK_HANDLE(texture, tex);
	textures[tex].res.id = NYAS_UNINIT;
	textures[tex].res.flags = RF_DIRTY | RF_FREE_AFTER_LOAD;
	for (int face = 0; face < 6; ++face) {
		textures[tex].pix[face] = NULL;
	}
	return tex;
}

static nyas_mesh
nyas__new_mesh()
{
	nyas_mesh mesh = nyas__create_mesh_handle();
	meshes[mesh].res.id = NYAS_UNINIT;
	meshes[mesh].res.flags = RF_DIRTY;
	meshes[mesh].attr_flags = 0;
	meshes[mesh].vtx = NULL;
	meshes[mesh].idx = NULL;
	meshes[mesh].vtx_size = 0;
	meshes[mesh].elements = 0;
	meshes[mesh].internal_buffers_id[0] = NYAS_UNINIT;
	meshes[mesh].internal_buffers_id[1] = NYAS_UNINIT;
	return mesh;
}

static nyas_framebuffer
nyas__new_framebuffer()
{
	nyas_framebuffer fb = nyas__create_fb_handle();
	framebuffers[fb].res.id = NYAS_UNINIT;
	framebuffers[fb].res.flags = RF_DIRTY;
	framebuffers[fb].color_tex = NYAS_INACTIVE;
	framebuffers[fb].depth_tex = NYAS_INACTIVE;
	return fb;
}

void
nyas_px_init()
{
	curr_pool = nyas_mem_reserve(NYAS_MB(16));
	next_pool = nyas_mem_reserve(NYAS_MB(16));
	curr_pool_tail = curr_pool;
	next_pool_tail = next_pool;

	meshes = nyas_mem_reserve(sizeof(*meshes) * NYAS_MAX_MESHES);
	textures = nyas_mem_reserve(sizeof(*textures) * NYAS_MAX_TEXTURES);
	framebuffers =
	  nyas_mem_reserve(sizeof(*framebuffers) * NYAS_MAX_FRAMEBUFFERS);
	shaders = nyas_mem_reserve(sizeof(*shaders) * NYAS_MAX_SHADERS);
	mesh_count = 0;
	texture_count = 1;
	framebuffer_count = 0;
	shader_count = 0;

	/*
	2 Frame allocator (2 frame since is the lifetime of render resources)
	first frame making the render queue and second frame for the actual
	render Frame_pool[0] is the entire buffer and frame_pool[1] is a ptr to
	the half of it that way we can alternate freeing only one half each
	frame so it is synced with the render queues
	*/
	frame_pool[0] = nyas_mem_reserve(NYAS_FRAME_POOL_SIZE);
	frame_pool[1] = frame_pool[0] + NYAS_FRAME_POOL_SIZE / 2;
	frame_pool_last = frame_pool[0];
	frame_switch = 0;

	SPHERE_MESH = nyas_mesh_create_sphere(32, 32);
	CUBE_MESH = nyas_mesh_create_cube();
	QUAD_MESH = nyas_mesh_create_quad();
}

/*
 * Concatenates a list of commands to the render queue.
 */
void
nyas_cmd_add(nyas_cmd *rc)
{
	if (render_queue.next_last) {
		render_queue.next_last->next = rc;
	} else {
		render_queue.next = rc;
	}

	nyas_cmd *c = NULL;
	for (c = rc; c->next != NULL; c = c->next)
		;
	render_queue.next_last = c;
}

void
nyas_px_render(void)
{
	nyas_cmd *i = render_queue.curr;
	if (!i) {
		return;
	}

	i->execute(&(i->data));
	while (i != render_queue.curr_last) {
		i = i->next;
		i->execute(&(i->data));
	}
	render_queue.curr = NULL;
	render_queue.curr_last = NULL;
	// TODO delete resources marked for release
}

void
nyas_frame_end(void)
{
	render_queue.curr = render_queue.next;
	render_queue.curr_last = render_queue.next_last;
	render_queue.next = NULL;
	render_queue.next_last = NULL;

	nyas_cmd *tmp = curr_pool;
	curr_pool = next_pool;
	curr_pool_tail = next_pool_tail;
	next_pool = tmp;
	next_pool_tail = tmp; /* Free rendered commands */
	memset(next_pool, '\0', NYAS_RENDER_QUEUE_CAPACITY * sizeof(nyas_cmd));

	frame_switch = !frame_switch;
	frame_pool_last = frame_pool[frame_switch];
	memset(frame_pool_last, '\0', NYAS_FRAME_POOL_SIZE / 2);
}

nyas_cmd *
nyas_cmd_alloc()
{
	NYAS_ASSERT((next_pool_tail - next_pool) <
	              NYAS_RENDER_QUEUE_CAPACITY - 1 &&
	            "Not enough memory in the RenderQueue pool");
	return next_pool_tail++;
}

void *
nyas_alloc_frame(unsigned int size)
{
	NYAS_ASSERT(((frame_pool_last + size) - frame_pool[frame_switch]) <
	              NYAS_FRAME_POOL_SIZE / 2 &&
	            "Not enough memory in the frame pool");
	void *ret = frame_pool_last;
	frame_pool_last += size;
	return ret;
}

nyas_tex
nyas_tex_load_img(const char *path, enum nyas_textype t)
{
	NYAS_ASSERT(*path != '\0' && "For empty textures use nyas_tex_create");

	nyas_tex tex = nyas__new_texture();
	textures[tex].type = t;

	int nchannels = 0;
	int *width = &textures[tex].width;
	int *height = &textures[tex].height;
	stbi_set_flip_vertically_on_load(1);

	switch (textures[tex].type) {
	case NYAS_TEX_RGB_F16:
	case NYAS_TEX_RGBA_F16:
	case NYAS_TEX_LUT:
		textures[tex].pix[0] = stbi_loadf(path, width, height, &nchannels, 0);
		NYAS_ASSERT(textures[tex].pix[0] && "The image couldn't be loaded");
		break;

	case NYAS_TEX_RGB:
	case NYAS_TEX_SRGB:
		nchannels = 3;
		textures[tex].pix[0] = stbi_load(path, width, height, &nchannels, 3);
		NYAS_ASSERT(textures[tex].pix[0] && "The image couldn't be loaded.");
		break;

	case NYAS_TEX_R:
		nchannels = 1;
		textures[tex].pix[0] = stbi_load(path, width, height, &nchannels, 1);
		NYAS_ASSERT(textures[tex].pix[0] && "The image couldn't be loaded.");
		break;

	case NYAS_TEX_SKYBOX:
		stbi_set_flip_vertically_on_load(0);
		const char *cube_prefix = "RLUDFB";
		char path_buffer[512];
		memset(path_buffer, '\0', 512);
		strcpy(path_buffer, path);
		for (int i = 0; i < 6; ++i) {
			path_buffer[13] = cube_prefix[i];
			textures[tex].pix[i] = stbi_load(path_buffer, width, height,
			                                 &nchannels, 0);
			NYAS_ASSERT(textures[tex].pix[i] &&
			            "Couldn't load the image to the cubemap");
		}
		break;

	default:
		NYAS_ASSERT(false && "Default case LoadTexture.");
		return NYAS_UNINIT;
	}

	return tex;
}

nyas_tex
nyas_tex_create(int width, int height, enum nyas_textype t)
{
	NYAS_ASSERT(width > 0 && height > 0 && "Incorrect dimensions");

	nyas_tex tex = nyas__new_texture();
	textures[tex].width = width;
	textures[tex].height = height;
	textures[tex].type = t;

	return tex;
}

int *
nyas_tex_size(nyas_tex tex, int *out)
{
	out[0] = textures[tex].width;
	out[1] = textures[tex].height;
	return out;
}

void
nyas_tex_freepix(nyas_tex tex)
{
	for (int face = 0; face < 6; ++face) {
		if (textures[tex].pix[face]) {
			stbi_image_free(textures[tex].pix[face]);
			textures[tex].pix[face] = NULL;
		}
	}
}

nyas_shader
nyas_shader_create(const char *shader)
{
	nyas_shader ret = nyas__create_shader_handle();
	shaders[ret].shader_name = shader;
	shaders[ret].res.id = NYAS_UNINIT;
	shaders[ret].res.flags = RF_DIRTY;
	return ret;
}

nyas_mesh
nyas_mesh_create_cube()
{
	static float VERTICES[] = {
		// positions          // normals           // uv
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,
		0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		-0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

		-0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,
		0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
		0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f,

		-0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	};

	static uint32_t INDICES[] = {
		0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
		13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
	};

	nyas_mesh ret = nyas__new_mesh();
	meshes[ret].res.flags = RF_DIRTY;
	meshes[ret].attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV);
	meshes[ret].vtx = &VERTICES[0];
	meshes[ret].idx = &INDICES[0];
	meshes[ret].vtx_size = sizeof(VERTICES);
	meshes[ret].elements = sizeof(INDICES) / sizeof(*INDICES);

	return ret;
}

nyas_mesh
nyas_mesh_create_sphere(int32_t y_segments, int32_t x_segments)
{
	NYAS_ASSERT(y_segments > 2 && x_segments > 2 &&
	            "Invalid number of segments");

	const float x_step = 1.0f / (float)(y_segments - 1);
	const float y_step = 1.0f / (float)(x_segments - 1);

	nyas_mesh ret = nyas__new_mesh();
	meshes[ret].res.flags = RF_DIRTY | RF_FREE_AFTER_LOAD;
	meshes[ret].attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV);
	meshes[ret].vtx_size = y_segments * x_segments * 8 * sizeof(float);
	meshes[ret].elements = y_segments * x_segments * 6;
	meshes[ret].vtx = nyas_alloc(meshes[ret].vtx_size);
	meshes[ret].idx = nyas_alloc(meshes[ret].elements * sizeof(uint32_t));

	float *v = meshes[ret].vtx;
	for (int y = 0; y < x_segments; ++y) {
		float py = sinf((float)-M_PI_2 + (float)M_PI * (float)y * x_step);
		for (int x = 0; x < y_segments; ++x) {
			float px = cosf(M_PI * 2.0f * x * y_step) *
			  sinf(M_PI * y * x_step);
			float pz = sinf(M_PI * 2.0f * x * y_step) *
			  sinf(M_PI * y * x_step);

			*v++ = px;
			*v++ = py;
			*v++ = pz;
			*v++ = px;
			*v++ = py;
			*v++ = pz;
			*v++ = (float)x * y_step;
			*v++ = (float)y * x_step;
		}
	}

	uint32_t *i = meshes[ret].idx;
	for (int y = 0; y < x_segments; ++y) {
		for (int x = 0; x < y_segments; ++x) {
			*i++ = y * y_segments + x;
			*i++ = y * y_segments + x + 1;
			*i++ = (y + 1) * y_segments + x + 1;
			*i++ = y * y_segments + x;
			*i++ = (y + 1) * y_segments + x + 1;
			*i++ = (y + 1) * y_segments + x;
		}
	}

	return ret;
}

nyas_mesh
nyas_mesh_create_quad()
{
	static float VERTICES[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		1.0f,  -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	};

	static unsigned int INDICES[] = { 0, 1, 2, 0, 2, 3 };

	nyas_mesh mesh = nyas__new_mesh();
	meshes[mesh].res.flags = RF_DIRTY;
	meshes[mesh].attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) |
	  (1 << A_UV);
	meshes[mesh].vtx = &VERTICES[0];
	meshes[mesh].idx = &INDICES[0];
	meshes[mesh].vtx_size = sizeof(VERTICES);
	meshes[mesh].elements = sizeof(INDICES) / sizeof(*INDICES);

	return mesh;
}

nyas_mesh
nyas_mesh_load_obj(const char *path)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t *shapes = NULL;
	size_t shape_count;
	tinyobj_material_t *mats = NULL;
	size_t mats_count;

	int result = tinyobj_parse_obj(&attrib, &shapes, &shape_count, &mats,
	                               &mats_count, path, nyas__file_reader, NULL,
	                               TINYOBJ_FLAG_TRIANGULATE);

	NYAS_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
	if (result != TINYOBJ_SUCCESS) {
		NYAS_LOG_ERR("Error loading obj. Err: %d", result);
		return CUBE_MESH;
	}

	size_t tri_count = attrib.num_face_num_verts;
	size_t vertices_count = tri_count * 3 * 14;
	size_t indices_count = tri_count * 3;
	float *vertices = nyas_alloc(vertices_count * sizeof(*vertices));
	IDX_T *indices = nyas_alloc(indices_count * sizeof(*indices));
	float *vit = vertices;
	size_t ii = 0; // TODO: Indices right.

	size_t index_offset = 0;
	for (size_t i = 0; i < attrib.num_face_num_verts; ++i) {
		for (size_t f = 0; f < attrib.face_num_verts[i] / 3; ++f) {
			tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset++];
			float v1[14], v2[14], v3[14];

			v1[0] = attrib.vertices[3 * idx.v_idx + 0];
			v1[1] = attrib.vertices[3 * idx.v_idx + 1];
			v1[2] = attrib.vertices[3 * idx.v_idx + 2];
			v1[3] = attrib.normals[3 * idx.vn_idx + 0];
			v1[4] = attrib.normals[3 * idx.vn_idx + 1];
			v1[5] = attrib.normals[3 * idx.vn_idx + 2];
			v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset++];
			v2[0] = attrib.vertices[3 * idx.v_idx + 0];
			v2[1] = attrib.vertices[3 * idx.v_idx + 1];
			v2[2] = attrib.vertices[3 * idx.v_idx + 2];
			v2[3] = attrib.normals[3 * idx.vn_idx + 0];
			v2[4] = attrib.normals[3 * idx.vn_idx + 1];
			v2[5] = attrib.normals[3 * idx.vn_idx + 2];
			v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset++];
			v3[0] = attrib.vertices[3 * idx.v_idx + 0];
			v3[1] = attrib.vertices[3 * idx.v_idx + 1];
			v3[2] = attrib.vertices[3 * idx.v_idx + 2];
			v3[3] = attrib.normals[3 * idx.vn_idx + 0];
			v3[4] = attrib.normals[3 * idx.vn_idx + 1];
			v3[5] = attrib.normals[3 * idx.vn_idx + 2];
			v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			// Calculate tangent and bitangent
			struct vec3 delta_p1 = svec3_subtract(svec3(v2[0], v2[1], v2[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec3 delta_p2 = svec3_subtract(svec3(v3[0], v3[1], v3[2]),
			                                      svec3(v1[0], v1[1], v1[2]));
			struct vec2 delta_uv1 = svec2_subtract(svec2(v2[12], v2[13]),
			                                       svec2(v1[12], v1[13]));
			struct vec2 delta_uv2 = svec2_subtract(svec2(v3[12], v3[13]),
			                                       svec2(v1[12], v1[13]));
			float r = 1.0f /
			  (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
			struct vec3 tan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p1, delta_uv2.y),
			                 svec3_multiply_f(delta_p2, delta_uv1.y)),
			  r);
			struct vec3 bitan = svec3_multiply_f(
			  svec3_subtract(svec3_multiply_f(delta_p2, delta_uv1.x),
			                 svec3_multiply_f(delta_p1, delta_uv2.x)),
			  r);

			v1[6] = tan.x;
			v1[7] = tan.y;
			v1[8] = tan.z;
			v2[6] = tan.x;
			v2[7] = tan.y;
			v2[8] = tan.z;
			v3[6] = tan.x;
			v3[7] = tan.y;
			v3[8] = tan.z;

			v1[9] = bitan.x;
			v1[10] = bitan.y;
			v1[11] = bitan.z;
			v2[9] = bitan.x;
			v2[10] = bitan.y;
			v2[11] = bitan.z;
			v3[9] = bitan.x;
			v3[10] = bitan.y;
			v3[11] = bitan.z;

			for (int j = 0; j < 14; ++j) {
				*vit++ = v1[j];
			}

			for (int j = 0; j < 14; ++j) {
				*vit++ = v2[j];
			}

			for (int j = 0; j < 14; ++j) {
				*vit++ = v3[j];
			}

			for (int j = 0; j < 3; ++j, ++ii) {
				indices[ii] = ii;
			}
		}
	}

	nyas_mesh mesh = nyas__new_mesh();
	meshes[mesh].res.flags = RF_DIRTY | RF_FREE_AFTER_LOAD;
	meshes[mesh].attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) |
	  (1 << A_TANGENT) | (1 << A_BITANGENT) | (1 << A_UV);
	meshes[mesh].vtx = vertices;
	meshes[mesh].idx = indices;
	meshes[mesh].vtx_size = vertices_count * sizeof(meshes[mesh].vtx[0]);
	meshes[mesh].elements = indices_count;

	return mesh;
}

nyas_framebuffer
nyas_fb_create(int width, int height, bool color, bool depth)
{
	NYAS_ASSERT(width > 0 && height > 0 && "Invalid dimensions");
	nyas_framebuffer fb = nyas__new_framebuffer();

	if (color) {
		framebuffers[fb].color_tex = nyas_tex_create(width, height,
		                                             NYAS_TEX_RGBA_F16);
	}

	if (depth) {
		framebuffers[fb].depth_tex = nyas_tex_create(width, height,
		                                             NYAS_TEX_DEPTH);
	}

	return fb;
}

nyas_tex
nyas_fb_color(nyas_framebuffer fb)
{
	return framebuffers[fb].color_tex;
}

void
nyas_fb_size(nyas_framebuffer fb, int *w, int *h)
{
	NYAS__CHECK_HANDLE(framebuffer, fb);
	nypx_ifb *ifb = framebuffers + fb;
	if (ifb->color_tex != NYAS_IGNORE) {
		NYAS__CHECK_HANDLE(texture, ifb->color_tex);
		nypx_itex *itex = textures + ifb->color_tex;
		*w = itex->width;
		*h = itex->height;
	} else if (ifb->depth_tex != NYAS_IGNORE) {
		NYAS__CHECK_HANDLE(texture, ifb->depth_tex);
		nypx_itex *itex = textures + ifb->depth_tex;
		*w = itex->width;
		*h = itex->height;
	} else {
		*w = 0;
		*h = 0;
	}
}

nyas_mat
nyas_mat_default(void)
{
	nyas_mat ret = { .ptr = NULL,
		             .data_count = 0,
		             .tex_count = 0,
		             .cube_count = 0,
		             .shader = NYAS_INVALID };
	return ret;
}

void *
nyas_mat_alloc(nyas_mat *mat)
{
	int elements = mat->data_count + mat->tex_count + mat->cube_count;
	mat->ptr = nyas_alloc(elements * sizeof(float));
	return mat->ptr;
}

void *
nyas_mat_alloc_frame(nyas_mat *m)
{
	int elements = m->data_count + m->tex_count + m->cube_count;
	m->ptr = nyas_alloc_frame(elements * sizeof(float));
	return m->ptr;
}
