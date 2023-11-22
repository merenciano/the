#include "internalresources.h"

nyas_arr mesh_pool;
nyas_arr tex_pool;
nyas_arr shader_pool;
nyas_arr framebuffer_pool;

bool
nypx_resource_check(void *rsrc)
{
	return rsrc && ((nypx_irsrc *)rsrc)->id >= 0;
}

nyas_cmd_queue render_queue;
