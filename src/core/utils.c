#include "utils.h"

#include "core/io.h"
#include "core/mem.h"
#include "render/pixels_internal.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

nyas_mesh NYAS_UTILS_SPHERE;
nyas_mesh NYAS_UTILS_CUBE;
nyas_mesh NYAS_UTILS_QUAD;

typedef struct nyas_job job;
NYAS_DECL_ARR(job);

struct nyut_asset_loader {
	struct nyarr_job *seq;
	struct nyarr_job *async;
};

static void
load_tex(void *arg)
{
	struct nyut_tex_ldargs *a = arg;
	nyas_tex_load(a->tex, &a->desc, a->path);
}

static void
load_mesh(void *arg)
{
	struct nyut_mesh_ldargs *a = arg;
	*a->mesh = nyas_mesh_load_file(a->path); // TODO: separate create and load like textures in
	                                         // order to avoid concurrent writes in mesh_pool
}

static void
load_shader(void *arg)
{
	struct nyut_shad_ldargs *a = arg;
	*a->shader = nyas_shader_create(&a->desc);
	// TODO: Shaders compilation and program linking.
}

static void
load_env(void *args)
{
	struct nyut_env_ldargs *ea = args;
	nyut_env_load(ea->path, ea->lut, ea->sky, ea->irr, ea->pref);
}

struct nyut_asset_loader *
nyut_assets_create(void)
{
	struct nyut_asset_loader *l = malloc(sizeof(struct nyut_asset_loader));
	l->async = NULL;
	l->seq = NULL;
	return l;
}

void
nyut_assets_add_mesh(struct nyut_asset_loader *l, struct nyut_mesh_ldargs *args)
{
	job *job = nyarr_job_push(&l->async);
	job->job = load_mesh;
	job->args = args;
}

void
nyut_assets_add_tex(struct nyut_asset_loader *l, struct nyut_tex_ldargs *args)
{
	job *job = nyarr_job_push(&l->async);
	job->job = load_tex;
	job->args = args;
}

void
nyut_assets_add_shader(struct nyut_asset_loader *l, struct nyut_shad_ldargs *args)
{
	job *job = nyarr_job_push(&l->seq);
	job->job = load_shader;
	job->args = args;
}

void
nyut_assets_add_env(struct nyut_asset_loader *l, struct nyut_env_ldargs *args)
{
	job *job = nyarr_job_push(&l->async);
	job->job = load_env;
	job->args = args;
}

void
nyut_assets_add_job(struct nyut_asset_loader *l, struct nyas_job j, bool async)
{
	if (async) {
		nyarr_job_push_value(&l->async, j);
	} else {
		nyarr_job_push_value(&l->seq, j);
	}
}

void
nyut_assets_load(struct nyut_asset_loader *l, int threads)
{
	nysched *load_sched = NULL;
	if (l->async) {
		load_sched = nyas_sched_create(threads);
		for (int i = 0; i < l->async->count; ++i) {
			nyas_sched_do(load_sched, l->async->at[i]);
		}
	}

	if (l->seq) {
		for (int i = 0; i < l->seq->count; ++i) {
			(*l->seq->at[i].job)(l->seq->at[i].args);
		}
	}

	if (l->async) {
		nyas_sched_wait(load_sched); // TODO(Check): sched_destroy waits?
		nyas_sched_destroy(load_sched);
	}
	free(l->seq);
	free(l->async);
	free(l);
}

static void
nyas__mesh_set_cube(mesh *mesh)
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

	nyas_free(mesh->vtx);
	nyas_free(mesh->idx);

	mesh->attrib = (1 << NYAS_VA_POS) | (1 << NYAS_VA_NORMAL) | (1 << NYAS_VA_UV);
	mesh->vtx = nyas_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = nyas_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

static void
nyas__mesh_set_sphere(mesh *mesh, int x_segments, int y_segments)
{
	NYAS_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

	const float x_step = 1.0f / (float)(y_segments - 1);
	const float y_step = 1.0f / (float)(x_segments - 1);

	nyas_free(mesh->vtx);
	nyas_free(mesh->idx);

	mesh->attrib = (1 << NYAS_VA_POS) | (1 << NYAS_VA_NORMAL) | (1 << NYAS_VA_UV);
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
nyas__mesh_set_quad(mesh *mesh)
{
	static const float VERTICES[] = {
	  -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f,
	  0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	};

	static const nyas_idx INDICES[] = { 0, 1, 2, 0, 2, 3 };

	nyas_free(mesh->vtx);
	nyas_free(mesh->idx);

	mesh->attrib = (1 << NYAS_VA_POS) | (1 << NYAS_VA_NORMAL) | (1 << NYAS_VA_UV);
	mesh->vtx = nyas_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = nyas_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

void nyut_mesh_init_geometry(void)
{
	NYAS_UTILS_SPHERE = nyas_mesh_create();
	NYAS_UTILS_CUBE = nyas_mesh_create();
	NYAS_UTILS_QUAD = nyas_mesh_create();
	nyut_mesh_set_geometry(NYAS_UTILS_SPHERE, NYAS_SPHERE);
	nyut_mesh_set_geometry(NYAS_UTILS_CUBE, NYAS_CUBE);
	nyut_mesh_set_geometry(NYAS_UTILS_QUAD, NYAS_QUAD);
}

void
nyut_mesh_set_geometry(nyas_mesh msh, enum nyut_geometry geo)
{
	mesh *m = &mesh_pool.buf->at[msh];

	switch (geo) {
	case NYAS_QUAD: nyas__mesh_set_quad(m); break;
	case NYAS_CUBE: nyas__mesh_set_cube(m); break;
	case NYAS_SPHERE: nyas__mesh_set_sphere(m, 32, 32); break;
	}
	m->res.flags |= NYAS_IRF_DIRTY;
}

void
nyut_env_load(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref)
{
	FILE *f = fopen(path, "r");
	char hdr[9];
	fread(hdr, 8, 1, f);
	hdr[8] = '\0';
	if (strncmp("NYAS_ENV", hdr, 9) != 0) {
		NYAS_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
		return;
	}

	*sky = nyas_tex_create();
	tex *t = &tex_pool.buf->at[*sky];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->data.type = NYAS_TEX_CUBEMAP;
	t->data.fmt = NYAS_TEX_FMT_RGB16F;
	t->data.width = 1024;
	t->data.height = 1024;
	t->data.mag_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.min_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.wrap_s = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_t = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_r = NYAS_TEX_WRAP_CLAMP;
	t->img = NULL;

	size_t size = 1024 * 1024 * 3 * 2; // size * nchannels * sizeof(channel)
	for (int i = 0; i < 6; ++i) {
		struct nyas_texture_image *img = nyarr_nyteximg_push(&t->img);
		img->lod = 0;
		img->face = i;
		img->pix = nyas_alloc(size);
		fread(img->pix, size, 1, f);
		NYAS_ASSERT(img->pix && "The image couldn't be loaded");
	}

	*irr = nyas_tex_create();
	t = &tex_pool.buf->at[*irr];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->data.type = NYAS_TEX_CUBEMAP;
	t->data.fmt = NYAS_TEX_FMT_RGB16F;
	t->data.width = 1024;
	t->data.height = 1024;
	t->data.mag_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.min_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.wrap_s = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_t = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_r = NYAS_TEX_WRAP_CLAMP;
	t->img = NULL;

	for (int i = 0; i < 6; ++i) {
		struct nyas_texture_image *img = nyarr_nyteximg_push(&t->img);
		img->lod = 0;
		img->face = i;
		img->pix = nyas_alloc(size);
		fread(img->pix, size, 1, f);
		NYAS_ASSERT(img->pix && "The image couldn't be loaded");
	}

	*pref = nyas_tex_create();
	t = &tex_pool.buf->at[*pref];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->data.type = NYAS_TEX_CUBEMAP;
	t->data.fmt = NYAS_TEX_FMT_RGB16F;
	t->data.width = 256;
	t->data.height = 256;
	t->data.mag_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.min_filter = NYAS_TEX_FLTR_LINEAR_MIPMAP_LINEAR;
	t->data.wrap_s = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_t = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_r = NYAS_TEX_WRAP_CLAMP;
	t->img = NULL;

	size = 256 * 256 * 3 * 2;
	for (int lod = 0; lod < 9; ++lod) {
		for (int face = 0; face < 6; ++face) {
			struct nyas_texture_image *img = nyarr_nyteximg_push(&t->img);
			img->lod = lod;
			img->face = face;
			img->pix = nyas_alloc(size);
			fread(img->pix, size, 1, f);
			NYAS_ASSERT(img->pix && "The image couldn't be loaded");
		}
		size /= 4;
	}

	*lut = nyas_tex_create();
	t = &tex_pool.buf->at[*lut];
	t->res.id = 0;
	t->res.flags = NYAS_IRF_DIRTY;
	t->data.type = NYAS_TEX_2D;
	t->data.fmt = NYAS_TEX_FMT_RG16F;
	t->data.width = 512;
	t->data.height = 512;
	t->data.mag_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.min_filter = NYAS_TEX_FLTR_LINEAR;
	t->data.wrap_s = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_t = NYAS_TEX_WRAP_CLAMP;
	t->data.wrap_r = NYAS_TEX_WRAP_CLAMP;
	t->img = NULL;

	size = 512 * 512 * 2 * 2;
	struct nyas_texture_image *img = nyarr_nyteximg_push(&t->img);

	img->lod = 0;
	img->pix = nyas_alloc(size);
	fread(img->pix, size, 1, f);
	NYAS_ASSERT(img->pix && "The image couldn't be loaded");
	fclose(f);
}
