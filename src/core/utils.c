#include "utils.h"

#include "core/io.h"
#include "core/mem.h"
#include "render/pixels_internal.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

the_mesh THE_UTILS_SPHERE;
the_mesh THE_UTILS_CUBE;
the_mesh THE_UTILS_QUAD;

typedef struct the_job job;
THE_DECL_ARR(job);

struct tut_asset_loader {
	struct thearr_job *seq;
	struct thearr_job *async;
};

static void
load_tex(void *arg)
{
	struct tut_tex_ldargs *a = arg;
	the_tex_load(a->tex, &a->desc, a->path);
}

static void
load_mesh(void *arg)
{
	struct tut_mesh_ldargs *a = arg;
	*a->mesh = the_mesh_load_file(a->path); // TODO: separate create and load like textures in
	                                         // order to avoid concurrent writes in mesh_pool
}

static void
load_shader(void *arg)
{
	struct tut_shad_ldargs *a = arg;
	*a->shader = the_shader_create(&a->desc);
	// TODO: Shaders compilation and program linking.
}

static void
load_env(void *args)
{
	struct tut_env_ldargs *ea = args;
	tut_env_load(ea->path, ea->lut, ea->sky, ea->irr, ea->pref);
}

struct tut_asset_loader *
tut_assets_create(void)
{
	struct tut_asset_loader *l = malloc(sizeof(struct tut_asset_loader));
	l->async = NULL;
	l->seq = NULL;
	return l;
}

void
tut_assets_add_mesh(struct tut_asset_loader *l, struct tut_mesh_ldargs *args)
{
	job *job = thearr_job_push(&l->async);
	job->job = load_mesh;
	job->args = args;
}

void
tut_assets_add_tex(struct tut_asset_loader *l, struct tut_tex_ldargs *args)
{
	job *job = thearr_job_push(&l->async);
	job->job = load_tex;
	job->args = args;
}

void
tut_assets_add_shader(struct tut_asset_loader *l, struct tut_shad_ldargs *args)
{
	job *job = thearr_job_push(&l->seq);
	job->job = load_shader;
	job->args = args;
}

void
tut_assets_add_env(struct tut_asset_loader *l, struct tut_env_ldargs *args)
{
	job *job = thearr_job_push(&l->async);
	job->job = load_env;
	job->args = args;
}

void
tut_assets_add_job(struct tut_asset_loader *l, struct the_job j, bool async)
{
	if (async) {
		thearr_job_push_value(&l->async, j);
	} else {
		thearr_job_push_value(&l->seq, j);
	}
}

void
tut_assets_load(struct tut_asset_loader *l, int threads)
{
	thesched *load_sched = NULL;
	if (l->async) {
		load_sched = the_sched_create(threads);
		for (int i = 0; i < l->async->count; ++i) {
			the_sched_do(load_sched, l->async->at[i]);
		}
	}

	if (l->seq) {
		for (int i = 0; i < l->seq->count; ++i) {
			(*l->seq->at[i].job)(l->seq->at[i].args);
		}
	}

	if (l->async) {
		the_sched_wait(load_sched); // TODO(Check): sched_destroy waits?
		the_sched_destroy(load_sched);
	}
	free(l->seq);
	free(l->async);
	free(l);
}

static void
the__mesh_set_cube(mesh *mesh)
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

	static const the_idx INDICES[] = {
	  0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
	  13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
	};

	the_free(mesh->vtx);
	the_free(mesh->idx);

	mesh->attrib = (1 << THE_VA_POS) | (1 << THE_VA_NORMAL) | (1 << THE_VA_UV);
	mesh->vtx = the_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = the_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

static void
the__mesh_set_sphere(mesh *mesh, int x_segments, int y_segments)
{
	THE_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

	const float x_step = 1.0f / (float)(y_segments - 1);
	const float y_step = 1.0f / (float)(x_segments - 1);

	the_free(mesh->vtx);
	the_free(mesh->idx);

	mesh->attrib = (1 << THE_VA_POS) | (1 << THE_VA_NORMAL) | (1 << THE_VA_UV);
	mesh->vtx_size = y_segments * x_segments * 8 * sizeof(float);
	mesh->elem_count = y_segments * x_segments * 6;
	mesh->vtx = the_alloc(mesh->vtx_size);
	mesh->idx = the_alloc(mesh->elem_count * sizeof(the_idx));

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

	the_idx *i = mesh->idx;
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
the__mesh_set_quad(mesh *mesh)
{
	static const float VERTICES[] = {
	  -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f,
	  0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	};

	static const the_idx INDICES[] = { 0, 1, 2, 0, 2, 3 };

	the_free(mesh->vtx);
	the_free(mesh->idx);

	mesh->attrib = (1 << THE_VA_POS) | (1 << THE_VA_NORMAL) | (1 << THE_VA_UV);
	mesh->vtx = the_alloc(sizeof(VERTICES));
	memcpy(mesh->vtx, VERTICES, sizeof(VERTICES));
	mesh->idx = the_alloc(sizeof(INDICES));
	memcpy(mesh->idx, INDICES, sizeof(INDICES));
	mesh->vtx_size = sizeof(VERTICES);
	mesh->elem_count = sizeof(INDICES) / sizeof(*INDICES);
}

void tut_mesh_init_geometry(void)
{
	THE_UTILS_SPHERE = the_mesh_create();
	THE_UTILS_CUBE = the_mesh_create();
	THE_UTILS_QUAD = the_mesh_create();
	tut_mesh_set_geometry(THE_UTILS_SPHERE, THE_SPHERE);
	tut_mesh_set_geometry(THE_UTILS_CUBE, THE_CUBE);
	tut_mesh_set_geometry(THE_UTILS_QUAD, THE_QUAD);
}

void
tut_mesh_set_geometry(the_mesh msh, enum tut_geometry geo)
{
	mesh *m = &mesh_pool.buf->at[msh];

	switch (geo) {
	case THE_QUAD: the__mesh_set_quad(m); break;
	case THE_CUBE: the__mesh_set_cube(m); break;
	case THE_SPHERE: the__mesh_set_sphere(m, 32, 32); break;
	}
	m->res.flags |= THE_IRF_DIRTY;
}

void
tut_env_load(const char *path, the_tex *lut, the_tex *sky, the_tex *irr, the_tex *pref)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		THE_LOG_ERR("Failed to open file %s", path);
		return;
	}
	char hdr[9];
	if (fread(hdr, 8, 1, f) != 1) {
		THE_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
		fclose(f);
		return;
	}

	hdr[8] = '\0';
	if (strncmp("NYAS_ENV", hdr, 9) != 0) {
		THE_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
		fclose(f);
		return;
	}

	*sky = the_tex_create();
	tex *t = &tex_pool.buf->at[*sky];
	t->res.id = 0;
	t->res.flags = THE_IRF_DIRTY;
	t->data.type = THE_TEX_CUBEMAP;
	t->data.fmt = THE_TEX_FMT_RGB16F;
	t->data.width = 1024;
	t->data.height = 1024;
	t->data.mag_filter = THE_TEX_FLTR_LINEAR;
	t->data.min_filter = THE_TEX_FLTR_LINEAR;
	t->data.wrap_s = THE_TEX_WRAP_CLAMP;
	t->data.wrap_t = THE_TEX_WRAP_CLAMP;
	t->data.wrap_r = THE_TEX_WRAP_CLAMP;
	t->img = NULL;

	size_t size = 1024 * 1024 * 3 * 2; // size * nchannels * sizeof(channel)
	for (int i = 0; i < 6; ++i) {
		struct the_texture_image *img = thearr_theteximg_push(&t->img);
		img->lod = 0;
		img->face = i;
		img->pix = the_alloc(size);
		if (fread(img->pix, size, 1, f) != 1) {
			THE_LOG_ERR("Error reading .env file. Aborting %s.", path);
			fclose(f);
			return;
		}

		THE_ASSERT(img->pix && "The image couldn't be loaded");
	}

	*irr = the_tex_create();
	t = &tex_pool.buf->at[*irr];
	t->res.id = 0;
	t->res.flags = THE_IRF_DIRTY;
	t->data.type = THE_TEX_CUBEMAP;
	t->data.fmt = THE_TEX_FMT_RGB16F;
	t->data.width = 1024;
	t->data.height = 1024;
	t->data.mag_filter = THE_TEX_FLTR_LINEAR;
	t->data.min_filter = THE_TEX_FLTR_LINEAR;
	t->data.wrap_s = THE_TEX_WRAP_CLAMP;
	t->data.wrap_t = THE_TEX_WRAP_CLAMP;
	t->data.wrap_r = THE_TEX_WRAP_CLAMP;
	t->img = NULL;

	for (int i = 0; i < 6; ++i) {
		struct the_texture_image *img = thearr_theteximg_push(&t->img);
		img->lod = 0;
		img->face = i;
		img->pix = the_alloc(size);
		if (fread(img->pix, size, 1, f) != 1) {
			THE_LOG_ERR("Error reading .env file. Aborting %s.", path);
			fclose(f);
			return;
		}

		THE_ASSERT(img->pix && "The image couldn't be loaded");
	}

	*pref = the_tex_create();
	t = &tex_pool.buf->at[*pref];
	t->res.id = 0;
	t->res.flags = THE_IRF_DIRTY;
	t->data.type = THE_TEX_CUBEMAP;
	t->data.fmt = THE_TEX_FMT_RGB16F;
	t->data.width = 256;
	t->data.height = 256;
	t->data.mag_filter = THE_TEX_FLTR_LINEAR;
	t->data.min_filter = THE_TEX_FLTR_LINEAR_MIPMAP_LINEAR;
	t->data.wrap_s = THE_TEX_WRAP_CLAMP;
	t->data.wrap_t = THE_TEX_WRAP_CLAMP;
	t->data.wrap_r = THE_TEX_WRAP_CLAMP;
	t->img = NULL;

	size = 256 * 256 * 3 * 2;
	for (int lod = 0; lod < 9; ++lod) {
		for (int face = 0; face < 6; ++face) {
			struct the_texture_image *img = thearr_theteximg_push(&t->img);
			img->lod = lod;
			img->face = face;
			img->pix = the_alloc(size);
			if (fread(img->pix, size, 1, f) != 1) {
				THE_LOG_ERR("Error reading .env file. Aborting %s.", path);
				fclose(f);
				return;
			}
			THE_ASSERT(img->pix && "The image couldn't be loaded");
		}
		size /= 4;
	}

	*lut = the_tex_create();
	t = &tex_pool.buf->at[*lut];
	t->res.id = 0;
	t->res.flags = THE_IRF_DIRTY;
	t->data.type = THE_TEX_2D;
	t->data.fmt = THE_TEX_FMT_RG16F;
	t->data.width = 512;
	t->data.height = 512;
	t->data.mag_filter = THE_TEX_FLTR_LINEAR;
	t->data.min_filter = THE_TEX_FLTR_LINEAR;
	t->data.wrap_s = THE_TEX_WRAP_CLAMP;
	t->data.wrap_t = THE_TEX_WRAP_CLAMP;
	t->data.wrap_r = THE_TEX_WRAP_CLAMP;
	t->img = NULL;

	size = 512 * 512 * 2 * 2;
	struct the_texture_image *img = thearr_theteximg_push(&t->img);

	img->lod = 0;
	img->pix = the_alloc(size);
	if (fread(img->pix, size, 1, f) != 1) {
		THE_LOG_ERR("Error reading .env file. Aborting %s.", path);
		fclose(f);
		return;
	}
	THE_ASSERT(img->pix && "The image couldn't be loaded");
	fclose(f);
}
