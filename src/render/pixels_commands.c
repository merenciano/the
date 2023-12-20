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
static const GLint attrib_sizes[VERTEX_ATTRIBUTE_COUNT] = { 3, 3, 3, 3, 2 };

/*
  Attribute's layout name in the shader.
  The array's position must match with the attribute's
  value at enum nyas_VertexAttributes.
*/
static const char *attrib_names[VERTEX_ATTRIBUTE_COUNT] = {
	"a_position", "a_normal", "a_tangent", "a_bitangent", "a_uv"
};

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
nyas__attach_to_fb(nyas_fbattach a)
{
	int id = ((tex_t *)nyas_arr_at(tex_pool, a.tex))->res.id;
	GLenum target = a.side < 0 ? GL_TEXTURE_2D :
								 GL_TEXTURE_CUBE_MAP_POSITIVE_X + a.side;

	GLenum slot = a.slot == NYAS_ATTACH_COLOR ? GL_COLOR_ATTACHMENT0 :
												GL_DEPTH_ATTACHMENT;

	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, target, id, a.level);
}

static void
nyas__set_viewport_from_fb(nyas_framebuffer fb)
{
	int w, h;
	nyas_fb_size(fb, &w, &h);
	glViewport(0, 0, w, h);
}

static void
nyas__set_viewport(int x, int y)
{
	if (x < 0) {
		nyas_v2i win_size = nyas_window_size();
		glViewport(0, 0, win_size.x, win_size.y);
	} else {
		glViewport(0, 0, x, y);
	}
}

static GLsizei
nyas__get_attrib_stride(int32_t attr_flags)
{
	GLsizei stride = 0;
	for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
		if (attr_flags & (1 << i)) {
			stride += attrib_sizes[i];
		}
	}
	return stride * sizeof(float);
}

static void
nyas__mesh_update(nyas_mesh mesh, nyas_shader shader)
{
	r_mesh *m = nyas_arr_at(mesh_pool, mesh);
	shdr_t *s = nyas_arr_at(shader_pool, shader);

	glBindVertexArray(m->res.id);
	glBindBuffer(GL_ARRAY_BUFFER, m->internal_buffers_id[0]);
	glBufferData(GL_ARRAY_BUFFER, m->vtx_size, m->vtx, GL_STATIC_DRAW);

	GLint offset = 0;
	GLsizei stride = nyas__get_attrib_stride(m->attr_flags);
	for (int i = 0; i < VERTEX_ATTRIBUTE_COUNT; ++i) {
		if (!(m->attr_flags & (1 << i))) {
			continue;
		}

		GLint size = attrib_sizes[i];
		GLint attrib_pos = glGetAttribLocation(s->res.id, attrib_names[i]);
		if (attrib_pos >= 0) {
			glEnableVertexAttribArray(attrib_pos);
			glVertexAttribPointer(attrib_pos, size, GL_FLOAT, GL_FALSE, stride,
			                      (void *)(offset * sizeof(float)));
		}
		offset += size;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->internal_buffers_id[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->elements * sizeof(nyas_idx),
	             (const void *)m->idx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (m->res.flags & RF_FREE_AFTER_LOAD) {
		if (m->vtx) {
			nyas_free(m->vtx);
			m->vtx = NULL;
		}
		if (m->idx) {
			nyas_free(m->idx);
			m->idx = NULL;
		}
	}
}

static void
nyas__create_mesh(nyas_mesh mesh, nyas_shader shader)
{
	r_mesh *m = nyas_arr_at(mesh_pool, mesh);

	glGenVertexArrays(1, (GLuint *)&m->res.id);
	glGenBuffers(2, m->internal_buffers_id);
	nyas__mesh_update(mesh, shader);
}

static void
nyas__sync_gpu_mesh(nyas_mesh m, nyas_shader s)
{
	r_mesh *im = nyas_arr_at(mesh_pool, m);
	if (im->res.id == NYAS_UNINIT) {
		nyas__create_mesh(m, s);
	} else if (im->res.flags & RF_DIRTY) {
		nyas__mesh_update(m, s);
	}
	im->res.flags = 0;
}

static void
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
}

static void
nyas__sync_gpu_fb(nyas_framebuffer fb, const nyas_fbattach *atta)
{
	NYAS_ASSERT(atta && "Null ptr arg.");
	r_fb *ifb = nyas_arr_at(framebuffer_pool, fb);
	if (ifb->res.id == NYAS_UNINIT) {
		glGenFramebuffers(1, (GLuint *)&(ifb->res.id));
	}
	NYAS_ASSERT(nypx__resource_check(ifb) && "Invalid internal resource.");
	glBindFramebuffer(GL_FRAMEBUFFER, ifb->res.id);
	if (atta->slot == NYAS_ATTACH_DEPTH) {
		// Change depth texture
		CHECK_HANDLE(tex, atta->tex);
		ifb->depth_tex = atta->tex;
		nyas__sync_gpu_tex(ifb->depth_tex);
		nyas__attach_to_fb(*atta);
	} else if (atta->slot == NYAS_ATTACH_COLOR) {
		// Change color texture
		CHECK_HANDLE(tex, atta->tex);
		ifb->color_tex = atta->tex;
		nyas__sync_gpu_tex(ifb->color_tex);
		nyas__attach_to_fb(*atta);
	} else {
		if (ifb->depth_tex != NYAS_INACTIVE) {
			nyas__sync_gpu_tex(ifb->depth_tex);
			nyas_fbattach a = { .tex = ifb->depth_tex,
				                .slot = NYAS_ATTACH_DEPTH,
				                .side = -1,
				                .level = 0 };
			nyas__attach_to_fb(a);
		}

		if (ifb->color_tex != NYAS_INACTIVE) {
			nyas__sync_gpu_tex(ifb->color_tex);
			nyas_fbattach a = { .tex = ifb->color_tex,
				                .slot = NYAS_ATTACH_COLOR,
				                .side = -1,
				                .level = 0 };
			nyas__attach_to_fb(a);
		}
	}
	ifb->res.flags = 0;
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
		tex_t *itx = nyas_arr_at(tex_pool, srcdata_tex[i]);
		if (itx->res.flags & NYAS_IRF_DIRTY) {
			nyas__sync_gpu_tex(srcdata_tex[i]);
		}
		*tmpdata_texid++ = itx->res.id;
	}

	// set tex cubemap data (src {handle} --> dst {internal id})
	nyas_tex *srcdata_cubemap = srcdata_tex + s->count[common].tex;
	for (int i = 0; i < s->count[common].cubemap; ++i) {
		tex_t *itx = nyas_arr_at(tex_pool, srcdata_cubemap[i]);
		if (itx->res.flags & NYAS_IRF_DIRTY) {
			nyas__sync_gpu_tex(srcdata_cubemap[i]);
		}
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
	r_mesh *imsh = nyas_arr_at(mesh_pool, mesh);
	CHECK_HANDLE(mesh, mesh);
	NYAS_ASSERT(imsh->elements && "Attempt to draw an uninitialized mesh");

	if (nyas__is_dirty(imsh)) {
		nyas__sync_gpu_mesh(mesh, data->draw.material.shader);
		if (imsh->res.id == NYAS_UNINIT) {
			nyas__create_mesh(mesh, data->draw.material.shader);
		} else if (imsh->res.flags & RF_DIRTY) {
			nyas__mesh_update(mesh, data->draw.material.shader);
		}
		imsh->res.flags = 0;
	}

	glBindVertexArray(imsh->res.id);
	shdr_t *s = nyas_arr_at(shader_pool, data->draw.material.shader);
	nyas__set_shader_data(s, data->draw.material.ptr, false);
	glDrawElements(GL_TRIANGLES, imsh->elements, ELEMENT_TYPE, 0);
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
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		nyas__set_viewport(-1, -1);
		return;
	}

	r_fb *ifb = nyas_arr_at(framebuffer_pool, d->fb);
	if ((int)d->attachment.slot != NYAS_IGNORE) {
		ifb->res.flags |= RF_DIRTY;
	}

	CHECK_HANDLE(framebuffer, d->fb);
	if (nyas__is_dirty(ifb)) {
		nyas__sync_gpu_fb(d->fb, &d->attachment);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, ifb->res.id);
	}
	if (d->vp_x > 0) {
		nyas__set_viewport(d->vp_x, d->vp_y);
	} else {
		nyas__set_viewport_from_fb(d->fb);
	}
}

#endif // NYAS_OPENGL
