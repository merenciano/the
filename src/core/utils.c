#include "utils.h"

static void load_tex(void *arg)
{
	struct nyutil_tex_ldinfo *info = arg;
	nyas_tex_load(info->tex, &info->desc, info->path);
}

static void load_mesh(void *arg)
{
	struct nyutil_mesh_ldinfo *info = arg;
	*info->mesh = nyas_mesh_load_file(info->path); // TODO: separate create and load like textures in order to avoid concurrent writes in mesh_pool
}

static void load_shader(void *arg)
{
	struct nyutil_shad_ldinfo *info = arg;
	*info->shader = nyas_shader_create(&info->desc);
	// TODO: Shaders compilation and program linking.
}

void nyutil_assets_add_mesh(struct nyutil_asset_loader *l, struct nyut_mesh_loadinfo *info)
{
	nyjob *job = nyarr_nyjob_push(&l->async);
	job->job = load_mesh;
	job->args = info;
}

void nyutil_assets_add_tex(struct nyutil_asset_loader *l, struct nyut_tex_info *info)
{
	nyjob *job = nyarr_nyjob_push(&l->async);
	job->job = load_tex;
	job->args = info;
}

void nyutil_assets_add_shader(struct nyutil_asset_loader *l, struct nyut_shad_loadinfo *info)
{
	nyjob *job = nyarr_nyjob_push(&l->seq);
	job->job = load_shader;
	job->args = info;
}
