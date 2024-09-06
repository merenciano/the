#ifndef THE_SCENE_H
#define THE_SCENE_H

#include "render/pixels.h"

struct the_cam {
	float view[16];
	float proj[16];
	float far;
	float fov;
};

struct the_control_config {
	float speed;
	float sensitivity;
	float scroll_sensitivity;
	float deltatime;
};

struct the_entity {
	float transform[16];
	the_mesh mesh;
	the_mat mat;
};

typedef struct the_entity theent;
THE_DECL_ARR(theent);
THE_DECL_POOL(theent);

extern struct thepool_theent entity_pool;
extern struct the_cam camera;

void the_camera_init_default(struct the_cam *cam);
void the_camera_init(struct the_cam *cam, struct the_vec3 pos, struct the_vec3 target);
struct the_vec3 the_camera_eye(struct the_cam *cam);
void the_camera_control(struct the_cam *cam, struct the_control_config cfg);

// Matrix with zeroed translation i.e. projection * vec4(vec3(view)). For skybox.
float *the_camera_static_vp(struct the_cam *cam, the_mat4 out);

#endif // THE_SCENE_H
