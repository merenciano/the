#ifndef NYAS_TOOLS_RESOURCE_MAP_H
#define NYAS_TOOLS_RESOURCE_MAP_H

#include "core/hmap.h"
#include "render/renderer.h"

typedef struct {
	nyas_hmap *meshes;
	nyas_hmap *textures;
} nyas_resourcemap;

void nyas_resourcemap_mesh_file(nyas_resourcemap *rm,
                                const char *name,
                                const char *path);
void nyas_resourcemap_mesh_add(nyas_resourcemap *rm,
                               const char *name,
                               nyas_mesh mesh);
nyas_mesh nyas_resourcemap_mesh(nyas_resourcemap *rm, const char *name);
void nyas_resourcemap_tex_file(nyas_resourcemap *rm,
                               const char *name,
                               const char *path,
                               enum nyas_textype t);
void nyas_resourcemap_tex_add(nyas_resourcemap *rm,
                              const char *name,
                              int width,
                              int height,
                              enum nyas_textype t);
nyas_tex nyas_resourcemap_tex(nyas_resourcemap *rm, const char *name);

#endif // NYAS_TOOLS_RESOURCE_MAP_H
