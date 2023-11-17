#include "internalresources.h"

nypx_imesh *meshes;
nypx_itex *textures;
nypx_ifb *framebuffers;
nypx_ishd *shaders;
int mesh_count;
int texture_count;
int framebuffer_count;
int shader_count;

bool
nypx_resource_check(void *rsrc)
{
	return rsrc && ((nypx_irsrc *)rsrc)->id >= 0;
}
