#include "internalresources.h"

THE_InternalMesh *meshes;
THE_InternalTexture *textures;
THE_InternalFramebuffer *framebuffers;
THE_InternalShader *shaders;
int mesh_count;
int texture_count;
int framebuffer_count;
int shader_count;

bool
the__resource_check(void *r)
{
	return r && ((THE_InternalResource*)r)->id >= 0;
}
