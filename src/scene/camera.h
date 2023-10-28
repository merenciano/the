#ifndef THE_RENDER_CAMERA_H
#define THE_RENDER_CAMERA_H

#include <stdint.h>
#include <stdbool.h>


struct THE_Camera {
	float view_mat[16];
	float proj_mat[16];
	float far_value;
	float fov;
};

extern struct THE_Camera camera;

void THE_CameraInit(struct THE_Camera *cam, float fov,
	float far, uint32_t width, uint32_t height);

float *THE_CameraPosition(float *out_v3, struct THE_Camera *cam);
float *THE_CameraForward(float *out_v3, struct THE_Camera *cam);

/* Matrix without the translate values.
 * Result from projection * vec4(vec3(view));
 * Used for the skybox.
 */
float *THE_CameraStaticViewProjection(float *out_m4, struct THE_Camera *cam);
void THE_CameraMovementSystem(struct THE_Camera *cam, float deltatime);

#endif