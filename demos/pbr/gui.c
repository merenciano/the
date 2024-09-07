#include "gui.h"
#include "pbr.h"

#include "core/io.h"
#include "core/scene.h"
#include "core/utils.h"
#include "render/pixels_internal.h"

#include <glad/glad.h>

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
nuklear_init(void)
{
	ctx = nk_glfw3_init(&glfw, the_io->internal_window, NK_GLFW3_INSTALL_CALLBACKS);
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&glfw, &atlas);
	nk_glfw3_font_stash_end(&glfw);
}

void
nuklear_draw(void)
{
	static char mesh_path[512] = { 0 };
	nk_glfw3_new_frame(&glfw);

	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	               NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {

		if (nk_tree_push(ctx, NK_TREE_TAB, "Entities", NK_MINIMIZED)) {
			for (int i = 0; i < entity_pool.count; ++i) {
				struct the_entity *e = entity_pool.buf->at + i;
				char entity_name[64];
				snprintf(entity_name, 64, "Entity %d", i);
				if (nk_tree_push_id(ctx, NK_TREE_TAB, entity_name, NK_MINIMIZED, i)) {
					// Transform
					if (nk_tree_push_id(ctx, NK_TREE_TAB, "Transform", NK_MINIMIZED, i)) {
						nk_layout_row_dynamic(ctx, 30, 4);
						nk_property_float(
						  ctx, "00", -100.0f, &e->transform[0], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "10", -100.0f, &e->transform[1], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "20", -100.0f, &e->transform[2], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "30", -100.0f, &e->transform[3], 100.0f, 0.5f, 0.1f);
						nk_layout_row_dynamic(ctx, 30, 4);
						nk_property_float(
						  ctx, "01", -100.0f, &e->transform[4], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "11", -100.0f, &e->transform[5], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "21", -100.0f, &e->transform[6], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "31", -100.0f, &e->transform[7], 100.0f, 0.5f, 0.1f);
						nk_layout_row_dynamic(ctx, 30, 4);
						nk_property_float(
						  ctx, "02", -100.0f, &e->transform[8], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "12", -100.0f, &e->transform[9], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "22", -100.0f, &e->transform[10], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "32", -100.0f, &e->transform[11], 100.0f, 0.5f, 0.1f);
						nk_layout_row_dynamic(ctx, 30, 4);
						nk_property_float(
						  ctx, "03", -100.0f, &e->transform[12], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "13", -100.0f, &e->transform[13], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "23", -100.0f, &e->transform[14], 100.0f, 0.5f, 0.1f);
						nk_property_float(
						  ctx, "33", -100.0f, &e->transform[15], 100.0f, 0.5f, 0.1f);
						nk_tree_pop(ctx);
					}

					// Mesh
					if (nk_tree_push_id(ctx, NK_TREE_TAB, "Mesh", NK_MINIMIZED, i)) {
						nk_layout_row_dynamic(ctx, 30, 1);
						nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, mesh_path, 512, NULL);
						nk_layout_row_dynamic(ctx, 30, 5);
						if (nk_button_label(ctx, "Load")) {
							the_mesh mesh = the_mesh_load_file(mesh_path);
							e->mesh = mesh;
							memset(mesh_path, 0, 512);
						}
						if (nk_button_label(ctx, "Reload")) {
							the_mesh_reload_file(e->mesh, mesh_path);
							memset(mesh_path, 0, 512);
						}
						if (nk_button_label(ctx, "Cube")) {
							// tut_mesh_set_geometry(e->mesh, THE_CUBE);
							e->mesh = THE_UTILS_CUBE;
						}
						if (nk_button_label(ctx, "Sphere")) {
							// tut_mesh_set_geometry(e->mesh, THE_SPHERE);
							e->mesh = THE_UTILS_SPHERE;
						}
						if (nk_button_label(ctx, "Quad")) {
							// tut_mesh_set_geometry(e->mesh, THE_QUAD);
							e->mesh = THE_UTILS_QUAD;
						}
						nk_tree_pop(ctx);
					}

					// Material
					if (nk_tree_push_hashed(ctx, NK_TREE_TAB, "Material", NK_MINIMIZED,
					                        NK_FILE_LINE, nk_strlen(NK_FILE_LINE), i)) {
						struct pbr_desc_unit *unit = e->mat.ptr;
						struct pbr_desc_scene *scene =
						  (shader_pool.buf->at + e->mat.shader)->common;
						struct the_texture_internal *tex = &tex_pool.buf->at[*the_mat_tex(e->mat)];

						if (nk_tree_push_hashed(ctx, NK_TREE_TAB, "Albedo", NK_MINIMIZED,
						                        NK_FILE_LINE, nk_strlen(NK_FILE_LINE), i)) {
							struct the_texture_internal *tex =
							  &tex_pool.buf->at[*the_mat_tex(e->mat)];
							nk_layout_row_static(ctx, 256, 256, 1);
							nk_image(ctx, nk_image_id(tex->res.id));
							nk_layout_row_dynamic(ctx, 30, 2);
							nk_label(ctx, "Albedo texture intensity: ", NK_TEXT_LEFT);
							unit->use_albedo_map =
							  nk_slide_float(ctx, 0.0f, unit->use_albedo_map, 1.0f, 0.01f);
							nk_label(ctx, "Color: ", NK_TEXT_LEFT);
							*(struct nk_colorf *)unit->color =
							  nk_color_picker(ctx, *(struct nk_colorf *)unit->color, NK_RGB);
							nk_tree_pop(ctx);
						}

						if (nk_tree_push_hashed(ctx, NK_TREE_TAB, "Normal map", NK_MINIMIZED,
						                        NK_FILE_LINE, nk_strlen(NK_FILE_LINE), i)) {
							nk_layout_row_static(ctx, 256, 256, 1);
							nk_image(ctx, nk_image_id(tex[1].res.id));
							nk_layout_row_dynamic(ctx, 30, 2);
							nk_label(ctx, "Normal map intensity: ", NK_TEXT_LEFT);
							unit->normal_map_intensity =
							  nk_slide_float(ctx, 0.0f, unit->normal_map_intensity, 1.0f, 0.01f);
							nk_tree_pop(ctx);
						}

						if (nk_tree_push_hashed(ctx, NK_TREE_TAB, "Material maps", NK_MINIMIZED,
						                        NK_FILE_LINE, nk_strlen(NK_FILE_LINE), i)) {
							nk_layout_row_static(ctx, 124, 124, 2);
							nk_image(ctx, nk_image_id(tex[2].res.id));
							nk_image(ctx, nk_image_id(tex[3].res.id));
							nk_layout_row_dynamic(ctx, 30, 2);
							nk_label(ctx, "Material texture intensity: ", NK_TEXT_LEFT);
							unit->use_pbr_maps =
							  nk_slide_float(ctx, 0.0f, unit->use_pbr_maps, 1.0f, 0.01f);
							nk_label(ctx, "Roughness: ", NK_TEXT_LEFT);
							unit->roughness =
							  nk_slide_float(ctx, 0.0f, unit->roughness, 1.0f, 0.01f);
							nk_label(ctx, "Metalness: ", NK_TEXT_LEFT);
							unit->metallic =
							  nk_slide_float(ctx, 0.0f, unit->metallic, 1.0f, 0.01f);
							nk_tree_pop(ctx);
						}

						nk_layout_row_dynamic(ctx, 30, 3);
						nk_label(ctx, "Tiling: ", NK_TEXT_LEFT);
						nk_property_float(ctx, "#X", 0.0f, &unit->tiling_x, 100.0f, 1.0f, 0.01f);
						nk_property_float(ctx, "#Y", 0.0f, &unit->tiling_y, 100.0f, 1.0f, 0.01f);

						nk_layout_row_dynamic(ctx, 30, 1);
						nk_label(ctx, "Light direction: ", NK_TEXT_LEFT);
						nk_layout_row_dynamic(ctx, 30, 3);
						nk_property_float(
						  ctx, "#X:", -100.0f, scene->sunlight, 100.0f, 0.01f, 0.01f);
						nk_property_float(
						  ctx, "#Y:", -100.0f, scene->sunlight + 1, 100.0f, 0.01f, 0.01f);
						nk_property_float(
						  ctx, "#Z:", -100.0f, scene->sunlight + 2, 100.0f, 0.01f, 0.01f);
						nk_layout_row_dynamic(ctx, 30, 2);
						nk_label(ctx, "Light intensity: ", NK_TEXT_LEFT);
						scene->sunlight[3] =
						  nk_slide_float(ctx, 0.0f, scene->sunlight[3], 10.0f, 0.05f);

						nk_tree_pop(ctx);
					}
					nk_tree_pop(ctx);
				}
			}
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Resources", NK_MINIMIZED)) {
			nk_labelf(ctx, NK_TEXT_LEFT, "Textures: %d / %ld (%lu Bytes)", tex_pool.count,
			          tex_pool.buf->count, sizeof(struct the_texture_internal));
			nk_labelf(ctx, NK_TEXT_LEFT, "Meshes: %d / %ld (%lu Bytes)", mesh_pool.count,
			          mesh_pool.buf->count, sizeof(struct the_mesh_internal));
			nk_labelf(
			  ctx, NK_TEXT_LEFT, "Framebuffers: %d / %ld (%lu Bytes)", framebuffer_pool.count,
			  framebuffer_pool.buf->count, sizeof(struct the_framebuffer_internal));
			nk_labelf(ctx, NK_TEXT_LEFT, "Shaders: %d / %ld (%lu Bytes)", shader_pool.count,
			          shader_pool.buf->count, sizeof(struct the_shader_internal));
			if (nk_tree_push(ctx, NK_TREE_TAB, "Shaders", NK_MINIMIZED)) {
				for (int i = 0; i < shader_pool.count; ++i) {
					nk_label(ctx, shader_pool.buf->at[i].name, NK_TEXT_LEFT);
					if (nk_button_label(ctx, "Reload")) {
						the_shader_reload(i);
					}
				}
				nk_tree_pop(ctx);
			}
			nk_tree_pop(ctx);
		}
	}
	nk_end(ctx);

	nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}
