#include "camera.h"
#include "core/io.h"
#include "core/manager.h"
#include "ecs/transform.h"
#include "internalresources.h"

static const float SENSIBILITY = 1.0f / 1000.0f;
static const float SPEED = 10.0f;
static const float SCROLL_SENSIBILITY = 1.0f;
static const struct vec3 UP = {.x = 0.0f, .y = 1.0f, .z = 0.0f};

void THE_CameraInit(THE_Camera *cam, float fov, float far, u32 width, u32 height, u8 is_light)
{
	cam->fov = fov;
	cam->far_value = far;
	cam->view_mat = smat4_identity();
	cam->proj_mat = smat4_perspective_fov(to_radians(fov), (float)width, (float)height, 0.01f, far);
	cam->fb = THE_CreateFramebuffer(width, height, !is_light, true);
}

struct mat4 THE_CameraStaticViewProjection(THE_Camera *cam)
{
	struct mat4 static_view = smat4(
		cam->view_mat.m11, cam->view_mat.m12, cam->view_mat.m13, 0.0f,
		cam->view_mat.m21, cam->view_mat.m22, cam->view_mat.m23, 0.0f,
		cam->view_mat.m31, cam->view_mat.m32, cam->view_mat.m33, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f );

	return smat4_multiply(cam->proj_mat, static_view);
}

struct vec3 THE_CameraPosition(THE_Camera *cam)
{
	struct mat4 inv = smat4_inverse(cam->view_mat);
	return svec3(inv.m14, inv.m24, inv.m34);
}

struct vec3 THE_CameraForward(THE_Camera *cam)
{
	return svec3(cam->view_mat.m31, cam->view_mat.m32, cam->view_mat.m33);
}

THE_Texture THE_CameraOutputColorTexture(THE_Camera *cam)
{
	return framebuffers[cam->fb].color_tex;
}

void THE_CameraMovementSystem(THE_Camera *cam)
{

	struct vec3 eye = THE_CameraPosition(cam);
	struct vec3 fwd = svec3_negative(svec3_normalize(THE_CameraForward(cam)));
	static float mouse_down_pos[2] = { 0.0, 0.0 };
	static float fov = 70.0f;
	float speed = SPEED * THE_DeltaTime();

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