#ifndef NYAS_SCENE_CAMERA_H
#define NYAS_SCENE_CAMERA_H

typedef struct nyas_cam {
	float view[16];
	float proj[16];
	float far;
	float fov;
} nyas_cam;

extern nyas_cam camera;

void nyas_camera_init(nyas_cam *cam, float fov,
	float far, float width, float height);

float *nyas_camera_pos(float *out_v3, nyas_cam *cam);
float *nyas_camera_fwd(float *out_v3, nyas_cam *cam);

/* Matrix without the translate values.
 * Result from projection * vec4(vec3(view));
 * Used for the skybox.
 */
float *nyas_camera_static_vp(float *out_m4, nyas_cam *cam);
void nyas_camera_control(nyas_cam *cam, float deltatime);

#endif // NYAS_SCENE_CAMERA_H
