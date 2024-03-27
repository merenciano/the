#ifndef NYAS_UTILS_H
#define NYAS_UTILS_H

#include "sched.h"
#include <nyas_core.h>
#include <nyas_draw.h>

#include <stdbool.h>

// Asset loader
struct nyut_tex_ldargs {
	struct nyas_texture_desc desc;
	const char *path;
	nyas_tex tex;
};

struct nyut_mesh_ldargs {
	const char *path;
	nyas_mesh *mesh;
};

struct nyut_shad_ldargs {
	struct nyas_shader_desc desc;
	nyas_shader *shader;
};

struct nyut_env_ldargs {
	const char *path;
	nyas_tex *sky;
	nyas_tex *irr;
	nyas_tex *pref;
	nyas_tex *lut;
};

typedef struct nyut_asset_loader nyut_assetldr;
nyut_assetldr *nyut_assets_create(void);
void nyut_assets_add_mesh(nyut_assetldr *l, struct nyut_mesh_ldargs *args);
void nyut_assets_add_tex(nyut_assetldr *l, struct nyut_tex_ldargs *args);
void nyut_assets_add_shader(nyut_assetldr *l, struct nyut_shad_ldargs *args);
void nyut_assets_add_env(nyut_assetldr *l, struct nyut_env_ldargs *args);
void nyut_assets_add_job(nyut_assetldr *l, struct nyas_job j, bool async);
void nyut_assets_load(nyut_assetldr *l, int threads);

// Geometry
enum nyut_geometry { NYAS_QUAD, NYAS_CUBE, NYAS_SPHERE };

extern nyas_mesh NYAS_UTILS_SPHERE;
extern nyas_mesh NYAS_UTILS_CUBE;
extern nyas_mesh NYAS_UTILS_QUAD;

void nyut_mesh_init_geometry(void);
void nyut_mesh_set_geometry(nyas_mesh mesh, enum nyut_geometry geo);

// Environment maps
void nyut_env_load(const char *path, nyas_tex *lut, nyas_tex *sky, nyas_tex *irr, nyas_tex *pref);

#endif // NYAS_UTILS_H
