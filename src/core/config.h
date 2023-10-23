// TODO: Replantear todo esto de config, manager y ejecucion porque
// no me gusta nada como ha salido.
#ifndef THE_CORE_CONFIG_H
#define THE_CORE_CONFIG_H

#include "thefinitions.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

extern const char *WINDOW_TITLE;
extern const int32_t MAX_TEXTURES;
extern const int32_t MAX_BUFFERS;

struct THE_Config {
	THE_Callback init_func;
	THE_Callback logic_func;
	THE_Callback close_func;
	size_t alloc_capacity; // Memory chunk size for the buddy allocator (in bytes).
	int32_t render_queue_capacity; // Maximum renderable entities in the scene. This is for engine allocation purposes.
	int32_t max_geometries;
	int32_t window_width;
	int32_t window_height;
	bool vsync;
};

#endif
