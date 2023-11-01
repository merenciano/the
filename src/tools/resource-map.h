#ifndef THE_TOOLS_RESOURCE_MAP_H
#define THE_TOOLS_RESOURCE_MAP_H

#include "core/hmap.h"
#include "render/renderer.h"

typedef struct {
	THE_HMap *meshes;
	THE_HMap *textures;
} THE_ResourceMap;

void THE_ResourceMapAddMeshFromPath(THE_ResourceMap *rm, const char *name, const char *path);
void THE_ResourceMapAddMesh(THE_ResourceMap *rm, const char *name, THE_Mesh mesh);
THE_Mesh THE_ResourceMapGetMesh(THE_ResourceMap *rm, const char *name);
void THE_ResourceMapAddTextureFromPath(THE_ResourceMap *rm, const char *name, const char *path, enum THE_TexType t);
void THE_ResourceMapAddTexture(THE_ResourceMap *rm, const char *name, int width, int height, enum THE_TexType t);
THE_Texture THE_ResourceMapGetTexture(THE_ResourceMap *rm, const char *name);

#endif
