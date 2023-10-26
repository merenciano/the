#include "camera.h"
#include "core/io.h"

#include <mathc.h>

typedef struct THE_Camera THE_Camera;

static const float SENSIBILITY = 1.0f / 1000.0f;
static const float SPEED = 10.0f;
static const float SCROLL_SENSIBILITY = 1.0f;
static float UP[3] = {0.0f, 1.0f, 0.0f};

void THE_CameraInit(THE_Camera *cam, float fov, float far, uint32_t width, uint32_t height)
{
	cam->fov = fov;
	cam->far_value = far;
	mat4_identity(cam->view_mat);
	mat4_perspective_fov(cam->proj_mat, to_radians(fov), (float)width, (float)height, 0.01f, far);
}

float *THE_CameraStaticViewProjection(float *out_m4, THE_Camera *cam)
{
	mat4_assign(out_m4, cam->view_mat);
	out_m4[3] = 0.0f;
	out_m4[7] = 0.0f;
	out_m4[11] = 0.0f;
	out_m4[12] = 0.0f;
	out_m4[13] = 0.0f;
	out_m4[14] = 0.0f;
	out_m4[15] = 0.0f;
	return mat4_multiply(out_m4 ,cam->proj_mat, out_m4);
}

float *THE_CameraPosition(float *out_v3, THE_Camera *cam)
{
	float inv[16];
	mat4_inverse(inv, cam->view_mat);
	out_v3[0] = inv[12];
	out_v3[1] = inv[13];
	out_v3[2] = inv[14];
	return out_v3;
}

float *THE_CameraForward(float *out_v3, THE_Camera *cam)
{
	out_v3[0] = cam->view_mat[2];
	out_v3[1] = cam->view_mat[6];
	out_v3[2] = cam->view_mat[10];
	return out_v3;
}

void THE_CameraMovementSystem(THE_Camera *cam, float deltatime)
{
	float eye[3];
	THE_CameraPosition(eye, cam);
	float fwd[3];
	THE_CameraForward(fwd, cam);
	vec3_negative(fwd, vec3_normalize(fwd, fwd));
	static float mouse_down_pos[2] = { 0.0f, 0.0f };
	float speed = SPEED * deltatime;

	// Rotation
	if (THE_InputIsButtonDown(THE_MOUSE_RIGHT)) {
		mouse_down_pos[0] = THE_InputGetMouseX();
		mouse_down_pos[1] = THE_InputGetMouseY();
	}

	float tmp_vec[3];
	if (THE_InputIsButtonPressed(THE_MOUSE_RIGHT))
	{
		float mouse_offset[2] = {
			THE_InputGetMouseX() - mouse_down_pos[0],
			mouse_down_pos[1] - THE_InputGetMouseY()
		}; // Y axis inverted

		mouse_offset[0] *= SENSIBILITY;
		mouse_offset[1] *= SENSIBILITY;

		vec3_add(fwd, fwd, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd), -mouse_offset[0]));
		vec3_add(fwd, fwd, vec3_multiply_f(tmp_vec, UP, mouse_offset[1]));

		mouse_down_pos[0] = THE_InputGetMouseX();
		mouse_down_pos[1] = THE_InputGetMouseY();
	}

	// Position
	if (THE_InputIsButtonPressed(THE_KEY_UP))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_DOWN))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, -speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_LEFT))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd), speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_RIGHT))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, UP, fwd), -speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_1))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, UP, speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_4))
	{
		vec3_add(eye, eye, vec3_multiply_f(tmp_vec, UP, -speed));
	}

	mat4_look_at(cam->view_mat, eye, vec3_add(tmp_vec, eye, fwd), UP);

	// Zoom
	if (THE_InputGetScroll() != 0.0f)
	{
		cam->fov -= THE_InputGetScroll() * SCROLL_SENSIBILITY;
		cam->fov = clampf(cam->fov, 1.0f, 120.0f);
		mat4_perspective(cam->proj_mat, to_radians(cam->fov), (float)THE_WindowGetWidth() / (float)THE_WindowGetHeight(), 0.1f, cam->far_value);
	}
}