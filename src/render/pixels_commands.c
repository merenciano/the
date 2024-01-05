#include "pixels_internal.h"

#include "core/io.h"
#include "core/log.h"
#include "core/mem.h"

#include <string.h>

#ifdef NYAS_OPENGL
#include <glad/glad.h>

#ifdef NYAS_ELEM_SIZE_16
#define ELEMENT_TYPE GL_UNSIGNED_SHORT
#else
#define ELEMENT_TYPE GL_UNSIGNED_INT
#endif

typedef struct nyas_internal_shader shdr_t;
typedef struct nyas_internal_texture tex_t;
typedef struct nyas_internal_mesh mesh_t;
typedef struct nyas_internal_framebuffer fb_t;

typedef struct {
	GLint internal_format;
	GLenum format;
	GLenum type;
	GLint wrap;
	GLint min_filter;
	GLint mag_filter;
} nyas_texcube_cnfg;

typedef struct {
	GLint internal_format;
	GLenum format;
	GLenum type;
	GLint wrap;
	GLint filter;
	int channels;
} nyas_tex_cnfg;

/*
  Attribute's number of elements for each vertex.
  The array's position must match with the
  enum (nyas_VertexAttributes) value of the attribute.
*/
static const GLint attrib_sizes[VTXATTR_COUNT] = { 3, 3, 3, 3, 2 };

/*
  Attribute's layout name in the shader.
  The array's position must match with the attribute's
  value at enum nyas_VertexAttributes.
*/
static const char *attrib_names[VTXATTR_COUNT] = { "a_position", "a_normal",
	                                               "a_tangent", "a_bitangent",
	                                               "a_uv" };

static bool
nypx__resource_check(void *rsrc)
{
	return rsrc && ((r_resource *)rsrc)->id >= 0;
}

static bool
nyas__is_dirty(void *resource)
{
	r_resource *r = resource;
	return r->flags & RF_DIRTY;
}

static void
nyas__enable_render_opt(int opt)
{
	switch (opt) {
	case NYAS_BLEND:
		glEnable(GL_BLEND);
		break;
	case NYAS_CULL_FACE:
		glEnable(GL_CULL_FACE);
		break;
	case NYAS_DEPTH_TEST:
		glEnable(GL_DEPTH_TEST);
		break;
	case NYAS_DEPTH_WRITE:
		glDepthMask(GL_TRUE);
		break;
	default:
		break;
	}
}

static void
nyas__disable_render_opt(int opt)
{
	switch (opt) {
	case NYAS_BLEND:
		glDisable(GL_BLEND);
		break;
	case NYAS_CULL_FACE:
		glDisable(GL_CULL_FACE);
		break;
	case NYAS_DEPTH_TEST:
		glDisable(GL_DEPTH_TEST);
		break;
	case NYAS_DEPTH_WRITE:
		glDepthMask(GL_FALSE);
		break;
	default:
		break;
	}
}

static GLenum
nyas__gl_blend(enum nyas_blendfn_opt bf)
{
	switch (bf) {
	case NYAS_BLEND_FUNC_ONE:
		return GL_ONE;
	case NYAS_BLEND_FUNC_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case NYAS_BLEND_FUNC_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case NYAS_BLEND_FUNC_ZERO:
		return GL_ZERO;
	default:
		return -1;
	}
}

static GLenum
nyas__gl_cull(nyas_cullface_opt cull)
{
	switch (cull) {
	case NYAS_CULL_FACE_BACK:
		return GL_BACK;
	case NYAS_CULL_FACE_FRONT:
		return GL_FRONT;
	case NYAS_CULL_FACE_FRONT_AND_BACK:
		return GL_FRONT_AND_BACK;
	default:
		return -1;
	}
}

static GLenum
nyas__gl_depth(nyas_depthfn_opt df)
{
	switch (df) {
	case NYAS_DEPTH_FUNC_LEQUAL:
		return GL_LEQUAL;
	case NYAS_DEPTH_FUNC_LESS:
		return GL_LESS;
	default:
		return -1;
	}
}

static void
nyas__sync_gpu_mesh(nyas_mesh mesh, nyas_shader shader)
{
	CHECK_HANDLE(mesh, mesh);
	CHECK_HANDLE(shader, shader);
	mesh_t *m = nyas_arr_at(mesh_pool, mesh);
	shdr_t *s = nyas_arr_at(shader_pool, shader);
	NYAS_ASSERT(nypx__resource_check(s) && "Invalid internal resource.");

	if (!(m->res.flags & NYAS_IRF_CREATED)) {
		nypx_mesh_create(&m->res.id, &m->res_vb.id, &m->res_ib.id);
		m->res.flags |= NYAS_IRF_DIRTY;
	}

	if (m->res.flags & NYAS_IRF_DIRTY) {
		nypx_mesh_set(m->res.id, m->res_vb.id, m->res_ib.id, s->res.id,
		              m->attrib, m->vtx, m->vtx_size, m->idx, m->elem_count);
	}
	m->res.flags = NYAS_IRF_CREATED;
	NYAS_ASSERT(nypx__resource_check(m) && "Invalid internal resource.");
}

static tex_t *
nyas__sync_gpu_tex(nyas_tex tex)
{
	CHECK_HANDLE(tex, tex);
	tex_t *t = nyas_arr_at(tex_pool, tex);
	if (!(t->res.flags & NYAS_IRF_CREATED)) {
		nypx_tex_create(&t->res.id, t->type);
		t->res.flags |= NYAS_IRF_DIRTY;
	}

	if (t->res.flags & NYAS_IRF_DIRTY) {
		nypx_tex_set(t->res.id, t->type, t->width, t->height, t->pix);
	}
	t->res.flags = NYAS_IRF_CREATED;
	NYAS_ASSERT(nypx__resource_check(t) && "Invalid internal resource.");
	return t;
}

static fb_t *
nyas__sync_gpu_fb(nyas_framebuffer fb)
{
	CHECK_HANDLE(framebuffer, fb);
	fb_t *ifb = nyas_arr_at(framebuffer_pool, fb);
	if (!(ifb->res.flags & NYAS_IRF_CREATED)) {
		nypx_fb_create(&ifb->res.id);
	}
	NYAS_ASSERT(nypx__resource_check(ifb) && "Invalid internal resource.");
	ifb->res.flags = NYAS_IRF_CREATED;
	return ifb;
}

static void
nyas__set_shader_data(shdr_t *s, void *srcdata, int common)
{
	NYAS_ASSERT((common == 0 || common == 1) && "Invalid common value.");
	NYAS_ASSERT(nypx__resource_check(s) && "Invalid internal shader.");

	size_t count = s->count[common].data + s->count[common].tex +
	  s->count[common].cubemap;
	if (!count) {
		return;
	}
	// copy for thread safety, uniform setting will be async (drawcommand)
	float *tmpdata = nyas_alloc_frame(count * sizeof(float));
	// copy numeric data
	memcpy(tmpdata, srcdata, s->count[common].data * sizeof(float));

	// set tex data (src {handle} --> dst {internal id})
	nyas_tex *srcdata_tex = (nyas_tex *)srcdata + s->count[common].data;
	uint32_t *tmpdata_texid = (uint32_t *)tmpdata + s->count[common].data;
	for (int i = 0; i < s->count[common].tex; ++i) {
		tex_t *itx = nyas__sync_gpu_tex(srcdata_tex[i]);
		*tmpdata_texid++ = itx->res.id;
	}

	// set tex cubemap data (src {handle} --> dst {internal id})
	nyas_tex *srcdata_cubemap = srcdata_tex + s->count[common].tex;
	for (int i = 0; i < s->count[common].cubemap; ++i) {
		tex_t *itx = nyas__sync_gpu_tex(srcdata_cubemap[i]);
		*tmpdata_texid++ = itx->res.id;
	}

	// set opengl uniforms
	int tuoffset = NYAS_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA * common;
	if (s->count[common].data) {
		nypx_shader_set_data(s->loc[common].data, tmpdata,
		                     s->count[common].data / 4);
		tmpdata += s->count[common].data;
	}

	if (s->count[common].tex) {
		nypx_shader_set_tex(s->loc[common].tex, (int *)tmpdata,
		                    s->count[common].tex, tuoffset);
		tmpdata += s->count[common].tex;
		tuoffset += s->count[common].tex;
	}

	if (s->count[common].cubemap) {
		nypx_shader_set_cube(s->loc[common].cubemap, (int *)tmpdata,
		                     s->count[common].cubemap, tuoffset);
	}
}

void
nyas_setshader_fn(nyas_cmdata *data)
{
	static const char *uniforms[] = { "u_data",       "u_tex",
		                              "u_cube",       "u_common_data",
		                              "u_common_tex", "u_common_cube" };

	CHECK_HANDLE(shader, data->mat.shader);
	shdr_t *s = nyas_arr_at(shader_pool, data->mat.shader);
	if (!(s->res.flags & NYAS_IRF_CREATED)) {
		nypx_shader_create(&s->res.id);
		NYAS_ASSERT(nypx__resource_check(s) && "Error shader creation.");
		s->res.flags |= NYAS_IRF_DIRTY;
	}

	if (s->res.flags & NYAS_IRF_DIRTY) {
		NYAS_ASSERT(s->name && *s->name && "Shader name needed.");
		nypx_shader_compile(s->res.id, s->name);
		nypx_shader_loc(s->res.id, &s->loc[0].data, &uniforms[0], 6);
		s->res.flags = NYAS_IRF_CREATED; // reset dirty flag
	}

	nypx_shader_use(s->res.id);
	nyas__set_shader_data(s, data->mat.ptr, true);
}

void
nyas_clear_fn(nyas_cmdata *data)
{
	GLbitfield mask = 0;
	if (data->clear.color_buffer) {
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (data->clear.depth_buffer) {
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (data->clear.stencil_buffer) {
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	glClearColor(data->clear.color[0], data->clear.color[1],
	             data->clear.color[2], data->clear.color[3]);
	glClear(mask);
}

void
nyas_draw_fn(nyas_cmdata *data)
{
	nyas_mesh mesh = data->draw.mesh;
	mesh_t *imsh = nyas_arr_at(mesh_pool, mesh);
	CHECK_HANDLE(mesh, mesh);
	NYAS_ASSERT(imsh->elem_count && "Attempt to draw an uninitialized mesh");

	if (imsh->res.flags & NYAS_IRF_DIRTY) {
		nyas__sync_gpu_mesh(mesh, data->draw.material.shader);
	}

	glBindVertexArray(imsh->res.id);
	shdr_t *s = nyas_arr_at(shader_pool, data->draw.material.shader);
	nyas__set_shader_data(s, data->draw.material.ptr, false);
	glDrawElements(GL_TRIANGLES, imsh->elem_count, ELEMENT_TYPE, 0);
	glBindVertexArray(0);
}

void
nyas_rops_fn(nyas_cmdata *data)
{
	int enable = data->rend_opts.enable_flags;
	int disable = data->rend_opts.disable_flags;
	for (int i = 0; i < NYAS_REND_OPTS_COUNT; ++i) {
		/* Prioritizing disable for bad configs (both on). */
		if (disable & (1 << i)) {
			nyas__disable_render_opt(1 << i);
		} else if (enable & (1 << i)) {
			nyas__enable_render_opt(1 << i);
		}
	}

	nyas_blend_fn blend = data->rend_opts.blend_func;
	/* Ignore unless both have a value assigned. */
	if (blend.src && blend.dst) {
		glBlendFunc(nyas__gl_blend(blend.src), nyas__gl_blend(blend.dst));
	}

	nyas_cullface_opt cull = data->rend_opts.cull_face;
	if (cull) {
		glCullFace(nyas__gl_cull(cull));
	}

	nyas_depthfn_opt depth = data->rend_opts.depth_func;
	if (depth) {
		glDepthFunc(nyas__gl_depth(depth));
	}
}

void
nyas_setfb_fn(nyas_cmdata *data)
{
	NYAS_ASSERT(data && "Null param");
	const nyas_set_fb_cmdata *d = &data->set_fb;
	if (d->fb == NYAS_DEFAULT) {
		nypx_fb_use(0);
	} else {
		fb_t *ifb = nyas__sync_gpu_fb(d->fb);
		nypx_fb_use(d->fb);
		if (d->attach.type != NYAS_IGNORE) {
			tex_t *t = nyas__sync_gpu_tex(d->attach.tex);
			nypx_fb_set(ifb->res.id, t->res.id, d->attach.type,
			            d->attach.mip_level, d->attach.face);
		}
	}
	nypx_viewport(0, 0, d->vp_x, d->vp_y);
}

#endif // NYAS_OPENGL
