#include "imgui.h"

#include "core/io.h"
#include "render/pixels_internal.h"

#include <glad/glad.h>
#include <stdio.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include <nuklear_glfw_gl3.h>

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

struct nk_glfw glfw = { 0 };
struct nk_context *ctx;

void
nyas_imgui_init(void)
{
	ctx = nk_glfw3_init(&glfw, internal_window, NK_GLFW3_INSTALL_CALLBACKS);
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&glfw, &atlas);
	nk_glfw3_font_stash_end(&glfw);
}

void
nyas_imgui_draw(void)
{
	struct nk_colorf bg = { 1.0f, 0.0f, 0.0f, 1.0f };
	nk_glfw3_new_frame(&glfw);

	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	               NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {

		enum { EASY, HARD };
		static int op = EASY;
		static int property = 20;
		nk_layout_row_static(ctx, 30, 80, 1);
		if (nk_button_label(ctx, "button"))
			fprintf(stdout, "button pressed\n");

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_option_label(ctx, "easy", op == EASY))
			op = EASY;
		if (nk_option_label(ctx, "hard", op == HARD))
			op = HARD;

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_color(ctx, nk_rgb_cf(bg),
		                         nk_vec2(nk_widget_width(ctx), 400))) {
			nk_layout_row_dynamic(ctx, 120, 1);
			bg = nk_color_picker(ctx, bg, NK_RGBA);
			nk_layout_row_dynamic(ctx, 25, 1);
			bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
			bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
			bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
			bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
			nk_combo_end(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Resources", NK_MINIMIZED)) {
			nk_labelf(ctx, NK_TEXT_LEFT, "Textures: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(tex_pool), nyas_arr_cap(tex_pool),
			          sizeof(r_tex));
			nk_labelf(ctx, NK_TEXT_LEFT, "Meshes: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(mesh_pool), nyas_arr_cap(mesh_pool),
			          sizeof(r_mesh));
			nk_labelf(ctx, NK_TEXT_LEFT, "Framebuffers: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(framebuffer_pool),
			          nyas_arr_cap(framebuffer_pool), sizeof(r_fb));
			nk_labelf(ctx, NK_TEXT_LEFT, "Shaders: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(shader_pool), nyas_arr_cap(shader_pool),
			          sizeof(r_shader));
			if (nk_tree_push(ctx, NK_TREE_TAB, "Shaders", NK_MINIMIZED)) {
				for (int i = 0; i < nyas_arr_len(shader_pool); ++i) {
					nk_label(ctx, ((r_shader*)shader_pool)[i].shader_name, NK_TEXT_LEFT);
					if (nk_button_label(ctx, "Reload")) {
						((r_shader*)shader_pool)[i].res.flags |= RF_DIRTY;
					}
				}
				nk_tree_pop(ctx);
			}
			nk_tree_pop(ctx);
		}
	}
	nk_end(ctx);

	nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER,
	                MAX_ELEMENT_BUFFER);
}
