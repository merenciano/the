#include "imgui.h"

#include "core/io.h"
#include "render/pixels_internal.h"
#include "render/pixels_extra.h"
#include "scene/entity.h"

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
	static char mesh_path[512] = { 0 };
	nk_glfw3_new_frame(&glfw);

	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	               NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {
		for (int i = 0; i < nyas_entity_count(); ++i) {
			nyas_entity *e = nyas_entities() + i;
			// Mesh
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, mesh_path, 512,
			                               NULL);
			nk_layout_row_dynamic(ctx, 30, 2);
			if (nk_button_label(ctx, "Load OBJ")) {
				nyas_mesh_load_obj(e->mesh, mesh_path);
			}
			if (nk_button_label(ctx, "Load MSH")) {
				nyas_mesh_load_msh(e->mesh, mesh_path);
			}

			// Material
			nyas_pbr_desc_unit *unit = e->mat.ptr;
			nyas_pbr_desc_scene *scene = ((struct nyas_internal_shader*)shader_pool)->common;

			nk_label(ctx, "Albedo texture intensity: ", NK_TEXT_LEFT);
			unit->use_albedo_map = nk_slide_float(ctx, 0.0f, unit->use_albedo_map, 1.0f, 0.01f);
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Color: ", NK_TEXT_LEFT);
			*(struct nk_colorf*)unit->color = nk_color_picker(ctx, *(struct nk_colorf*)unit->color, NK_RGB);
			//unit->use_albedo_map = nk_slide_float(ctx, 0.0f, unit->use_albedo_map, 1.0f, 0.01f);
			nk_layout_row_dynamic(ctx, 30, 2);
			nk_label(ctx, "Normal map intensity: ", NK_TEXT_LEFT);
			unit->normal_map_intensity = nk_slide_float(ctx, 0.0f, unit->normal_map_intensity, 1.0f, 0.01f);
			nk_label(ctx, "Material texture intensity: ", NK_TEXT_LEFT);
			unit->use_pbr_maps = nk_slide_float(ctx, 0.0f, unit->use_pbr_maps, 1.0f, 0.01f);
			nk_label(ctx, "Roughness: ", NK_TEXT_LEFT);
			unit->roughness = nk_slide_float(ctx, 0.0f, unit->roughness, 1.0f, 0.01f);
			nk_label(ctx, "Metalness: ", NK_TEXT_LEFT);
			unit->metallic = nk_slide_float(ctx, 0.0f, unit->metallic, 1.0f, 0.01f);
			nk_layout_row_dynamic(ctx, 30, 3);
			nk_label(ctx, "Tiling: ", NK_TEXT_LEFT);
			unit->tiling_x = nk_slide_float(ctx, 0.0f, unit->tiling_x, 100.0f, 0.01f);
			unit->tiling_y = nk_slide_float(ctx, 0.0f, unit->tiling_y, 100.0f, 0.01f);
			
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Light direction: ", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, 30, 3);
			nk_property_float(ctx, "#X:", -100.0f, scene->sunlight, 100.0f, 0.01f, 0.01f);
			nk_property_float(ctx, "#Y:", -100.0f, scene->sunlight + 1, 100.0f, 0.01f, 0.01f);
			nk_property_float(ctx, "#Z:", -100.0f, scene->sunlight + 2, 100.0f, 0.01f, 0.01f);
			nk_layout_row_dynamic(ctx, 30, 2);
			nk_label(ctx, "Light intensity: ", NK_TEXT_LEFT);
			scene->sunlight[3] = nk_slide_float(ctx, 0.0f, scene->sunlight[3], 10.0f, 0.05f);

			struct nyas_internal_texture* scene_tex = nyas_arr_at(tex_pool, nyas_shader_tex(0)[1]);
			nk_layout_row_static(ctx, 250, 250, 1);
			//nk_image(ctx, nk_image_id(scene_tex->res.id));
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Resources", NK_MINIMIZED)) {
			nk_labelf(ctx, NK_TEXT_LEFT, "Textures: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(tex_pool), nyas_arr_cap(tex_pool),
			          sizeof(struct nyas_internal_texture));
			nk_labelf(ctx, NK_TEXT_LEFT, "Meshes: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(mesh_pool), nyas_arr_cap(mesh_pool),
			          sizeof(r_mesh));
			nk_labelf(ctx, NK_TEXT_LEFT, "Framebuffers: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(framebuffer_pool),
			          nyas_arr_cap(framebuffer_pool), sizeof(r_fb));
			nk_labelf(ctx, NK_TEXT_LEFT, "Shaders: %lu / %lu (%lu Bytes)",
			          nyas_arr_len(shader_pool), nyas_arr_cap(shader_pool),
			          sizeof(struct nyas_internal_shader));
			if (nk_tree_push(ctx, NK_TREE_TAB, "Shaders", NK_MINIMIZED)) {
				for (size_t i = 0; i < nyas_arr_len(shader_pool); ++i) {
					nk_label(
					  ctx,
					  ((struct nyas_internal_shader *)shader_pool)[i].name,
					  NK_TEXT_LEFT);
					if (nk_button_label(ctx, "Reload")) {
						((struct nyas_internal_shader *)shader_pool)[i]
						  .res.flags |= NYAS_IRF_DIRTY;
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
