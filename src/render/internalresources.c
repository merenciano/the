#include "internalresources.h"

//nypx_itex *textures;
nypx_ifb *framebuffers;
nypx_ishd *shaders;

//int texture_count;
int framebuffer_count;
int shader_count;

nyas_arr mesh_pool;
nyas_arr tex_pool;
nyas_arr shader_pool;
nyas_arr framebuffer_pool;

bool
nypx_resource_check(void *rsrc)
{
	return rsrc && ((nypx_irsrc *)rsrc)->id >= 0;
}
