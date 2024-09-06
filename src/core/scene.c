#include "io.h"
#include "scene.h"
#include <mathc.h>
#include <stdlib.h>

#define VEC3_UP ((float[3]){ 0.0f, 1.0f, 0.0f })

THE_IMPL_ARR(theent);
THE_IMPL_POOL(theent);

struct thepool_theent entity_pool = {.buf = NULL, .next = 0, .count = 0};
struct the_cam camera;

static inline struct the_vec3
the_camera_fwd(struct the_cam *cam)
{
	return (struct the_vec3){ cam->view[2], cam->view[6], cam->view[10] };
}

void
the_camera_init(struct the_cam *cam, struct the_vec3 pos, struct the_vec3 target)
{
	mat4_look_at(cam->view, (float*)&pos, (float*)&target, VEC3_UP);
	mat4_perspective_fov(
	  cam->proj, to_radians(cam->fov), the_io->window_size.x, the_io->window_size.y, 0.01f,
	  cam->far);
}

void
the_camera_init_default(struct the_cam *cam)
{
	cam->fov = 70.0f;
	cam->far = 300.0f;
	the_camera_init(
	  cam, (struct the_vec3){ 0.0f, 2.0f, 2.0f }, (struct the_vec3){ 0.0f, 0.0f, -1.0f });
}

struct the_vec3
the_camera_eye(struct the_cam *cam)
{
	the_mat4 inv;
	mat4_inverse(inv, cam->view);
	return (struct the_vec3){ inv[12], inv[13], inv[14] };
}

float *
the_camera_static_vp(struct the_cam *cam, the_mat4 out)
{
	mat4_assign(out, cam->view);
	out[3] = 0.0f;
	out[7] = 0.0f;
	out[11] = 0.0f;
	out[12] = 0.0f;
	out[13] = 0.0f;
	out[14] = 0.0f;
	out[15] = 0.0f;
	return mat4_multiply(out, cam->proj, out);
}

void
the_camera_control(struct the_cam *cam, struct the_control_config cfg)
{
	struct the_vec3 eye = the_camera_eye(cam);
	struct the_vec3 fwd = the_camera_fwd(cam);
	vec3_negative((float*)&fwd, vec3_normalize((float*)&fwd, (float*)&fwd));
	static struct the_vec2 mouse_down_pos = { 0.0f, 0.0f };
	float speed = cfg.speed * cfg.deltatime;

	// Rotation
	if (the_io->mouse_button[THE_MOUSE_RIGHT] == THE_KEYSTATE_DOWN) {
		mouse_down_pos = the_io->mouse_pos;
	}

	float tmp_vec[3];
	if (the_io->mouse_button[THE_MOUSE_RIGHT] == THE_KEYSTATE_PRESSED) {
		struct the_vec2 curr_pos = the_io->mouse_pos;
		struct the_vec2 offset = {
			(curr_pos.x - mouse_down_pos.x) * cfg.sensitivity,
			(mouse_down_pos.y - curr_pos.y) * cfg.sensitivity
		};

		vec3_add((float*)&fwd, (float*)&fwd,
		         vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, (float*)&fwd), -offset.x));
		vec3_add((float*)&fwd, (float*)&fwd, vec3_multiply_f(tmp_vec, VEC3_UP, offset.y));

		mouse_down_pos = curr_pos;
	}

	// Position
	if (the_io->keys[THE_KEY_W] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, (float*)&fwd, speed));
	}

	if (the_io->keys[THE_KEY_S] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, (float*)&fwd, -speed));
	}

	if (the_io->keys[THE_KEY_A] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, (float*)&fwd), speed));
	}

	if (the_io->keys[THE_KEY_D] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, (float*)&fwd), -speed));
	}

	if (the_io->keys[THE_KEY_SPACE] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, VEC3_UP, speed));
	}

	if (the_io->keys[THE_KEY_LEFT_SHIFT] == THE_KEYSTATE_PRESSED) {
		vec3_add((float*)&eye, (float*)&eye, vec3_multiply_f(tmp_vec, VEC3_UP, -speed));
	}

	mat4_look_at(cam->view, (float*)&eye, vec3_add(tmp_vec, (float*)&eye, (float*)&fwd), VEC3_UP);

	// Zoom
	if (the_io->mouse_scroll.y != 0.0f) {
		cam->fov -= the_io->mouse_scroll.y * cfg.scroll_sensitivity;
		cam->fov = clampf(cam->fov, 1.0f, 120.0f);
		mat4_perspective(
		  cam->proj, to_radians(cam->fov),
		  (float)the_io->window_size.x / (float)the_io->window_size.y, 0.1f, cam->far);
	}
}
