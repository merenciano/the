// TODO: Replantear todo esto de config, manager y ejecucion porque
// no me gusta nada como ha salido.
#ifndef THE_CORE_CONFIG_H
#define THE_CORE_CONFIG_H

#include "thefinitions.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct THE_Config {
	void (*init_func)(void *context);
	bool (*update_func)(void *context);
	void *context;

	// Memory
	size_t heap_size; // Memory chunk size for every alloc THE does.

	// Window
	const char *window_title;
	int32_t window_width;
	int32_t window_height;
	bool vsync;
};

#endif
