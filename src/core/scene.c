#include "io.h"
#include "scene.h"
#include <mathc.h>

#define VEC3_UP ((float[3]){ 0.0f, 1.0f, 0.0f })

struct nyas_entity *entities;
struct nyas_cam camera;

static inline struct nyas_vec3
nyas_camera_fwd(struct nyas_cam *cam)
{
	return (struct nyas_vec3){ cam->view[2], cam->view[6], cam->view[10] };
}

void
nyas_camera_init(struct nyas_cam *cam, struct nyas_vec3 pos, struct nyas_vec3 target)
{
	mat4_look_at(cam->view, &pos, &target, VEC3_UP);
	mat4_perspective_fov(
	  cam->proj, to_radians(cam->fov), nyas_io->window_size.x, nyas_io->window_size.y, 0.01f,
	  cam->far);
}

void
nyas_camera_init_default(struct nyas_cam *cam)
{
	cam->fov = 70.0f;
	cam->far = 300.0f;
	nyas_camera_init(
	  cam, (struct nyas_vec3){ 0.0f, 2.0f, 2.0f }, (struct nyas_vec3){ 0.0f, 0.0f, -1.0f });
}

struct nyas_vec3
nyas_camera_eye(struct nyas_cam *cam)
{
	nyas_mat4 inv;
	mat4_inverse(inv, cam->view);
	return (struct nyas_vec3){ inv[12], inv[13], inv[14] };
}

float *
nyas_camera_static_vp(struct nyas_cam *cam, nyas_mat4 out)
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
nyas_camera_control(struct nyas_cam *cam, struct nyas_control_config cfg)
{
	struct nyas_vec3 eye = nyas_camera_eye(cam);
	struct nyas_vec3 fwd = nyas_camera_fwd(cam);
	vec3_negative(&fwd, vec3_normalize(&fwd, &fwd));
	static struct nyas_vec2 mouse_down_pos = { 0.0f, 0.0f };
	float speed = cfg.speed * cfg.deltatime;

	// Rotation
	if (nyas_io->mouse_button[NYAS_MOUSE_RIGHT] == NYAS_KEYSTATE_DOWN) {
		mouse_down_pos = nyas_io->mouse_pos;
	}

	float tmp_vec[3];
	if (nyas_io->mouse_button[NYAS_MOUSE_RIGHT] == NYAS_KEYSTATE_PRESSED) {
		struct nyas_vec2 curr_pos = nyas_io->mouse_pos;
		struct nyas_vec2 offset = {
			(curr_pos.x - mouse_down_pos.x) * cfg.sensitivity,
			(mouse_down_pos.y - curr_pos.y) * cfg.sensitivity
		};

		vec3_add(&fwd, &fwd,
		         vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, &fwd), -offset.x));
		vec3_add(&fwd, &fwd, vec3_multiply_f(tmp_vec, VEC3_UP, offset.y));

		mouse_down_pos = curr_pos;
	}

	// Position
	if (nyas_io->keys[NYAS_KEY_W] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, &fwd, speed));
	}

	if (nyas_io->keys[NYAS_KEY_S] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, &fwd, -speed));
	}

	if (nyas_io->keys[NYAS_KEY_A] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, &fwd), speed));
	}

	if (nyas_io->keys[NYAS_KEY_D] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, VEC3_UP, &fwd), -speed));
	}

	if (nyas_io->keys[NYAS_KEY_SPACE] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, VEC3_UP, speed));
	}

	if (nyas_io->keys[NYAS_KEY_LEFT_SHIFT] == NYAS_KEYSTATE_PRESSED) {
		vec3_add(&eye, &eye, vec3_multiply_f(tmp_vec, VEC3_UP, -speed));
	}

	mat4_look_at(cam->view, &eye, vec3_add(tmp_vec, &eye, &fwd), VEC3_UP);

	// Zoom
	if (nyas_io->mouse_scroll.y != 0.0f) {
		cam->fov -= nyas_io->mouse_scroll.y * cfg.scroll_sensitivity;
		cam->fov = clampf(cam->fov, 1.0f, 120.0f);
		mat4_perspective(
		  cam->proj, to_radians(cam->fov),
		  (float)nyas_io->window_size.x / (float)nyas_io->window_size.y, 0.1f, cam->far);
	}
}
