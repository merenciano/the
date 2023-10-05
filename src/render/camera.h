#ifndef __THE_RENDER_CAMERA_H__
#define __THE_RENDER_CAMERA_H__

#include "renderer.h"

extern void THE_CameraInit(THE_Camera* cam, float fov, float far, u32 width, u32 height, u8 is_light);
extern struct vec3 THE_CameraPosition(THE_Camera *cam);
extern struct vec3 THE_CameraForward(THE_Camera *cam);
extern THE_Texture THE_CameraOutputColorTexture(THE_Camera *cam);

/* Matrix without the translate values.
 * Result from projection * vec4(vec3(view));
 * Used for the skybox
 */
extern struct mat4 THE_CameraStaticViewProjection(THE_Camera *cam);

extern void THE_CameraMovementSystem(THE_Camera *cam);

#endif
