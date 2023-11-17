#include "camera.h"
#include "core/io.h"

#include <mathc.h>

nyas_cam camera;

static const float SENSIBILITY = 1.0f / 1000.0f;
static const float SPEED = 100.0f;
static const float SCROLL_SENSIBILITY = 1.0f;
static float UP[3] = { 0.0f, 1.0f, 0.0f };

void
nyas_camera_init(nyas_cam *cam,
               float fov,
               float far,
               float width,
               float height)
{
	cam->fov = fov;
	cam->far = far;
	float pos[] = { 0.0f, 2.0f, 2.0f };
	float target[] = { 0.0f, 0.0f, -1.0f };
	mat4_look_at(cam->view, pos, target, UP);
	mat4_perspective_fov(cam->proj, to_radians(fov), width, height, 0.01f,
	                     far);
}

float *
nyas_camera_static_vp(float *out_m4, nyas_cam *cam)
{
	mat4_assign(out_m4, cam->view);
	out_m4[3] = 0.0f;
	out_m4[7] = 0.0f;
	out_m4[11] = 0.0f;
	out_m4[12] = 0.0f;
	out_m4[13] = 0.0f;
	out_m4[14] = 0.0f;
	out_m4[15] = 0.0f;
	return mat4_multiply(out_m4, cam->proj, out_m4);
}

float *
nyas_camera_pos(float *out_v3, nyas_cam *cam)
{
	float inv[16];
	mat4_inverse(inv, cam->view);
	out_v3[0] = inv[12];
	out_v3[1] = inv[13];
	out_v3[2] = inv[14];
	return out_v3;
}

float *
nyas_camera_fwd(float *out_v3, nyas_cam *cam)
{
	out_v3[0] = cam->view[2];
	out_v3[1] = cam->view[6];
	out_v3[2] = cam->view[10];
	return out_v3;
}
void
nyas_camera_control(nyas_cam *cam, float deltatime)
{
	float eye[3];
	nyas_camera_pos(eye, cam);
	float fwd[3];
	nyas_camera_fwd(fwd, cam);
	vec3_negative(fwd, vec3_normalize(fwd, fwd));
	static float mouse_down_pos[2] = { 0.0f, 0.0f };
	float speed = SPEED * deltatime;

	// Rotation
	if (nyas_input_down(NYAS_MOUSE_RIGHT)) {
		mouse_down_pos[0] = nyas_input_mouse_x();
		mouse_down_pos[1] = nyas_input_mouse_y();
	}

	float tmp_vec[3];
	if (nyas_input_pressed(NYAS_MOUSE_RIGHT)) {
		float mouse_offset[2] = { nyas_input_mouse_x() - mouse_down_pos[0],
			                      mouse_down_pos[1] -
			                        nyas_input_mouse_y() }; // Y axis inverted

		mouse_offset[0] *= SENSIBILITY;
		mouse_offset[1] *= SENSIBILITY;

		vec3_add(fwd, fwd,
		         vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd),
		                         -mouse_offset[0]));
		vec3_add(fwd, fwd, vec3_multiply_f(tmp_vec, UP, mouse_offset[1]));

		mouse_down_pos[0] = nyas_input_mouse_x();
		mouse_down_pos[1] = nyas_input_mouse_y();
	}

	// Position
	if (nyas_input_pressed(NYAS_KEY_UP)) {
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, speed));
	}

	if (nyas_input_pressed(NYAS_KEY_DOWN)) {
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, -speed));
	}

	if (nyas_input_pressed(NYAS_KEY_LEFT)) {
		vec3_add(eye, eye,
		         vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd),
		                         speed));
	}

	if (nyas_input_pressed(NYAS_KEY_RIGHT)) {
		vec3_add(eye, eye,
		         vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd),
		                         -speed));
	}

	if (nyas_input_pressed(NYAS_KEY_1)) {
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, UP, speed));
	}

	if (nyas_input_pressed(NYAS_KEY_4)) {
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, UP, -speed));
	}

	mat4_look_at(cam->view, eye, vec3_add(tmp_vec, eye, fwd), UP);

	// Zoom
	if (nyas_input_scroll() != 0.0f) {
		cam->fov -= nyas_input_scroll() * SCROLL_SENSIBILITY;
		cam->fov = clampf(cam->fov, 1.0f, 120.0f);
		mat4_perspective(cam->proj, to_radians(cam->fov),
		                 (float)nyas_window_width() /
		                   (float)nyas_window_height(),
		                 0.1f, cam->far);
	}
}
