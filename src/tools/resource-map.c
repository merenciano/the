#include "resource-map.h"

#include <core/log.h>

void
nyas_resourcemap_mesh_file(nyas_resourcemap *rm,
                           const char *name,
                           const char *path)
{
	nyas_mesh mesh = nyas_mesh_load_obj(path);
	nyas_hmap_insert(rm->meshes, name, &mesh);
}

void
nyas_resourcemap_mesh_add(nyas_resourcemap *rm,
                          const char *name,
                          nyas_mesh mesh)
{
	nyas_hmap_insert(rm->meshes, name, &mesh);
}

nyas_mesh
nyas_resourcemap_mesh(nyas_resourcemap *rm, const char *name)
{
	void *result = nyas_hmap_get(rm->meshes, name);
	NYAS_ASSERT(result != NYAS_HMAP_INVALID_VALUE && "Invalid result");
	return *(nyas_mesh *)result;
}

void
nyas_resourcemap_tex_file(nyas_resourcemap *rm,
                          const char *name,
                          const char *path,
                          enum nyas_textype t)
{
	NYAS_ASSERT(path && "Null path");
	nyas_tex tex = nyas_tex_load_img(path, t);
	nyas_hmap_insert(rm->textures, name, &tex);
}

void
nyas_resourcemap_tex_add(nyas_resourcemap *rm,
                         const char *name,
                         int width,
                         int height,
                         enum nyas_textype t)
{
	NYAS_ASSERT(width > 0 && height > 0 && "0,0 size texture is no texture");
	nyas_tex tex = nyas_tex_create(width, height, t);
	nyas_hmap_insert(rm->textures, name, &tex);
}

nyas_tex
nyas_resourcemap_tex(nyas_resourcemap *rm, const char *name)
{
	void *result = nyas_hmap_get(rm->textures, name);
	NYAS_ASSERT(result != NYAS_HMAP_INVALID_VALUE && "Invalid result");
	return *(nyas_tex *)result;
}
