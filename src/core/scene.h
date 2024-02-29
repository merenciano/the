#ifndef NYAS_SCENE_H
#define NYAS_SCENE_H

#include "render/pixels.h"

struct nyas_cam {
	float view[16];
	float proj[16];
	float far;
	float fov;
};

struct nyas_control_config {
	float speed;
	float sensitivity;
	float scroll_sensitivity;
	float deltatime;
};

struct nyas_entity {
	float transform[16];
	nyas_mesh mesh;
	nyas_mat mat;
};

typedef struct nyas_entity nyent;
NYAS_DECL_ARR(nyent);
NYAS_DECL_POOL(nyent);

extern struct nypool_nyent entity_pool;
extern struct nyas_cam camera;

void nyas_camera_init_default(struct nyas_cam *cam);
void nyas_camera_init(struct nyas_cam *cam, struct nyas_vec3 pos, struct nyas_vec3 target);
struct nyas_vec3 nyas_camera_eye(struct nyas_cam *cam);
void nyas_camera_control(struct nyas_cam *cam, struct nyas_control_config cfg);

// Matrix with zeroed translation i.e. projection * vec4(vec3(view)). For skybox.
float *nyas_camera_static_vp(struct nyas_cam *cam, nyas_mat4 out);

#endif // NYAS_SCENE_H
