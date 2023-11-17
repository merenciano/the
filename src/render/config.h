#ifndef NYAS_PIXEL_CONFIG_H
#define NYAS_PIXEL_CONFIG_H

#include "core/nyas_defs.h"

#ifdef NYAS_ELEM_ARRAY_32
#define IDX_T unsigned
#elif defined(NYAS_ELEM_ARRAY_16)
#define IDX_T uint16_t
#else
#define NYAS_ELEM_ARRAY_32
#define IDX_T unsigned int
#endif

#define NYAS_PIXEL_CHECKS

#define NYAS_RENDER_QUEUE_CAPACITY 1024
#define NYAS_FRAME_POOL_SIZE (NYAS_MB(16))
#define NYAS_MAX_TEXTURES 64
#define NYAS_MAX_MESHES 64
#define NYAS_MAX_FRAMEBUFFERS 32
#define NYAS_MAX_SHADERS 32

#endif // NYAS_PIXEL_CONFIG_H
