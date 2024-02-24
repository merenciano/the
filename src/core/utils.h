#ifndef NYAS_UTILS_H
#define NYAS_UTILS_H

#include "core/nyas_core.h"
#include "core/sched.h"
#include "render/pixels.h"

struct nyutil_tex_ldinfo {
	struct nyas_texture_desc desc;
	const char *path;
	nyas_tex tex;
};

struct nyutil_mesh_ldinfo {
	const char *path;
	nyas_mesh *mesh;
};

struct nyutil_shad_ldinfo {
	struct nyas_shader_desc desc;
	nyas_shader *shader;
};

typedef struct nyas_job nyjob;
NYAS_DECL_ARR(nyjob);

struct nyutil_asset_loader {
	struct nyarr_nyjob *seq;
	struct nyarr_nyjob *async;
	int threads;
};

void nyutil_assets_add_mesh(struct nyutil_asset_loader *l, struct nyut_mesh_loadinfo *info);

void nyutil_assets_add_tex(struct nyutil_asset_loader *l, struct nyut_tex_info *info);

void nyutil_assets_add_shader(struct nyutil_asset_loader *l, struct nyut_shad_loadinfo *info);

inline void nyutil_load_batch(const struct nyutil_asset_loader *loader)
{
	nysched *load_sched = nyas_sched_create(loader->threads, loader->async->count);
	for (int i = 0; i < loader->async->count; ++i) {
		nyas_sched_do(load_sched, loader->async->at[i]);
	}

	for (int i = 0; i < loader->seq->count; ++i) {
		(*loader->seq->at[i].job)(loader->seq->at[i].args);
	}

	//nyas_sched_wait(load_sched); TODO(Check): sched_destroy waits?
	nyas_sched_destroy(load_sched);
}

#endif // NYAS_UTILS_H
