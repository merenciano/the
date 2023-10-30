#ifndef THE_RENDER_CONFIG_H
#define THE_RENDER_CONFIG_H

#include "core/thefinitions.h"

#ifdef THE_ELEM_ARRAY_32
#define IDX_T unsigned
#elif defined(THE_ELEM_ARRAY_16)
#define IDX_T uint16_t
#else
#define THE_ELEM_ARRAY_32
#define IDX_T unsigned
#endif

#define THE_RENDER_CHECKS

#define THE_RENDER_QUEUE_CAPACITY 1024
#define THE_FRAME_POOL_SIZE (THE_MB(16))
#define THE_MAX_TEXTURES 64
#define THE_MAX_MESHES 64
#define THE_MAX_FRAMEBUFFERS 32
#define THE_MAX_SHADERS 32

#endif // THE_RENDER_CONFIG_H
