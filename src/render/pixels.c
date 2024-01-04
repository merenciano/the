#include "pixels.h"

#include "core/io.h"
#include "core/log.h"
#include "core/mem.h"
#include "pixels_internal.h"

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

typedef struct nyas_internal_shader shdr_t;
typedef struct nyas_internal_texture tex_t;

nyas_mesh SPHERE_MESH;
nyas_mesh CUBE_MESH;
nyas_mesh QUAD_MESH;

nyas_arr mesh_pool;
nyas_arr tex_pool;
nyas_arr shader_pool;
nyas_arr framebuffer_pool;

cmd_queue render_queue;

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
	(void)ctx;
	(void)is_mtl;
	(void)obj_path;

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
nyas__create_mesh_handle(void)
{
	nyas_mesh mesh = (nyas_mesh)nyas_arr_len(mesh_pool);
	nyas_arr_push(&mesh_pool);
	return mesh;
}

static nyas_tex
nyas__create_tex_handle(void)
{
	nyas_tex tex = (nyas_tex)nyas_arr_len(tex_pool);
	nyas_arr_push(&tex_pool);
	return tex;
}

static nyas_framebuffer
nyas__create_fb_handle(void)
{
	nyas_framebuffer fb = (nyas_framebuffer)nyas_arr_len(framebuffer_pool);
	nyas_arr_push(&framebuffer_pool);
	return fb;
}

static nyas_shader
nyas__create_shader_handle(void)
{
	nyas_shader shdr = (nyas_shader)nyas_arr_len(shader_pool);
	nyas_arr_push(&shader_pool);
	return shdr;
}

static nyas_mesh
nyas__new_mesh(void)
{
	nyas_mesh mesh = nyas__create_mesh_handle();
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	imsh->res.id = NYAS_UNINIT;
	imsh->res.flags = RF_DIRTY;
	imsh->attr_flags = 0;
	imsh->vtx = NULL;
	imsh->idx = NULL;
	imsh->vtx_size = 0;
	imsh->elements = 0;
	imsh->internal_buffers_id[0] = NYAS_UNINIT;
	imsh->internal_buffers_id[1] = NYAS_UNINIT;
	return mesh;
}

static nyas_framebuffer
nyas__new_framebuffer(void)
{
	nyas_framebuffer fb = nyas__create_fb_handle();
	r_fb *ifb = nyas_arr_at(framebuffer_pool, fb);
	ifb->res.id = NYAS_UNINIT;
	ifb->res.flags = RF_DIRTY;
	ifb->color_tex = NYAS_INACTIVE;
	ifb->depth_tex = NYAS_INACTIVE;
	return fb;
}

void
nyas_px_init(void)
{
	curr_pool = nyas_mem_reserve(NYAS_MB(16));
	next_pool = nyas_mem_reserve(NYAS_MB(16));
	curr_pool_tail = curr_pool;
	next_pool_tail = next_pool;

	mesh_pool = nyas_arr_create(NYAS_MESH_RESERVE, sizeof(r_mesh));
	tex_pool = nyas_arr_create(NYAS_TEX_RESERVE, sizeof(tex_t));
	framebuffer_pool = nyas_arr_create(NYAS_FB_RESERVE, sizeof(r_fb));
	shader_pool = nyas_arr_create(NYAS_SHADER_RESERVE, sizeof(shdr_t));

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
nyas_cmd_alloc(void)
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

int
nyas_tex_flags(int ch,
               bool fp,
               bool lin,
               bool cube,
               bool depth,
               bool tile,
               bool mips)
{
	int flags = ((ch - 1) & 0x03); // TODO: Cambiar a que 0 sean 4
	flags |= (TF_FLOAT * fp) | (TF_CUBE * cube) | (TF_DEPTH * depth);
	flags |= (TF_TILING * tile) | (TF_MIPMAP * mips) | (TF_LINEAR_COLOR * lin);
	flags |= TF_MAG_FILTER_LERP | TF_MIN_FILTER_LERP;
	flags |= (TF_MAG_MIP_FILTER_LERP * mips);
	return flags;
}

const char *
nyas__face_img_path(const char *path, const char *suffixes, int face)
{
	static char buffer[512];
	if (!face) {
		if (!suffixes) {
			return path;
		} else {
			memset(buffer, 0, 512);
		}
	}
	int count = snprintf(buffer, 512, path, suffixes[face]);
	NYAS_ASSERT(count <= 512 && "Xpand dat mf");
	return buffer;
}

nyas_tex
nyas_tex_load(const char *path, int flip, int flags)
{
	NYAS_ASSERT(*path != '\0' && "For empty textures use nyas_tex_empty");

	nyas_tex tex = nyas__create_tex_handle();
	CHECK_HANDLE(tex, tex);
	tex_t *t = nyas_arr_at(tex_pool, tex);
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->type = flags;
	int fmtchannels = t->type & 0x03; // 2 lsb for nchannels per pixel (0~4)
	++fmtchannels;

	int nchannels = fmtchannels;
	int *width = &t->width;
	int *height = &t->height;
	int faces = flags & TF_CUBE ? 6 : 1;
	const char *suffixes = flags & TF_CUBE ? "RLUDFB" : NULL;

	stbi_set_flip_vertically_on_load(flip);
	if (flags & TF_FLOAT) {
		for (int i = 0; i < faces; ++i) {
			const char *p = nyas__face_img_path(path, suffixes, i);
			t->pix[i] = stbi_loadf(p, width, height, &nchannels, 0);
			NYAS_ASSERT(t->pix[i] && "The image couldn't be loaded");
		}
	} else {
		for (int i = 0; i < faces; ++i) {
			const char *p = nyas__face_img_path(path, suffixes, i);
			t->pix[i] = stbi_load(p, width, height, &nchannels, fmtchannels);
			NYAS_ASSERT(t->pix[i] && "The image couldn't be loaded");
		}
	}

	return tex;
}

nyas_tex
nyas_tex_empty(int width, int height, int tex_flags)
{
	NYAS_ASSERT(width > 0 && height > 0 && "Incorrect dimensions");
	nyas_tex tex = nyas__create_tex_handle();
	CHECK_HANDLE(tex, tex);
	tex_t *t = nyas_arr_at(tex_pool, tex);
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->width = width;
	t->height = height;
	t->type = tex_flags;

	for (int face = 0; face < 6; ++face) {
		t->pix[face] = NULL;
	}

	return tex;
}

int *
nyas_tex_size(nyas_tex tex, int *out)
{
	out[0] = ((tex_t *)nyas_arr_at(tex_pool, tex))->width;
	out[1] = ((tex_t *)nyas_arr_at(tex_pool, tex))->height;
	return out;
}

nyas_shader
nyas_shader_create(const nyas_shader_desc *desc)
{
	nyas_shader ret = nyas__create_shader_handle();
	shdr_t *shdr = nyas_arr_at(shader_pool, ret);
	shdr->name = desc->name;
	shdr->res.id = 0;
	shdr->res.flags = 0;
	shdr->count[0].data = desc->data_count;
	shdr->count[0].tex = desc->tex_count;
	shdr->count[0].cubemap = desc->cubemap_count;
	shdr->count[1].data = desc->common_data_count;
	shdr->count[1].tex = desc->common_tex_count;
	shdr->count[1].cubemap = desc->common_cubemap_count;
	shdr->common =
	  nyas_alloc((desc->common_data_count + desc->common_tex_count +
	              desc->common_cubemap_count) *
	             sizeof(float));
	return ret;
}

void *
nyas_shader_data(nyas_shader shader)
{
	shdr_t *shdr = nyas_arr_at(shader_pool, shader);
	return shdr->common;
}

nyas_tex *
nyas_shader_tex(nyas_shader shader)
{
	shdr_t *shdr = nyas_arr_at(shader_pool, shader);
	return (nyas_tex *)shdr->common + shdr->count[1].data;
}

nyas_tex *
nyas_shader_cubemap(nyas_shader shader)
{
	shdr_t *shdr = nyas_arr_at(shader_pool, shader);
	return nyas_shader_tex(shader) + shdr->count[1].tex;
}

void
nyas_shader_reload(nyas_shader shader)
{
	shdr_t *shdr = nyas_arr_at(shader_pool, shader);
	shdr->res.flags |= RF_DIRTY;
}

nyas_mesh
nyas_mesh_create_cube(void)
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

	static nyas_idx INDICES[] = {
		0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
		13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
	};

	nyas_mesh ret = nyas__new_mesh();
	r_mesh *imsh = nyas_arr_at(mesh_pool, ret);
	imsh->res.flags = RF_DIRTY;
	imsh->attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV);
	imsh->vtx = &VERTICES[0];
	imsh->idx = &INDICES[0];
	imsh->vtx_size = sizeof(VERTICES);
	imsh->elements = sizeof(INDICES) / sizeof(*INDICES);

	return ret;
}

nyas_mesh
nyas_mesh_create_sphere(int y_segments, int x_segments)
{
	NYAS_ASSERT(y_segments > 2 && x_segments > 2 &&
	            "Invalid number of segments");

	const float x_step = 1.0f / (float)(y_segments - 1);
	const float y_step = 1.0f / (float)(x_segments - 1);

	nyas_mesh ret = nyas__new_mesh();
	r_mesh *imsh = nyas_arr_at(mesh_pool, ret);
	imsh->res.flags = RF_DIRTY | RF_FREE_AFTER_LOAD;
	imsh->attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV);
	imsh->vtx_size = y_segments * x_segments * 8 * sizeof(float);
	imsh->elements = y_segments * x_segments * 6;
	imsh->vtx = nyas_alloc(imsh->vtx_size);
	imsh->idx = nyas_alloc(imsh->elements * sizeof(nyas_idx));

	float *v = imsh->vtx;
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

	nyas_idx *i = imsh->idx;
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
nyas_mesh_create_quad(void)
{
	static float VERTICES[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		1.0f,  -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	};

	static nyas_idx INDICES[] = { 0, 1, 2, 0, 2, 3 };

	nyas_mesh mesh = nyas__new_mesh();
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	imsh->res.flags = RF_DIRTY;
	imsh->attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_UV);
	imsh->vtx = &VERTICES[0];
	imsh->idx = &INDICES[0];
	imsh->vtx_size = sizeof(VERTICES);
	imsh->elements = sizeof(INDICES) / sizeof(*INDICES);

	return mesh;
}

static nyas_idx
check_vertex(float *v, float *end, float *newvtx)
{
	nyas_idx i = 0;
	for (; v < end; ++i, v += 14) {
		if ((v[0] == newvtx[0]) && (v[1] == newvtx[1]) &&
		    (v[2] == newvtx[2]) && (v[3] == newvtx[3]) &&
		    (v[4] == newvtx[4]) && (v[5] == newvtx[5]) &&
		    (v[12] == newvtx[12]) && (v[13] == newvtx[13])) {
			return i;
		}
	}
	return i;
}

void
nyas_mesh_load_obj(nyas_mesh mesh, const char *path)
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
	}

	size_t vertex_count = attrib.num_face_num_verts * 3;
	size_t indices_count = vertex_count;
	float *vertices = nyas_alloc(vertex_count * 14 * sizeof(float));
	nyas_idx *indices = nyas_alloc(indices_count * sizeof(*indices));
	float *vit = vertices;

	size_t index_offset = 0;
	for (size_t i = 0; i < attrib.num_face_num_verts; ++i) {
		for (int f = 0; f < attrib.face_num_verts[i] / 3; ++f) {
			tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset];
			float v1[14], v2[14], v3[14];

			v1[0] = attrib.vertices[3 * idx.v_idx + 0];
			v1[1] = attrib.vertices[3 * idx.v_idx + 1];
			v1[2] = attrib.vertices[3 * idx.v_idx + 2];
			v1[3] = attrib.normals[3 * idx.vn_idx + 0];
			v1[4] = attrib.normals[3 * idx.vn_idx + 1];
			v1[5] = attrib.normals[3 * idx.vn_idx + 2];
			v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset + 1];
			v2[0] = attrib.vertices[3 * idx.v_idx + 0];
			v2[1] = attrib.vertices[3 * idx.v_idx + 1];
			v2[2] = attrib.vertices[3 * idx.v_idx + 2];
			v2[3] = attrib.normals[3 * idx.vn_idx + 0];
			v2[4] = attrib.normals[3 * idx.vn_idx + 1];
			v2[5] = attrib.normals[3 * idx.vn_idx + 2];
			v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset + 2];
			v3[0] = attrib.vertices[3 * idx.v_idx + 0];
			v3[1] = attrib.vertices[3 * idx.v_idx + 1];
			v3[2] = attrib.vertices[3 * idx.v_idx + 2];
			v3[3] = attrib.normals[3 * idx.vn_idx + 0];
			v3[4] = attrib.normals[3 * idx.vn_idx + 1];
			v3[5] = attrib.normals[3 * idx.vn_idx + 2];
			v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			// Calculate tangent and bitangent
			float delta_p1[3], delta_p2[3], delta_uv1[2], delta_uv2[2];
			vec3_subtract(delta_p1, v2, v1);
			vec3_subtract(delta_p2, v3, v1);
			vec2_subtract(delta_uv1, &v2[12], &v1[12]);
			vec2_subtract(delta_uv2, &v3[12], &v1[12]);
			float r = 1.0f /
			  (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

			float tn[3], bitn[3], tmp[3];
			vec3_multiply_f(tn, delta_p1, delta_uv2[1]);
			vec3_multiply_f(tmp, delta_p2, delta_uv1[1]);
			vec3_multiply_f(tn, vec3_subtract(tn, tn, tmp), r);

			vec3_multiply_f(bitn, delta_p2, delta_uv1[0]);
			vec3_multiply_f(tmp, delta_p1, delta_uv2[0]);
			vec3_multiply_f(bitn, vec3_subtract(bitn, bitn, tmp), r);

			v1[6] = tn[0];
			v1[7] = tn[1];
			v1[8] = tn[2];
			v2[6] = tn[0];
			v2[7] = tn[1];
			v2[8] = tn[2];
			v3[6] = tn[0];
			v3[7] = tn[1];
			v3[8] = tn[2];

			v1[9] = bitn[0];
			v1[10] = bitn[1];
			v1[11] = bitn[2];
			v2[9] = bitn[0];
			v2[10] = bitn[1];
			v2[11] = bitn[2];
			v3[9] = bitn[0];
			v3[10] = bitn[1];
			v3[11] = bitn[2];

			// Check vertex rep
			nyas_idx nxt_idx = check_vertex(vertices, vit, v1);
			indices[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - vertices)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v1[j];
				}
			}

			nxt_idx = check_vertex(vertices, vit, v2);
			indices[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - vertices)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v2[j];
				}
			}

			nxt_idx = check_vertex(vertices, vit, v3);
			indices[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - vertices)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v3[j];
				}
			}
		}
	}

	int attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_TANGENT) |
	  (1 << A_BITANGENT) | (1 << A_UV);
	nyas_mesh_set_vertices(mesh, vertices, (vit - vertices) * sizeof(float),
	                       attr_flags);
	nyas_mesh_set_indices(mesh, indices, indices_count);

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, shape_count);
	tinyobj_materials_free(mats, mats_count);
}

void
nyas_mesh_load_msh(nyas_mesh mesh, const char *path)
{
	int attr_flags = (1 << A_POSITION) | (1 << A_NORMAL) | (1 << A_TANGENT) |
	  (1 << A_BITANGENT) | (1 << A_UV);
	char *data;
	size_t sz;
	nyas__file_reader(NULL, path, 0, NULL, &data, &sz);
	size_t vsz = *(size_t *)data;
	data += sizeof(size_t);
	nyas_mesh_set_vertices(mesh, (void *)data, vsz, attr_flags);
	data += vsz;

	size_t isz = *(size_t *)data;
	data += sizeof(size_t);
	nyas_mesh_set_indices(mesh, (void *)data, isz / sizeof(nyas_idx));
}

void
nyas_mesh_set_vertices(nyas_mesh mesh, float *v, size_t size, int vattr)
{
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	imsh->res.flags = RF_DIRTY; //| RF_FREE_AFTER_LOAD;
	imsh->attr_flags = vattr;
	imsh->vtx = v;
	imsh->vtx_size = size;
}

void
nyas_mesh_set_indices(nyas_mesh mesh, nyas_idx *indices, size_t elements)
{
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	imsh->res.flags = RF_DIRTY;
	imsh->idx = indices;
	imsh->elements = elements;
}

nyas_mesh
nyas_mesh_create(void)
{
	return nyas__new_mesh();
}

nyas_framebuffer
nyas_fb_create(int width, int height, bool color, bool depth)
{
	NYAS_ASSERT(width > 0 && height > 0 && "Invalid dimensions");
	nyas_framebuffer fb = nyas__new_framebuffer();
	r_fb *ifb = nyas_arr_at(framebuffer_pool, fb);

	if (color) {
		int texflags = (TF_FLOAT | TF_MAG_FILTER_LERP | TF_MIN_FILTER_LERP |
		                0x3); // 0x3 = 4channels
		ifb->color_tex = nyas_tex_empty(width, height, texflags);
	}

	if (depth) {
		ifb->depth_tex = nyas_tex_empty(width, height, TF_DEPTH);
	}

	return fb;
}

nyas_tex
nyas_fb_color(nyas_framebuffer fb)
{
	return ((r_fb *)nyas_arr_at(framebuffer_pool, fb))->color_tex;
}

void
nyas_fb_size(nyas_framebuffer fb, int *w, int *h)
{
	CHECK_HANDLE(framebuffer, fb);
	r_fb *ifb = nyas_arr_at(framebuffer_pool, fb);
	if (ifb->color_tex != NYAS_IGNORE) {
		CHECK_HANDLE(tex, ifb->color_tex);
		tex_t *itex = nyas_arr_at(tex_pool, ifb->color_tex);
		*w = itex->width;
		*h = itex->height;
	} else if (ifb->depth_tex != NYAS_IGNORE) {
		CHECK_HANDLE(tex, ifb->depth_tex);
		tex_t *itex = nyas_arr_at(tex_pool, ifb->depth_tex);
		*w = itex->width;
		*h = itex->height;
	} else {
		*w = 0;
		*h = 0;
	}
}

nyas_mat
nyas_mat_dft(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	return ret;
}

nyas_mat
nyas_mat_pers(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = nyas_arr_at(shader_pool, shader);
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = nyas_alloc(elements * sizeof(float));
	return ret;
}

nyas_mat
nyas_mat_tmp(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = nyas_arr_at(shader_pool, shader);
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = nyas_alloc_frame(elements * sizeof(float));
	return ret;
}

nyas_mat
nyas_mat_from_shader(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = nyas_arr_at(shader_pool, shader);
	int elements = s->count[1].data + s->count[1].tex + s->count[1].cubemap;
	ret.ptr = nyas_alloc_frame(elements * sizeof(float));
	memcpy(ret.ptr, s->common, elements * sizeof(float));
	return ret;
}

void *
nyas_mat_alloc(nyas_mat *mat)
{
	shdr_t *s = nyas_arr_at(shader_pool, mat->shader);
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	mat->ptr = nyas_alloc(elements * sizeof(float));
	return mat->ptr;
}

nyas_tex *
nyas_mat_tex(nyas_mat *mat)
{
	shdr_t *s = nyas_arr_at(shader_pool, mat->shader);
	return (nyas_tex *)mat->ptr + s->count[0].data;
}
