#include "pixels.h"

#include "core/io.h"
#include "core/log.h"
#include "core/mem.h"
#include "render/pixels_internal.h"
#include "utils/array.h"

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

#ifdef NYAS_PIXEL_CHECKS
#define CHECK_HANDLE(TYPE, HANDLE) nypx__check_handle((HANDLE), (TYPE##_pool))
#else
#define CHECK_HANDLE(TYPE, HANDLE) (void)
#endif

#define NYAS_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA (8)

static void
nypx__check_handle(int h, void *arr)
{
	(void)h;
	(void)arr;
	NYAS_ASSERT(h >= 0 && "Bad resource state");
	NYAS_ASSERT(h < (nyas_resource_handle)nyas_arr_count(arr) && "Out of bounds handle");
}

typedef struct nyas_shader_internal shdr_t;
typedef struct nyas_texture_internal tex_t;
typedef struct nyas_texture_image teximg_t;
typedef struct nyas_texture_config texcfg_t;
typedef struct nyas_mesh_internal mesh_t;
typedef struct nyas_framebuffer_internal fb_t;

nyas_mesh SPHERE_MESH;
nyas_mesh CUBE_MESH;
nyas_mesh QUAD_MESH;

mesh_t *mesh_pool;
tex_t *tex_pool;
shdr_t *shader_pool;
fb_t *framebuffer_pool;

texcfg_t *tex_cfgs;
teximg_t *tex_imgs;

typedef struct nyas_cmd_queue {
	nyas_cmd *curr;
	nyas_cmd *curr_last;
	nyas_cmd *next;
	nyas_cmd *next_last;
} cmd_queue;

cmd_queue render_queue;

static nyas_cmd *curr_pool;
static nyas_cmd *curr_pool_tail;
static nyas_cmd *next_pool;
static nyas_cmd *next_pool_tail;

static int8_t *frame_pool[2];
static int8_t *frame_pool_last;
static int frame_switch;

static void
nyas__file_reader(
  void *ctx,
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
		*buf = NULL;
		*size = 0;
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
	nyas_mesh mesh = (nyas_mesh)nyas_arr_count(mesh_pool);
	nyas_arr_push(mesh_pool);
	return mesh;
}

static nyas_tex
nyas__create_tex_handle(void)
{
	nyas_tex tex = (nyas_tex)nyas_arr_count(tex_pool);
	nyas_arr_push(tex_pool);
	return tex;
}

static nyas_framebuffer
nyas__create_fb_handle(void)
{
	nyas_framebuffer fb = (nyas_framebuffer)nyas_arr_count(framebuffer_pool);
	nyas_arr_push(framebuffer_pool);
	return fb;
}

static nyas_shader
nyas__create_shader_handle(void)
{
	nyas_shader shdr = (nyas_shader)nyas_arr_count(shader_pool);
	nyas_arr_push(shader_pool);
	return shdr;
}

void
nyas_px_init(void)
{
	curr_pool = nyas_mem_reserve(NYAS_MB(16));
	next_pool = nyas_mem_reserve(NYAS_MB(16));
	curr_pool_tail = curr_pool;
	next_pool_tail = next_pool;

	mesh_pool = nyas_arr_create(mesh_t, NYAS_MESH_RESERVE);
	tex_pool = nyas_arr_create(tex_t, NYAS_TEX_RESERVE);
	framebuffer_pool = nyas_arr_create(fb_t, NYAS_FB_RESERVE);
	shader_pool = nyas_arr_create(shdr_t, NYAS_SHADER_RESERVE);

	tex_cfgs = nyas_arr_create(texcfg_t, 32);
	tex_imgs = nyas_arr_create(teximg_t, 32);

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

	SPHERE_MESH = nyas_mesh_load_geometry(NYAS_SPHERE);
	CUBE_MESH = nyas_mesh_load_geometry(NYAS_CUBE);
	QUAD_MESH = nyas_mesh_load_geometry(NYAS_QUAD);
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
	NYAS_ASSERT((next_pool_tail - next_pool) < NYAS_RENDER_QUEUE_CAPACITY - 1 &&
	            "Not enough memory in the RenderQueue pool");
	return next_pool_tail++;
}

void *
nyas_alloc_frame(unsigned int size)
{
	NYAS_ASSERT(((frame_pool_last + size) - frame_pool[frame_switch]) < NYAS_FRAME_POOL_SIZE / 2 &&
	            "Not enough memory in the frame pool");
	void *ret = frame_pool_last;
	frame_pool_last += size;
	return ret;
}

struct nyas_texture_desc
nyas_tex_default_desc(nyas_texture_type type)
{
	return (struct nyas_texture_desc){
		.flags = NYAS_TEX_FLAG_DEFAULT,
		.type = type,
		.width = 0,
		.height = 0,
		.levels = 1,
		.desired_channels = 3,
		.fmt = NYAS_TEX_FMT_SRGB,
		.min_filter = NYAS_TEX_FLTR_LINEAR,
		.mag_filter = NYAS_TEX_FLTR_LINEAR,
		.wrap_s = NYAS_TEX_WRAP_REPEAT,
		.wrap_t = NYAS_TEX_WRAP_REPEAT,
		.wrap_r = NYAS_TEX_WRAP_DEFAULT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

struct nyas_texture_desc
nyas_tex_defined_desc(nyas_texture_type type, nyas_texture_format fmt, int w, int h)
{
	return (struct nyas_texture_desc){
		.flags = NYAS_TEX_FLAG_DEFAULT,
		.type = type,
		.width = w,
		.height = h,
		.levels = 1,
		.desired_channels = 0,
		.fmt = fmt,
		.min_filter = NYAS_TEX_FLTR_LINEAR,
		.mag_filter = NYAS_TEX_FLTR_LINEAR,
		.wrap_s = NYAS_TEX_WRAP_REPEAT,
		.wrap_t = NYAS_TEX_WRAP_REPEAT,
		.wrap_r = NYAS_TEX_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

struct nyas_texture_desc
nyas_tex_lod_desc(nyas_texture_type type, int levels, bool gen_mipmaps)
{
	return (struct nyas_texture_desc){
		.flags = NYAS_TEX_FLAG_GENERATE_MIPMAPS * gen_mipmaps,
		.type = type,
		.width = 0,
		.height = 0,
		.levels = levels,
		.desired_channels = 0,
		.fmt = NYAS_TEX_FMT_RGB8,
		.min_filter = NYAS_TEX_FLTR_LINEAR,
		.mag_filter = NYAS_TEX_FLTR_LINEAR,
		.wrap_s = NYAS_TEX_WRAP_REPEAT,
		.wrap_t = NYAS_TEX_WRAP_REPEAT,
		.wrap_r = NYAS_TEX_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

const char *
nyas__face_img_path(const char *path, const char *suffixes, int face)
{
	static char buffer[1024];
	int count = snprintf(buffer, 1024, path, suffixes[face]);
	(void)count;
	NYAS_ASSERT(count < 1024 && "Xpand dat mf");
	return buffer;
}

static texcfg_t
texconfig_from_desc(nyas_tex t, struct nyas_texture_desc *desc)
{
	return (texcfg_t){
		.idx = t,
		.min = desc->min_filter,
		.mag = desc->mag_filter,
		.ws = desc->wrap_s,
		.wt = desc->wrap_t,
		.wr = desc->wrap_r,
		.border_color = { desc->border_color[0], desc->border_color[1], desc->border_color[2],
		                  desc->border_color[3] }
	};
}

static texcfg_t
texconfig(nyas_tex t, nyas_texture_filter f, nyas_texture_wrap w)
{
	return (texcfg_t){
		.idx = t,
		.min = f,
		.mag = f,
		.ws = w,
		.wt = w,
		.wr = w,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
}

nyas_tex
nyas_tex_load(struct nyas_texture_desc *desc, const char *path)
{
	NYAS_ASSERT(*path != '\0' && "For empty textures use nyas_tex_empty");

	nyas_tex tex = nyas__create_tex_handle();
	tex_t *t = &tex_pool[tex];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_GENERATE_MIPMAPS *
	  ((desc->flags & NYAS_TEX_FLAG_GENERATE_MIPMAPS) != 0);
	t->res.flags |= NYAS_IRF_DIRTY;
	t->type = desc->type;

	teximg_t *img = nyas_arr_push(tex_imgs);
	img->idx = tex;
	img->lod = 0;

	stbi_set_flip_vertically_on_load(desc->flags & NYAS_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD);
	int channels = 0;
	if (desc->type == NYAS_TEX_2D) {
		img->pix[0] = stbi_load(path, &t->w, &t->h, &channels, desc->desired_channels);
		NYAS_ASSERT(img->pix[0] && "The image couldn't be loaded");
	} else if (desc->type == NYAS_TEX_CUBEMAP) {
		const char *suffixes = "RLUDFB";
		for (int i = 0; i < 6; ++i) {
			const char *p = nyas__face_img_path(path, suffixes, i);
			img->pix[i] = stbi_load(p, &t->w, &t->h, &channels, desc->desired_channels);
			NYAS_ASSERT(img->pix[i] && "The image couldn't be loaded");
		}
	}

	switch (desc->desired_channels ? desc->desired_channels : channels) {
	case 4: desc->fmt = NYAS_TEX_FMT_RGBA8; break;
	case 3: if (desc->fmt != NYAS_TEX_FMT_RGB8) desc->fmt = NYAS_TEX_FMT_SRGB; break;
	case 2: desc->fmt = NYAS_TEX_FMT_RG8; break;
	case 1: desc->fmt = NYAS_TEX_FMT_R8; break;
	default: NYAS_ASSERT(!"Invalid channel count after loading img from file.");
	}

	t->fmt = desc->fmt;
	texcfg_t *new = nyas_arr_push(tex_cfgs);
	*new = texconfig_from_desc(tex, desc);
	return tex;
}

nyas_tex
nyas_tex_loadf(struct nyas_texture_desc *desc, const char *path, bool flip)
{
	NYAS_ASSERT(*path != '\0' && "For empty textures use nyas_tex_empty");

	nyas_tex tex = nyas__create_tex_handle();
	tex_t *t = &tex_pool[tex];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_GENERATE_MIPMAPS *
	               ((desc->flags & NYAS_TEX_FLAG_GENERATE_MIPMAPS) != 0);
	t->res.flags |= NYAS_IRF_DIRTY;
	t->type = desc->type;

	teximg_t *img = nyas_arr_push(tex_imgs);
	img->idx = tex;
	img->lod = 0;

	stbi_set_flip_vertically_on_load(flip);
	int channels = 0;
	if (desc->type == NYAS_TEX_2D) {
		img->pix[0] = stbi_loadf(path, &t->w, &t->h, &channels, desc->desired_channels);
		NYAS_ASSERT(img->pix[0] && "The image couldn't be loaded");
	}

	switch (desc->desired_channels ? desc->desired_channels : channels) {
	case 4: desc->fmt = NYAS_TEX_FMT_RGBA16F; break;
	case 3: desc->fmt = NYAS_TEX_FMT_RGB32F; break;
	case 2: desc->fmt = NYAS_TEX_FMT_RG16F; break;
	case 1: desc->fmt = NYAS_TEX_FMT_R16F; break;
	default: NYAS_ASSERT(!"Invalid channel count after loading img from file.");
	}

	t->fmt = desc->fmt;
	texcfg_t *new = nyas_arr_push(tex_cfgs);
	*new = texconfig_from_desc(tex, desc);
	return tex;
}

nyas_tex
nyas_tex_empty(struct nyas_texture_desc *desc)
{
	NYAS_ASSERT(desc->width > 0 && desc->height > 0 && "Incorrect dimensions");
	nyas_tex tex = nyas__create_tex_handle();
	tex_t *t = &tex_pool[tex];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_GENERATE_MIPMAPS *
	               ((desc->flags & NYAS_TEX_FLAG_GENERATE_MIPMAPS) != 0);
	t->res.flags |= NYAS_IRF_DIRTY;
	t->w = desc->width;
	t->h = desc->height;
	t->type = desc->type;
	t->fmt = desc->fmt;

	texcfg_t *new = nyas_arr_push(tex_cfgs);
	*new = texconfig_from_desc(tex, desc);
	return tex;
}

struct nyas_vec2i
nyas_tex_size(nyas_tex tex)
{
	return (struct nyas_vec2i){ tex_pool[tex].w, tex_pool[tex].h };
}

void
nyas_load_env(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref)
{
	FILE *f = fopen(path, "r");
	char hdr[9];
	fread(hdr, 8, 1, f);
	hdr[8] = '\0';
	if (strncmp("NYAS_ENV", hdr, 9) != 0) {
		NYAS_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
		return;
	}

	*sky = nyas__create_tex_handle();
	tex_t *t = &tex_pool[*sky];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->type = NYAS_TEX_CUBEMAP;
	t->fmt = NYAS_TEX_FMT_RGB16F;
	t->w = 1024;
	t->h = 1024;

	texcfg_t *cfg = nyas_arr_push(tex_cfgs);
	*cfg = texconfig(*sky, NYAS_TEX_FLTR_LINEAR, NYAS_TEX_WRAP_CLAMP);

	teximg_t *img = nyas_arr_push(tex_imgs);
	img->idx = *sky;
	img->lod = 0;
	size_t size = 1024 * 1024 * 3 * 2; // size * nchannels * sizeof(channel)
	for (int i = 0; i < 6; ++i) {
		img->pix[i] = nyas_alloc(size);
		fread(img->pix[i], size, 1, f);
		NYAS_ASSERT(img->pix[i] && "The image couldn't be loaded");
	}

	*irr = nyas__create_tex_handle();
	t = &tex_pool[*irr];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->type = NYAS_TEX_CUBEMAP;
	t->fmt = NYAS_TEX_FMT_RGB16F;
	t->w = 1024;
	t->h = 1024;

	cfg = nyas_arr_push(tex_cfgs);
	*cfg = texconfig(*irr, NYAS_TEX_FLTR_LINEAR, NYAS_TEX_WRAP_CLAMP);

	img = nyas_arr_push(tex_imgs);
	img->idx = *irr;
	img->lod = 0;
	for (int i = 0; i < 6; ++i) {
		img->pix[i] = nyas_alloc(size);
		fread(img->pix[i], size, 1, f);
		NYAS_ASSERT(img->pix[i] && "The image couldn't be loaded");
	}

	*pref = nyas__create_tex_handle();
	t = &tex_pool[*pref];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->type = NYAS_TEX_CUBEMAP;
	t->fmt = NYAS_TEX_FMT_RGB16F;
	t->w = 256;
	t->h = 256;

	cfg = nyas_arr_push(tex_cfgs);
	*cfg = texconfig(*pref, NYAS_TEX_FLTR_LINEAR, NYAS_TEX_WRAP_CLAMP);
	cfg->min = NYAS_TEX_FLTR_LINEAR_MIPMAP_LINEAR;

	size = 256 * 256 * 3 * 2;
	for (int lod = 0; lod < 9; ++lod) {
		img = nyas_arr_push(tex_imgs);
		img->idx = *pref;
		img->lod = lod;
		for (int i = 0; i < 6; ++i) {
			img->pix[i] = nyas_alloc(size);
			fread(img->pix[i], size, 1, f);
			NYAS_ASSERT(img->pix[i] && "The image couldn't be loaded");
		}
		size /= 4;
	}

	*lut = nyas__create_tex_handle();
	t = &tex_pool[*lut];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->type = NYAS_TEX_2D;
	t->fmt = NYAS_TEX_FMT_RG16F;
	t->w = 512;
	t->h = 512;
	size = 512 * 512 * 2 * 2;
	cfg = nyas_arr_push(tex_cfgs);
	*cfg = texconfig(*lut, NYAS_TEX_FLTR_LINEAR, NYAS_TEX_WRAP_CLAMP);
	img = nyas_arr_push(tex_imgs);
	img->idx = *lut;
	img->lod = 0;
	img->pix[0] = nyas_alloc(size);
	fread(img->pix[0], size, 1, f);
	NYAS_ASSERT(img->pix[0] && "The image couldn't be loaded");
	fclose(f);
}

nyas_shader
nyas_shader_create(const nyas_shader_desc *desc)
{
	nyas_shader ret = nyas__create_shader_handle();
	shader_pool[ret].name = desc->name;
	shader_pool[ret].res.id = 0;
	shader_pool[ret].res.flags = 0;
	shader_pool[ret].count[0].data = desc->data_count;
	shader_pool[ret].count[0].tex = desc->tex_count;
	shader_pool[ret].count[0].cubemap = desc->cubemap_count;
	shader_pool[ret].count[1].data = desc->common_data_count;
	shader_pool[ret].count[1].tex = desc->common_tex_count;
	shader_pool[ret].count[1].cubemap = desc->common_cubemap_count;
	shader_pool[ret].common = nyas_alloc(
	  (desc->common_data_count + desc->common_tex_count + desc->common_cubemap_count) *
	  sizeof(float));
	return ret;
}

void *
nyas_shader_data(nyas_shader shader)
{
	return shader_pool[shader].common;
}

nyas_tex *
nyas_shader_tex(nyas_shader shader)
{
	shdr_t *shdr = &shader_pool[shader];
	return (nyas_tex *)shdr->common + shdr->count[1].data;
}

nyas_tex *
nyas_shader_cubemap(nyas_shader shader)
{
	shdr_t *shdr = &shader_pool[shader];
	return nyas_shader_tex(shader) + shdr->count[1].tex;
}

void
nyas_shader_reload(nyas_shader shader)
{
	shader_pool[shader].res.flags |= NYAS_IRF_DIRTY;
}

static void
nyas__mesh_set_cube(mesh_t *mesh)
{
	static const float VERTICES[] = {
		// positions          // normals           // uv
		-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f, -0.5f,
		0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
		1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,

		-0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
		0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

		-0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
		-1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
		0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
		1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
		0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f,
		0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
		1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,

		-0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  -0.5f,
		0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
	};

	static const nyas_idx INDICES[] = {
		0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
		13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
	};

	if (mesh->vtx) {
		nyas_free(mesh->vtx);
	}

	if (mesh->idx) {
		nyas_free(mesh->idx);
	}

	mesh->attrib = (1 << VA_POSITION) | (1 << VA_NORMAL) | (1 << VA_UV);
	mesh->vtx = nyas_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = nyas_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

static void
nyas__mesh_set_sphere(mesh_t *mesh, int x_segments, int y_segments)
{
	NYAS_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

	const float x_step = 1.0f / (float)(y_segments - 1);
	const float y_step = 1.0f / (float)(x_segments - 1);

	if (mesh->vtx) {
		nyas_free(mesh->vtx);
	}

	if (mesh->idx) {
		nyas_free(mesh->idx);
	}

	mesh->attrib = (1 << VA_POSITION) | (1 << VA_NORMAL) | (1 << VA_UV);
	mesh->vtx_size = y_segments * x_segments * 8 * sizeof(float);
	mesh->elem_count = y_segments * x_segments * 6;
	mesh->vtx = nyas_alloc(mesh->vtx_size);
	mesh->idx = nyas_alloc(mesh->elem_count * sizeof(nyas_idx));

	float *v = mesh->vtx;
	for (int y = 0; y < x_segments; ++y) {
		float py = sinf((float)-M_PI_2 + (float)M_PI * (float)y * x_step);
		for (int x = 0; x < y_segments; ++x) {
			float px = cosf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);
			float pz = sinf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);

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

	nyas_idx *i = mesh->idx;
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
}

static void
nyas__mesh_set_quad(mesh_t *mesh)
{
	static const float VERTICES[] = {
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f,
		0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  -1.0f,
		1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	};

	static const nyas_idx INDICES[] = { 0, 1, 2, 0, 2, 3 };

	if (mesh->vtx) {
		nyas_free(mesh->vtx);
	}

	if (mesh->idx) {
		nyas_free(mesh->idx);
	}

	mesh->attrib = (1 << VA_POSITION) | (1 << VA_NORMAL) | (1 << VA_UV);
	mesh->vtx = nyas_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = nyas_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

static nyas_idx
check_vertex(float *v, float *end, float *newvtx)
{
	nyas_idx i = 0;
	for (; v < end; ++i, v += 14) {
		if ((v[0] == newvtx[0]) && (v[1] == newvtx[1]) && (v[2] == newvtx[2]) &&
		    (v[3] == newvtx[3]) && (v[4] == newvtx[4]) && (v[5] == newvtx[5]) &&
		    (v[12] == newvtx[12]) && (v[13] == newvtx[13])) {
			return i;
		}
	}
	return i;
}

static void
nyas__mesh_set_obj(mesh_t *mesh, const char *path)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t *shapes = NULL;
	size_t shape_count;
	tinyobj_material_t *mats = NULL;
	size_t mats_count;

	int result = tinyobj_parse_obj(
	  &attrib, &shapes, &shape_count, &mats, &mats_count, path, nyas__file_reader, NULL,
	  TINYOBJ_FLAG_TRIANGULATE);

	NYAS_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
	if (result != TINYOBJ_SUCCESS) {
		NYAS_LOG_ERR("Error loading obj. Err: %d", result);
	}

	size_t vertex_count = attrib.num_face_num_verts * 3;

	if (mesh->vtx) {
		nyas_free(mesh->vtx);
	}

	if (mesh->idx) {
		nyas_free(mesh->idx);
	}

	mesh->attrib = (1 << VA_POSITION) | (1 << VA_NORMAL) | (1 << VA_TANGENT) |
	  (1 << VA_BITANGENT) | (1 << VA_UV);
	mesh->vtx_size = vertex_count * 14 * sizeof(float);
	mesh->elem_count = vertex_count;
	mesh->vtx = nyas_alloc(mesh->vtx_size);
	mesh->idx = nyas_alloc(mesh->elem_count * sizeof(nyas_idx));

	float *vit = mesh->vtx;

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
			float r = 1.0f / (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

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
			nyas_idx nxt_idx = check_vertex(mesh->vtx, vit, v1);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v1[j];
				}
			}

			nxt_idx = check_vertex(mesh->vtx, vit, v2);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v2[j];
				}
			}

			nxt_idx = check_vertex(mesh->vtx, vit, v3);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v3[j];
				}
			}
		}
	}

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, shape_count);
	tinyobj_materials_free(mats, mats_count);
}

static void
nyas__mesh_set_msh(mesh_t *mesh, const char *path)
{
	char *data;
	size_t sz;
	nyas__file_reader(NULL, path, 0, NULL, &data, &sz);
	if (!data || !sz) {
		NYAS_LOG_ERR("Problem reading file %s", path);
		return;
	}

	if (mesh->vtx) {
		nyas_free(mesh->vtx);
	}

	if (mesh->idx) {
		nyas_free(mesh->idx);
	}

	mesh->attrib = (1 << VA_POSITION) | (1 << VA_NORMAL) | (1 << VA_TANGENT) |
	  (1 << VA_BITANGENT) | (1 << VA_UV);
	mesh->vtx_size = *(size_t *)data; // *(((size_t *)data)++)
	data += sizeof(size_t);
	mesh->vtx = nyas_alloc(mesh->vtx_size);
	memcpy(mesh->vtx, data, mesh->vtx_size);
	data += mesh->vtx_size;

	mesh->elem_count = (*(size_t *)data) / sizeof(nyas_idx); // *(((size_t *)data)++)
	data += sizeof(size_t);
	mesh->idx = nyas_alloc(mesh->elem_count * sizeof(nyas_idx));
	memcpy(mesh->idx, data, mesh->elem_count * sizeof(nyas_idx));

	nyas_free(data - mesh->vtx_size - (2 * sizeof(size_t)));
}

void
nyas_mesh_reload_file(nyas_mesh mesh, const char *path)
{
	mesh_t *m = &mesh_pool[mesh];
	size_t len = strlen(path);
	const char *extension = path + len;
	while (*--extension != '.') {}
	extension++;
	if (!strcmp(extension, "obj")) {
		nyas__mesh_set_obj(m, path);
	} else if (!strcmp(extension, "msh")) {
		nyas__mesh_set_msh(m, path);
	} else {
		NYAS_LOG_ERR("Extension (%s) of file %s not recognised.", extension, path);
	}

	m->res.flags |= NYAS_IRF_DIRTY;
}

void
nyas_mesh_reload_geometry(nyas_mesh mesh, enum nyas_geometry geo)
{
	mesh_t *m = &mesh_pool[mesh];

	switch (geo) {
	case NYAS_QUAD: nyas__mesh_set_quad(m); break;

	case NYAS_CUBE: nyas__mesh_set_cube(m); break;

	case NYAS_SPHERE: nyas__mesh_set_sphere(m, 32, 32); break;
	}
	m->res.flags |= NYAS_IRF_DIRTY;
}

static nyas_mesh
nyas__mesh_create(void)
{
	nyas_mesh mesh_handle = nyas__create_mesh_handle();
	mesh_pool[mesh_handle].res.id = 0;
	mesh_pool[mesh_handle].res.flags = NYAS_IRF_DIRTY;
	mesh_pool[mesh_handle].attrib = 0;
	mesh_pool[mesh_handle].vtx = NULL;
	mesh_pool[mesh_handle].idx = NULL;
	mesh_pool[mesh_handle].vtx_size = 0;
	mesh_pool[mesh_handle].elem_count = 0;
	mesh_pool[mesh_handle].res_vb.id = 0;
	mesh_pool[mesh_handle].res_vb.flags = NYAS_IRF_DIRTY;
	mesh_pool[mesh_handle].res_ib.id = 0;
	mesh_pool[mesh_handle].res_ib.flags = NYAS_IRF_DIRTY;

	return mesh_handle;
}

nyas_mesh
nyas_mesh_load_file(const char *path)
{
	nyas_mesh mesh_handle = nyas__mesh_create();
	nyas_mesh_reload_file(mesh_handle, path);
	return mesh_handle;
}

nyas_mesh
nyas_mesh_load_geometry(enum nyas_geometry geo)
{
	nyas_mesh mesh_handle = nyas__mesh_create();
	nyas_mesh_reload_geometry(mesh_handle, geo);
	return mesh_handle;
}

nyas_framebuffer
nyas_fb_create(void)
{
	nyas_framebuffer fb = nyas__create_fb_handle();
	framebuffer_pool[fb].res.id = 0;
	framebuffer_pool[fb].res.flags = NYAS_IRF_DIRTY;
	return fb;
}

nyas_mat
nyas_mat_pers(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = &shader_pool[shader];
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = nyas_alloc(elements * sizeof(float));
	return ret;
}

nyas_mat
nyas_mat_tmp(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = &shader_pool[shader];
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = nyas_alloc_frame(elements * sizeof(float));
	return ret;
}

nyas_mat
nyas_mat_copy(nyas_mat mat)
{
	nyas_mat ret = { .shader = mat.shader };
	shdr_t *s = &shader_pool[mat.shader];
	size_t size = (s->count[0].data + s->count[0].tex + s->count[0].cubemap) * 4;
	ret.ptr = nyas_alloc_frame(size);
	memcpy(ret.ptr, mat.ptr, size);
	return ret;
}

nyas_mat
nyas_mat_copy_shader(nyas_shader shader)
{
	nyas_mat ret = { .ptr = NULL, .shader = shader };
	shdr_t *s = &shader_pool[shader];
	int elements = s->count[1].data + s->count[1].tex + s->count[1].cubemap;
	ret.ptr = nyas_alloc_frame(elements * sizeof(float));
	memcpy(ret.ptr, s->common, elements * sizeof(float));
	return ret;
}

nyas_tex *
nyas_mat_tex(nyas_mat mat)
{
	return (nyas_tex *)mat.ptr + shader_pool[mat.shader].count[0].data;
}

nyas_tex *
nyas_mat_cubemap(nyas_mat mat)
{
	shdr_t *s = &shader_pool[mat.shader];
	return (nyas_tex *)mat.ptr + (s->count[0].data + s->count[0].tex);
}

static void
nyas__sync_gpu_mesh(nyas_mesh mesh, nyas_shader shader)
{
	CHECK_HANDLE(mesh, mesh);
	CHECK_HANDLE(shader, shader);
	mesh_t *m = &mesh_pool[mesh];

	if (!(m->res.flags & NYAS_IRF_CREATED)) {
		nypx_mesh_create(&m->res.id, &m->res_vb.id, &m->res_ib.id);
		m->res.flags |= NYAS_IRF_DIRTY;
	}

	if (m->res.flags & NYAS_IRF_DIRTY) {
		nypx_mesh_set(m->res.id, m->res_vb.id, m->res_ib.id, shader_pool[shader].res.id, m->attrib,
		              m->vtx, m->vtx_size, m->idx, m->elem_count);
	}
	m->res.flags = NYAS_IRF_CREATED;
}

static tex_t *
nyas__sync_gpu_tex(nyas_tex tex)
{
	CHECK_HANDLE(tex, tex);
	tex_t *t = &tex_pool[tex];
	if (!(t->res.flags & NYAS_IRF_CREATED)) {
		for (int i = 0; i < nyas_arr_count(tex_cfgs); ++i) {
			if (tex_cfgs[i].idx == tex) {
				nypx_tex_create(t, tex_cfgs + i);
				t->res.flags |= NYAS_IRF_CREATED;
				t->res.flags |= NYAS_IRF_DIRTY;
				//nyas_arr_rm(tex_cfgs, i);
				break;
			}
		}
	}

	if (t->res.flags & NYAS_IRF_DIRTY) {
		int count = 0;
		for (int i = 0; i < nyas_arr_count(tex_imgs); ++i) {
			if (tex_imgs[i].idx == tex) {
				nypx_tex_set(t, tex_imgs + i);
				//nyas_arr_rm(tex_imgs, i);
				//--i;
				++count;
			}
		}
		if (!count) {
			struct nyas_texture_image img = {.idx = tex, .lod = 0, .pix = {NULL, NULL, NULL, NULL, NULL, NULL}};
			nypx_tex_set(t, &img);
		}
		t->res.flags &= ~NYAS_IRF_DIRTY;
	}
	return t;
}

static void
nyas__set_shader_data(shdr_t *s, void *srcdata, int common)
{
	NYAS_ASSERT((common == 0 || common == 1) && "Invalid common value.");

	int dc = s->count[common].data;
	int tc = s->count[common].tex;
	int cc = s->count[common].cubemap;
	int dl = s->loc[common].data;
	int tl = s->loc[common].tex;
	int cl = s->loc[common].cubemap;
	int tex_unit = NYAS_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA * common;

	NYAS_ASSERT((dc >= 0) && (tc >= 0) && (cc >= 0));
	NYAS_ASSERT(tex_unit >= 0 && tex_unit < 128);

	if (!(dc + tc + cc)) {
		return;
	}

	// change texture handle for texture internal id
	nyas_tex *data_tex = (nyas_tex *)srcdata + dc;
	for (int i = 0; i < tc + cc; ++i) {
		tex_t *itx = nyas__sync_gpu_tex(data_tex[i]);
		data_tex[i] = (int)itx->res.id;
	}

	// set opengl uniforms
	if (dc) {
		nypx_shader_set_data(dl, srcdata, dc / 4);
	}

	if (tc) {
		nypx_shader_set_tex(tl, data_tex, tc, tex_unit);
	}

	if (cc) {
		nypx_shader_set_cube(cl, data_tex + tc, cc, tex_unit + tc);
	}
}

void
nyas_setshader_fn(nyas_cmdata *data)
{
	static const char *uniforms[] = {
		"u_data", "u_tex", "u_cube", "u_common_data", "u_common_tex", "u_common_cube"
	};

	CHECK_HANDLE(shader, data->mat.shader);
	shdr_t *s = &shader_pool[data->mat.shader];
	if (!(s->res.flags & NYAS_IRF_CREATED)) {
		nypx_shader_create(&s->res.id);
		s->res.flags |= NYAS_IRF_DIRTY;
	}

	if (s->res.flags & NYAS_IRF_DIRTY) {
		NYAS_ASSERT(s->name && *s->name && "Shader name needed.");
		nypx_shader_compile(s->res.id, s->name);
		nypx_shader_loc(s->res.id, &s->loc[0].data, &uniforms[0], 6);
		s->res.flags = NYAS_IRF_CREATED; // reset dirty flag
	}

	nypx_shader_use(s->res.id);
	nyas__set_shader_data(s, data->mat.ptr, true);
}

void
nyas_clear_fn(nyas_cmdata *data)
{
	nypx_clear_color(
	  data->clear.color[0], data->clear.color[1], data->clear.color[2], data->clear.color[3]);
	nypx_clear(data->clear.color_buffer, data->clear.depth_buffer, data->clear.stencil_buffer);
}

void
nyas_draw_fn(nyas_cmdata *data)
{
	nyas_mesh mesh = data->draw.mesh;
	mesh_t *imsh = &mesh_pool[mesh];
	CHECK_HANDLE(mesh, mesh);
	NYAS_ASSERT(imsh->elem_count && "Attempt to draw an uninitialized mesh");

	if (imsh->res.flags & NYAS_IRF_DIRTY) {
		nyas__sync_gpu_mesh(mesh, data->draw.material.shader);
	}

	shdr_t *s = &shader_pool[data->draw.material.shader];
	nyas__set_shader_data(s, data->draw.material.ptr, false);
	nypx_mesh_use(imsh, s);
	nypx_draw(imsh->elem_count, sizeof(nyas_idx) == 4);
}

void
nyas_rops_fn(nyas_cmdata *data)
{
	int enable = data->rend_opts.enable_flags;
	int disable = data->rend_opts.disable_flags;
	for (int i = 1; i < NYAS_REND_OPTS_COUNT; ++i) {
		// Check disable first for bad configs (both on)
		switch (disable & (1 << i)) {
		case NYAS_BLEND: nypx_blend_disable(); continue;
		case NYAS_CULL_FACE: nypx_cull_disable(); continue;
		case NYAS_DEPTH_TEST: nypx_depth_disable_test(); continue;
		case NYAS_DEPTH_WRITE: nypx_depth_disable_mask(); continue;
		}

		// Check if the option is in the enable group
		switch (enable & (1 << i)) {
		case NYAS_BLEND: nypx_blend_enable(); continue;
		case NYAS_CULL_FACE: nypx_cull_enable(); continue;
		case NYAS_DEPTH_TEST: nypx_depth_enable_test(); continue;
		case NYAS_DEPTH_WRITE: nypx_depth_enable_mask(); continue;
		}
	}

	/* Ignore unless both have a value assigned. */
	if (data->rend_opts.blend_src && data->rend_opts.blend_dst) {
		nypx_blend_set(data->rend_opts.blend_src, data->rend_opts.blend_dst);
	}

	if (data->rend_opts.cull_face) {
		nypx_cull_set(data->rend_opts.cull_face);
	}

	if (data->rend_opts.depth_func) {
		nypx_depth_set(data->rend_opts.depth_func);
	}
}

void
nyas_setfb_fn(nyas_cmdata *data)
{
	NYAS_ASSERT(data && "Null param");
	const nyas_set_fb_cmdata *d = &data->set_fb;
	if (d->fb == NYAS_DEFAULT) {
		nypx_fb_use(0);
	} else {
		// GPU Sync
		CHECK_HANDLE(framebuffer, d->fb);
		fb_t *ifb = &framebuffer_pool[d->fb];
		if (!(ifb->res.flags & NYAS_IRF_CREATED)) {
			nypx_fb_create(&ifb->res.id);
		}
		ifb->res.flags = NYAS_IRF_CREATED;

		nypx_fb_use(ifb->res.id);
		if (d->attach.type != NYAS_IGNORE) {
			tex_t *t = nyas__sync_gpu_tex(d->attach.tex);
			nypx_fb_set(
			  ifb->res.id, t->res.id, d->attach.type, d->attach.mip_level, d->attach.face);
		}
	}
	nypx_viewport(0, 0, d->vp_x, d->vp_y);
}
