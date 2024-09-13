#include "pixels.h"

#include "core/io.h"
#include "core/mem.h"
#include "render/pixels_internal.h"

#include <mathc.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC the_alloc
#define STBI_REALLOC the_realloc
#define STBI_FREE the_free
#endif

#include "stb_image.h"

#ifndef TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_MALLOC the_alloc
#define TINYOBJ_REALLOC the_realloc
#define TINYOBJ_CALLOC the_calloc
#define TINYOBJ_FREE the_free
#endif

#include "tinyobj_loader_c.h"

#define THE_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA (8)

THE_IMPL_ARR(mesh);
THE_IMPL_POOL(mesh);
THE_IMPL_ARR(tex);
THE_IMPL_POOL(tex);
THE_IMPL_ARR(shad);
THE_IMPL_POOL(shad);
THE_IMPL_ARR(fb);
THE_IMPL_POOL(fb);

THE_IMPL_ARR(theteximg);

static void
dummyfree(void *p)
{
	(void)p;
}

THE_IMPL_ARR_MA(thedrawcmd, the_falloc, dummyfree);

static inline void
thepx__check_handle(int h, void *arr)
{
	struct thepool_tex *pool = arr;
	(void)h;
	(void)pool;
	THE_ASSERT(arr && h >= 0 && pool->buf->count > h && "Invalid handle range.");
}

struct thepool_mesh mesh_pool = { .buf = NULL, .count = 0, .next = 0 };
struct thepool_tex tex_pool = { .buf = NULL, .count = 0, .next = 0 };
struct thepool_shad shader_pool = { .buf = NULL, .count = 0, .next = 0 };
struct thepool_fb framebuffer_pool = { .buf = NULL, .count = 0, .next = 0 };

static struct the_mem *circbuf = NULL;

void
the_falloc_set_buffer(void *buffer, ptrdiff_t size)
{
	THE_ASSERT(buffer && size > 0);
	circbuf = buffer;
	circbuf->cap = size - sizeof(*circbuf);
	circbuf->tail = 0;
}

void *
the_falloc(ptrdiff_t size)
{
	THE_ASSERT(size > 0);
	return the_circalloc(circbuf, size);
}

static void
the__file_reader(void *_1, const char *path, int _2, const char *_3, char **buf, size_t *size)
{
	(void)_1, (void)_2, (void)_3;
	the_file_read(path, buf, size);
}

static the_mesh
the__create_mesh_handle(void)
{
	return thepool_mesh_add(&mesh_pool);
}

static the_framebuffer
the__create_fb_handle(void)
{
	return thepool_fb_add(&framebuffer_pool);
}

static the_shader
the__create_shader_handle(void)
{
	return thepool_shad_add(&shader_pool);
}

const char *
the__face_img_path(const char *path, int face, int face_count)
{
	switch (face_count) {
	case 6:
	case 5:
	case 4:
	case 3:
	case 2: {
		const char *suffixes = "RLUDFB";
		static char buffer[1024];
		int count = snprintf(buffer, 1024, path, suffixes[face]);
		if (count < 1024) {
			THE_LOG_ERR("Cubemap face path format: %s is too long!", path);
			return NULL;
		}
		return buffer;
	}
	case 1: return path;
	default: THE_LOG_WARN("Invalid cubemap face count. Path is %s.", path); return path;
	}
}

static int
the__tex_channels(the_texture_format fmt)
{
	switch (fmt) {
	case THE_TEX_FMT_RGBA16F:
	case THE_TEX_FMT_RGBA8: return 4;
	case THE_TEX_FMT_RGB16F:
	case THE_TEX_FMT_RGB8:
	case THE_TEX_FMT_SRGB: return 3;
	case THE_TEX_FMT_RG16F:
	case THE_TEX_FMT_RG8: return 2;
	case THE_TEX_FMT_R16F:
	case THE_TEX_FMT_R8: return 1;
	default: return 0;
	}
}

static int
the__tex_faces(the_texture_type type)
{
	switch (type) {
	case THE_TEX_2D:
	case THE_TEX_ARRAY_2D: return 1;
	case THE_TEX_CUBEMAP:
	case THE_TEX_ARRAY_CUBEMAP: return 6;
	default: return 0;
	}
}

static bool
the__tex_is_float(the_texture_format fmt)
{
	switch (fmt) {
	case THE_TEX_FMT_RGBA16F:
	case THE_TEX_FMT_RGB16F:
	case THE_TEX_FMT_RGB32F:
	case THE_TEX_FMT_RG16F:
	case THE_TEX_FMT_R16F: return true;
	default: return false;
	}
}

the_tex
the_tex_create(void)
{
	int tex = thepool_tex_add(&tex_pool);
	tex_pool.buf->at[tex].res = (struct the_resource_internal){ .id = 0, .flags = 0 };
	tex_pool.buf->at[tex].data = (struct the_texture_desc){
		.flags = THE_TEX_FLAG_DEFAULT,
		.type = THE_TEX_2D,
		.width = 0,
		.height = 0,
		.fmt = THE_TEX_FMT_SRGB,
		.min_filter = THE_TEX_FLTR_LINEAR,
		.mag_filter = THE_TEX_FLTR_LINEAR,
		.wrap_s = THE_TEX_WRAP_REPEAT,
		.wrap_t = THE_TEX_WRAP_REPEAT,
		.wrap_r = THE_TEX_WRAP_REPEAT,
		.border_color = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
	tex_pool.buf->at[tex].img = NULL;
	return tex;
}

void
the_tex_load(the_tex texture, struct the_texture_desc *desc, const char *path)
{
	THE_ASSERT(*path != '\0' && "For empty textures use the_tex_set");
	tex *t = &tex_pool.buf->at[texture];
	t->res.id = 0;
	t->res.flags = THE_IRF_DIRTY;
	t->data = *desc;

	int fmt_ch = the__tex_channels(t->data.fmt);
	stbi_set_flip_vertically_on_load(t->data.flags & THE_TEX_FLAG_FLIP_VERTICALLY_ON_LOAD);

	int channels = 0;
	int face_count = the__tex_faces(t->data.type);
	for (int i = 0; i < face_count; ++i) {
		struct the_texture_image *img = thearr_theteximg_push(&t->img);
		const char *p = the__face_img_path(path, i, face_count);
		img->lod = 0;
		img->face = i;
		if (the__tex_is_float(t->data.fmt)) {
			img->pix = stbi_loadf(p, &t->data.width, &t->data.height, &channels, fmt_ch);
		} else {
			img->pix = stbi_load(p, &t->data.width, &t->data.height, &channels, fmt_ch);
		}

		if (!img->pix) {
			THE_LOG_ERR("The image '%s' couldn't be loaded", p);
		}
	}
}

void
the_tex_set(the_tex texture, struct the_texture_desc *desc)
{
	THE_ASSERT(desc->width > 0 && desc->height > 0 && "Incorrect dimensions");
	tex *t = &tex_pool.buf->at[texture];
	t->res.flags |= THE_IRF_DIRTY;
	t->data = *desc;
	if (!t->img) {
		int face_count = the__tex_faces(t->data.type);
		for (int i = 0; i < face_count; ++i) {
			struct the_texture_image *img = thearr_theteximg_push(&t->img);
			img->lod = 0;
			img->face = i;
			img->pix = NULL;
		}
	}
}

struct the_point
the_tex_size(the_tex tex)
{
	return (struct the_point){
		tex_pool.buf->at[tex].data.width, tex_pool.buf->at[tex].data.height
	};
}

the_shader
the_shader_create(const struct the_shader_desc *desc)
{
	the_shader ret = the__create_shader_handle();
	shader_pool.buf->at[ret].name = desc->name;
	shader_pool.buf->at[ret].res.id = 0;
	shader_pool.buf->at[ret].res.flags = THE_IRF_DIRTY;
	shader_pool.buf->at[ret].count[0].data = desc->data_count;
	shader_pool.buf->at[ret].count[0].tex = desc->tex_count;
	shader_pool.buf->at[ret].count[0].cubemap = desc->cubemap_count;
	shader_pool.buf->at[ret].count[1].data = desc->shared_data_count;
	shader_pool.buf->at[ret].count[1].tex = desc->common_tex_count;
	shader_pool.buf->at[ret].count[1].cubemap = desc->common_cubemap_count;
	shader_pool.buf->at[ret].common = the_alloc(
	  (desc->shared_data_count + desc->common_tex_count + desc->common_cubemap_count) *
	  sizeof(float));
	return ret;
}

void *
the_shader_data(the_shader shader)
{
	return shader_pool.buf->at[shader].common;
}

the_tex *
the_shader_tex(the_shader shader)
{
	shad *shdr = &shader_pool.buf->at[shader];
	return (the_tex *)shdr->common + shdr->count[1].data;
}

the_tex *
the_shader_cubemap(the_shader shader)
{
	shad *shdr = &shader_pool.buf->at[shader];
	return the_shader_tex(shader) + shdr->count[1].tex;
}

void
the_shader_reload(the_shader shader)
{
	shader_pool.buf->at[shader].res.flags |= THE_IRF_DIRTY;
}

static the_idx
check_vertex(const float *v, const float *end, const float *newvtx)
{
	the_idx i = 0;
	for (; v < end; ++i, v += 14) {
		if ((v[0] == newvtx[0]) && (v[1] == newvtx[1]) && (v[2] == newvtx[2]) &&
		    (v[3] == newvtx[3]) && (v[4] == newvtx[4]) && (v[5] == newvtx[5]) &&
		    (v[12] == newvtx[12]) && (v[13] == newvtx[13])) {
			return i;
		}
	}
	return i;
}

static void
the__mesh_set_obj(mesh *mesh, const char *path)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t *shapes = NULL;
	size_t shape_count;
	tinyobj_material_t *mats = NULL;
	size_t mats_count;

	int result = tinyobj_parse_obj(
	  &attrib, &shapes, &shape_count, &mats, &mats_count, path, the__file_reader, NULL,
	  TINYOBJ_FLAG_TRIANGULATE);

	THE_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
	if (result != TINYOBJ_SUCCESS) {
		THE_LOG_ERR("Error loading obj. Err: %d", result);
	}

	size_t vertex_count = attrib.num_face_num_verts * 3;

	the_free(mesh->vtx);
	the_free(mesh->idx);

	mesh->attrib = (1 << THE_VA_POS) | (1 << THE_VA_NORMAL) | (1 << THE_VA_TAN) |
	  (1 << THE_VA_BITAN) | (1 << THE_VA_UV);
	mesh->vtx_size = vertex_count * 14 * sizeof(float);
	mesh->elem_count = vertex_count;
	mesh->vtx = the_alloc(mesh->vtx_size);
	mesh->idx = the_alloc(mesh->elem_count * sizeof(the_idx));

	float *vit = mesh->vtx;

	size_t index_offset = 0;
	for (size_t i = 0; i < attrib.num_face_num_verts; ++i) {
		for (int f = 0; f < attrib.face_num_verts[i] / 3; ++f) {
			tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset];
			float v1[14], v2[14], v3[14];

			v1[0] = attrib.vertices[3 * idx.v_idx + 0];
			v1[1] = attrib.vertices[3 * idx.v_idx + 1];
			v1[2] = attrib.vertices[3 * idx.v_idx + 2];
			v1[3] = attrib.normals[3 * idx.vn_idx + 0];
			v1[4] = attrib.normals[3 * idx.vn_idx + 1];
			v1[5] = attrib.normals[3 * idx.vn_idx + 2];
			v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset + 1];
			v2[0] = attrib.vertices[3 * idx.v_idx + 0];
			v2[1] = attrib.vertices[3 * idx.v_idx + 1];
			v2[2] = attrib.vertices[3 * idx.v_idx + 2];
			v2[3] = attrib.normals[3 * idx.vn_idx + 0];
			v2[4] = attrib.normals[3 * idx.vn_idx + 1];
			v2[5] = attrib.normals[3 * idx.vn_idx + 2];
			v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			idx = attrib.faces[3 * f + index_offset + 2];
			v3[0] = attrib.vertices[3 * idx.v_idx + 0];
			v3[1] = attrib.vertices[3 * idx.v_idx + 1];
			v3[2] = attrib.vertices[3 * idx.v_idx + 2];
			v3[3] = attrib.normals[3 * idx.vn_idx + 0];
			v3[4] = attrib.normals[3 * idx.vn_idx + 1];
			v3[5] = attrib.normals[3 * idx.vn_idx + 2];
			v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
			v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

			// Calculate tangent and bitangent
			float delta_p1[3], delta_p2[3], delta_uv1[2], delta_uv2[2];
			vec3_subtract(delta_p1, v2, v1);
			vec3_subtract(delta_p2, v3, v1);
			vec2_subtract(delta_uv1, &v2[12], &v1[12]);
			vec2_subtract(delta_uv2, &v3[12], &v1[12]);
			float r = 1.0f / (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

			float tn[3], bitn[3], tmp[3];
			vec3_multiply_f(tn, delta_p1, delta_uv2[1]);
			vec3_multiply_f(tmp, delta_p2, delta_uv1[1]);
			vec3_multiply_f(tn, vec3_subtract(tn, tn, tmp), r);

			vec3_multiply_f(bitn, delta_p2, delta_uv1[0]);
			vec3_multiply_f(tmp, delta_p1, delta_uv2[0]);
			vec3_multiply_f(bitn, vec3_subtract(bitn, bitn, tmp), r);

			v1[6] = tn[0];
			v1[7] = tn[1];
			v1[8] = tn[2];
			v2[6] = tn[0];
			v2[7] = tn[1];
			v2[8] = tn[2];
			v3[6] = tn[0];
			v3[7] = tn[1];
			v3[8] = tn[2];

			v1[9] = bitn[0];
			v1[10] = bitn[1];
			v1[11] = bitn[2];
			v2[9] = bitn[0];
			v2[10] = bitn[1];
			v2[11] = bitn[2];
			v3[9] = bitn[0];
			v3[10] = bitn[1];
			v3[11] = bitn[2];

			// Check vertex rep
			the_idx nxt_idx = check_vertex(mesh->vtx, vit, v1);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v1[j];
				}
			}

			nxt_idx = check_vertex(mesh->vtx, vit, v2);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v2[j];
				}
			}

			nxt_idx = check_vertex(mesh->vtx, vit, v3);
			mesh->idx[index_offset++] = nxt_idx;
			if (nxt_idx * 14 == (vit - mesh->vtx)) {
				for (int j = 0; j < 14; ++j) {
					*vit++ = v3[j];
				}
			}
		}
	}

	tinyobj_attrib_free(&attrib);
	tinyobj_shapes_free(shapes, shape_count);
	tinyobj_materials_free(mats, mats_count);
}

static void
the__mesh_set_msh(mesh *mesh, const char *path)
{
	char *data;
	size_t sz;
	the__file_reader(NULL, path, 0, NULL, &data, &sz);
	if (!data || !sz) {
		THE_LOG_ERR("Problem reading file %s", path);
		return;
	}

	the_free(mesh->vtx);
	the_free(mesh->idx);

	mesh->attrib = (1 << THE_VA_POS) | (1 << THE_VA_NORMAL) | (1 << THE_VA_TAN) |
	  (1 << THE_VA_BITAN) | (1 << THE_VA_UV);
	mesh->vtx_size = *(size_t *)data; // *(((size_t *)data)++)
	data += sizeof(size_t);
	mesh->vtx = the_alloc(mesh->vtx_size);
	memcpy(mesh->vtx, data, mesh->vtx_size);
	data += mesh->vtx_size;

	mesh->elem_count = (*(size_t *)data) / sizeof(the_idx); // *(((size_t *)data)++)
	data += sizeof(size_t);
	mesh->idx = the_alloc(mesh->elem_count * sizeof(the_idx));
	memcpy(mesh->idx, data, mesh->elem_count * sizeof(the_idx));

	the_free(data - mesh->vtx_size - (2 * sizeof(size_t)));
}

void
the_mesh_reload_file(the_mesh msh, const char *path)
{
	mesh *m = &mesh_pool.buf->at[msh];
	size_t len = strlen(path);
	const char *extension = path + len;
	while (*--extension != '.') {}
	extension++;
	if (!strcmp(extension, "obj")) {
		the__mesh_set_obj(m, path);
	} else if (!strcmp(extension, "msh")) {
		the__mesh_set_msh(m, path);
	} else {
		THE_LOG_ERR("Extension (%s) of file %s not recognised.", extension, path);
	}

	m->res.flags |= THE_IRF_DIRTY;
}

static the_mesh
the__mesh_create(void)
{
	the_mesh mesh_handle = the__create_mesh_handle();
	mesh_pool.buf->at[mesh_handle].res.id = 0;
	mesh_pool.buf->at[mesh_handle].res.flags = THE_IRF_DIRTY;
	mesh_pool.buf->at[mesh_handle].attrib = 0;
	mesh_pool.buf->at[mesh_handle].vtx = NULL;
	mesh_pool.buf->at[mesh_handle].idx = NULL;
	mesh_pool.buf->at[mesh_handle].vtx_size = 0;
	mesh_pool.buf->at[mesh_handle].elem_count = 0;
	mesh_pool.buf->at[mesh_handle].res_vb.id = 0;
	mesh_pool.buf->at[mesh_handle].res_vb.flags = THE_IRF_DIRTY;
	mesh_pool.buf->at[mesh_handle].res_ib.id = 0;
	mesh_pool.buf->at[mesh_handle].res_ib.flags = THE_IRF_DIRTY;

	return mesh_handle;
}

the_mesh
the_mesh_create(void)
{
	return the__mesh_create();
}

the_mesh
the_mesh_load_file(const char *path)
{
	the_mesh mesh_handle = the__mesh_create();
	the_mesh_reload_file(mesh_handle, path);
	return mesh_handle;
}

the_framebuffer
the_fb_create(void)
{
	the_framebuffer framebuffer = the__create_fb_handle();
	framebuffer_pool.buf->at[framebuffer].res.id = 0;
	framebuffer_pool.buf->at[framebuffer].res.flags = THE_IRF_DIRTY;
	for (int i = 0; i < 8; ++i) {
		framebuffer_pool.buf->at[framebuffer].target[i].tex = THE_NONE;
	}
	return framebuffer;
}

void
the_fb_set_target(the_framebuffer framebuffer, int index, struct the_texture_target target)
{
	framebuffer_pool.buf->at[framebuffer].res.flags |= THE_IRF_DIRTY;
	framebuffer_pool.buf->at[framebuffer].target[index] = target;
}

the_mat
the_mat_create(the_shader shader)
{
	the_mat ret = { .ptr = NULL, .shader = shader };
	shad *s = &shader_pool.buf->at[shader];
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = the_alloc(elements * sizeof(float));
	return ret;
}

the_mat
the_mat_tmp(the_shader shader)
{
	the_mat ret = { .ptr = NULL, .shader = shader };
	shad *s = &shader_pool.buf->at[shader];
	int elements = s->count[0].data + s->count[0].tex + s->count[0].cubemap;
	ret.ptr = the_falloc(elements * sizeof(float));
	return ret;
}

the_mat
the_mat_copy(the_mat mat)
{
	the_mat ret = { .shader = mat.shader };
	shad *s = &shader_pool.buf->at[mat.shader];
	size_t size = (s->count[0].data + s->count[0].tex + s->count[0].cubemap) * 4;
	ret.ptr = the_falloc(size);
	memcpy(ret.ptr, mat.ptr, size);
	return ret;
}

the_mat
the_mat_copy_shader(the_shader shader)
{
	the_mat ret = { .ptr = NULL, .shader = shader };
	shad *s = &shader_pool.buf->at[shader];
	int elements = s->count[1].data + s->count[1].tex + s->count[1].cubemap;
	ret.ptr = the_falloc(elements * sizeof(float));
	memcpy(ret.ptr, s->common, elements * sizeof(float));
	return ret;
}

the_tex *
the_mat_tex(the_mat mat)
{
	return (the_tex *)mat.ptr + shader_pool.buf->at[mat.shader].count[0].data;
}

static void
the__sync_gpu_mesh(the_mesh msh, the_shader shader)
{
	thepx__check_handle(msh, &mesh_pool);
	thepx__check_handle(shader, &shader_pool);
	mesh *m = &mesh_pool.buf->at[msh];

	if (!(m->res.flags & THE_IRF_CREATED)) {
		thepx_mesh_create(&m->res.id, &m->res_vb.id, &m->res_ib.id);
		m->res.flags |= THE_IRF_CREATED;
	}

	if (m->res.flags & THE_IRF_DIRTY) {
		thepx_mesh_set(m, shader_pool.buf->at[shader].res.id);
		m->res.flags &= ~THE_IRF_DIRTY;
	}
}

static tex *
the__sync_gpu_tex(the_tex texture)
{
	thepx__check_handle(texture, &tex_pool);
	tex *t = &tex_pool.buf->at[texture];
	if (!(t->res.flags & THE_IRF_CREATED)) {
		thepx_tex_create(t);
		t->res.flags |= (THE_IRF_CREATED | THE_IRF_DIRTY);
	}

	if (t->res.flags & THE_IRF_DIRTY) {
		thepx_tex_set(t);
		t->res.flags &= ~THE_IRF_DIRTY;
	}
	return t;
}

static void
the__set_shader_data(shad *s, void *srcdata, int common)
{
	THE_ASSERT((common == 0 || common == 1) && "Invalid common value.");

	int dc = s->count[common].data;
	int tc = s->count[common].tex;
	int cc = s->count[common].cubemap;
	int dl = s->loc[common].data;
	int tl = s->loc[common].tex;
	int cl = s->loc[common].cubemap;
	int tex_unit = THE_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA * common;

	THE_ASSERT((dc >= 0) && (tc >= 0) && (cc >= 0));
	THE_ASSERT(tex_unit >= 0 && tex_unit < 128);

	if (!(dc + tc + cc)) {
		return;
	}

	// change texture handle for texture internal id
	the_tex *data_tex = (the_tex *)srcdata + dc;
	for (int i = 0; i < tc + cc; ++i) {
		tex *itx = the__sync_gpu_tex(data_tex[i]);
		data_tex[i] = (int)itx->res.id;
	}

	// set opengl uniforms
	if (dc) {
		thepx_shader_set_data(dl, srcdata, dc / 4);
	}

	if (tc) {
		thepx_shader_set_tex(tl, data_tex, tc, tex_unit);
	}

	if (cc) {
		thepx_shader_set_cube(cl, data_tex + tc, cc, tex_unit + tc);
	}
}

void
the__sync_shader(shad *s)
{
	static const char *uniforms[] = {
		"u_data", "u_tex", "u_cube", "u_shared_data", "u_common_tex", "u_common_cube"
	};

	if (!(s->res.flags & THE_IRF_CREATED)) {
		thepx_shader_create(&s->res.id);
		s->res.flags |= THE_IRF_CREATED;
	}

	if (s->res.flags & THE_IRF_DIRTY) {
		THE_ASSERT(s->name && *s->name && "Shader name needed.");
		thepx_shader_compile(s->res.id, s->name);
		thepx_shader_loc(s->res.id, &s->loc[0].data, &uniforms[0], 6);
		s->res.flags &= ~THE_IRF_DIRTY;
	}
}

static void
the__fb_sync(the_framebuffer framebuffer)
{
	thepx__check_handle(framebuffer, &framebuffer_pool);
	struct the_framebuffer_internal *fb = &framebuffer_pool.buf->at[framebuffer];
	if (!(fb->res.flags & THE_IRF_CREATED)) {
		thepx_fb_create(fb);
		fb->res.flags |= THE_IRF_CREATED;
	}

	thepx_fb_use(fb->res.id);
	if (fb->res.flags & THE_IRF_DIRTY) {
		for (int i = 0; fb->target[i].tex != THE_NONE; ++i) {
			struct the_texture_internal *t = the__sync_gpu_tex(fb->target[i].tex);
			thepx_fb_set(fb->res.id, t->res.id, &fb->target[i]);
		}
		fb->res.flags &= ~THE_IRF_DIRTY;
	}
}

void
the_draw(struct the_draw *dl)
{
	{ // target
		the_framebuffer framebuf = dl->state.target.fb;
		if (framebuf != THE_NOOP) {
			if (framebuf == THE_DEFAULT) {
				thepx_fb_use(0);
			} else {
				thepx__check_handle(framebuf, &framebuffer_pool);
				the__fb_sync(framebuf);
			}
		}
	}

	{ // pipeline
		the_mat mat = dl->state.pipeline.shader_mat;
		if (mat.shader != THE_NOOP) {
			thepx__check_handle(mat.shader, &shader_pool);
			shad *s = &shader_pool.buf->at[mat.shader];
			the__sync_shader(s);
			thepx_shader_use(s->res.id);
			the__set_shader_data(s, mat.ptr, true);
		}
		// TODO: Mesh attrib
	}

	{ // ops
		const struct the_draw_ops *ops = &dl->state.ops;
		struct the_color bg = dl->state.target.bgcolor;
		thepx_clear_color(bg.r, bg.g, bg.b, bg.a);
		thepx_clear(
		  ops->enable & (1 << THE_DRAW_CLEAR_COLOR), ops->enable & (1 << THE_DRAW_CLEAR_DEPTH),
		  ops->enable & (1 << THE_DRAW_CLEAR_STENCIL));

		thepx_viewport(ops->viewport);
		thepx_scissor(ops->scissor);

		if (ops->disable & (1 << THE_DRAW_TEST_DEPTH)) {
			thepx_depth_disable_test();
		} else if (ops->enable & (1 << THE_DRAW_TEST_DEPTH)) {
			thepx_depth_enable_test();
		}

		if (ops->disable & (1 << THE_DRAW_WRITE_DEPTH)) {
			thepx_depth_disable_mask();
		} else if (ops->enable & (1 << THE_DRAW_WRITE_DEPTH)) {
			thepx_depth_enable_mask();
		}

		if (ops->disable & (1 << THE_DRAW_TEST_STENCIL)) {
			thepx_stencil_disable_test();
		} else if (ops->enable & (1 << THE_DRAW_TEST_STENCIL)) {
			thepx_stencil_enable_test();
		}

		if (ops->disable & (1 << THE_DRAW_WRITE_STENCIL)) {
			thepx_stencil_disable_mask();
		} else if (ops->enable & (1 << THE_DRAW_WRITE_STENCIL)) {
			thepx_stencil_enable_mask();
		}

		if (ops->disable & (1 << THE_DRAW_BLEND)) {
			thepx_blend_disable();
		} else if (ops->enable & (1 << THE_DRAW_BLEND)) {
			thepx_blend_enable();
		}

		if (ops->disable & (1 << THE_DRAW_CULL)) {
			thepx_cull_disable();
		} else if (ops->enable & (1 << THE_DRAW_CULL)) {
			thepx_cull_enable();
		}

		if (ops->disable & (1 << THE_DRAW_SCISSOR)) {
			thepx_scissor_disable();
		} else if (ops->enable & (1 << THE_DRAW_SCISSOR)) {
			thepx_scissor_enable();
		}

		thepx_depth_set(ops->depth_fun);
		thepx_blend_set(ops->blend_src, ops->blend_dst);
		thepx_cull_set(ops->cull_face);
	}

	// commands
	for (int cmd = 0; cmd < dl->cmds->count; ++cmd) {
		the_mesh msh = dl->cmds->at[cmd].mesh;
		the_mat mat = dl->cmds->at[cmd].material;
		mesh *imsh = &mesh_pool.buf->at[msh];
		thepx__check_handle(msh, &mesh_pool);
		THE_ASSERT(imsh->elem_count && "Attempt to draw an uninitialized mesh");

		if (imsh->res.flags & THE_IRF_DIRTY) {
			the__sync_gpu_mesh(msh, mat.shader);
		}

		shad *s = &shader_pool.buf->at[mat.shader];
		the__set_shader_data(s, mat.ptr, false);
		thepx_mesh_use(imsh, s);
		thepx_draw(imsh->elem_count, sizeof(the_idx) == 4);
	}
}

void the_render_sync()
{
	thepx_sync();
}

#if defined(THE_GLES2)
#include "pixels_gles2.c"
#else
#include "pixels_gl3.c"
#endif
