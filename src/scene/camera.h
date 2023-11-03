#ifndef THE_RENDER_CAMERA_H
#define THE_RENDER_CAMERA_H

typedef struct THE_Camera {
	float view_mat[16];
	float proj_mat[16];
	float far_value;
	float fov;
} THE_Camera;

extern THE_Camera camera;

void THE_CameraInit(THE_Camera *cam, float fov,
	float far, float width, float height);

float *THE_CameraPosition(float *out_v3, THE_Camera *cam);
float *THE_CameraForward(float *out_v3, THE_Camera *cam);

/* Matrix without the translate values.
 * Result from projection * vec4(vec3(view));
 * Used for the skybox.
 */
float *THE_CameraStaticViewProjection(float *out_m4, THE_Camera *cam);
void THE_CameraMovementSystem(THE_Camera *cam, float deltatime);

#endif
