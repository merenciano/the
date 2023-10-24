#include "camera.h"
#include "core/io.h"
#include "core/manager.h"
#include "internalresources.h"

#include <mathc.h>

typedef struct THE_Camera THE_Camera;

static const float SENSIBILITY = 1.0f / 1000.0f;
static const float SPEED = 10.0f;
static const float SCROLL_SENSIBILITY = 1.0f;
static const struct vec3 UP = {.x = 0.0f, .y = 1.0f, .z = 0.0f};

void THE_CameraInit(THE_Camera *cam, float fov, float far, uint32_t width, uint32_t height, bool is_light)
{
	cam->fov = fov;
	cam->far_value = far;
	mat4_identity(cam->view_mat);
	mat4_perspective_fov(cam->proj_mat, to_radians(fov), (float)width, (float)height, 0.01f, far);
}

float *THE_CameraStaticViewProjection(THE_Camera *cam)
{
	float sv[16];
	mat4_assign(sv, cam->view_mat);
	sv[3] = 0.0f;
	sv[7] = 0.0f;
	sv[11] = 0.0f;
	sv[12] = 0.0f;
	sv[13] = 0.0f;
	sv[14] = 0.0f;
	sv[15] = 0.0f;
	return mat4_multiply(sv ,cam->proj_mat, sv);
}

float *THE_CameraPosition(THE_Camera *cam)
{
	float inv[16];
	mat4_inverse(inv, cam->view_mat);
	return inv + 12;
}

float *THE_CameraForward(THE_Camera *cam)
{
	float fwd[3] = {
		cam->view_mat[2],
		cam->view_mat[6],
		cam->view_mat[10]
	};
	return fwd;
}

void THE_CameraMovementSystem(THE_Camera *cam, float deltatime)
{
	struct vec3 eye = THE_CameraPosition(cam);
	float fwd[3];
	vec3_negative(fwd, vec3_normalize(fwd, THE_CameraForward(cam)));
	static float mouse_down_pos[2] = { 0.0, 0.0 };
	static float fov = 70.0f;
	float speed = SPEED * deltatime;

	// Rotation
	if (THE_InputIsButtonDown(THE_MOUSE_RIGHT)) {
		mouse_down_pos[0] = THE_InputGetMouseX();
		mouse_down_pos[1] = THE_InputGetMouseY();
	}

	if (THE_InputIsButtonPressed(THE_MOUSE_RIGHT))
	{
		float mouse_offset[2] = {
			THE_InputGetMouseX() - mouse_down_pos[0],
			mouse_down_pos[1] - THE_InputGetMouseY()
		}; // Y axis inverted

		mouse_offset[0] *= SENSIBILITY;
		mouse_offset[1] *= SENSIBILITY;

		fwd = svec3_add(fwd, svec3_multiply_f(svec3_cross(UP, fwd), -mouse_offset[0]));
		fwd = svec3_add(fwd, svec3_multiply_f(UP, mouse_offset[1]));

		mouse_down_pos[0] = THE_InputGetMouseX();
		mouse_down_pos[1] = THE_InputGetMouseY();
	}

	// Position
	if (THE_InputIsButtonPressed(THE_KEY_UP))
	{
		eye = svec3_add(eye, svec3_multiply_f(fwd, speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_DOWN))
	{
		eye = svec3_add(eye, svec3_multiply_f(fwd, -speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_LEFT))
	{
		eye = svec3_add(eye, svec3_multiply_f(svec3_cross(UP, fwd), speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_RIGHT))
	{
		eye = svec3_add(eye, svec3_multiply_f(svec3_cross(UP, fwd), -speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_1))
	{
		eye = svec3_add(eye, svec3_multiply_f(UP, speed));
	}

	if (THE_InputIsButtonPressed(THE_KEY_4))
	{
		eye = svec3_add(eye, svec3_multiply_f(UP, -speed));
	}

	cam->view_mat = smat4_look_at(eye, svec3_add(eye, fwd), UP);

	// Zoom
	if (THE_InputGetScroll() != 0.0f)
	{
		fov -= THE_InputGetScroll() * SCROLL_SENSIBILITY;
		fov = clampf(fov, 1.0f, 120.0f);
		cam->proj_mat = smat4_perspective(to_radians(fov), (float)THE_WindowGetWidth() / (float)THE_WindowGetHeight(), 0.1f, camera.far_value);
	}
}