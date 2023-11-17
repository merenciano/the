#ifndef NYAS_CORE_EXECUTION_H
#define NYAS_CORE_EXECUTION_H

#include <stdlib.h>
#include <stdbool.h>

struct nyas_config {
	void (*init_func)(void *context);
	bool (*update_func)(void *context);
	void *context;

	// Memory
	size_t heap_size; // Memory chunk size for dynamic alloc.

	// Window
	const char *window_title;
	int window_width;
	int window_height;
	bool vsync;
};

extern float deltatime;

void nyas_app_start(struct nyas_config *config);
void nyas_app_end(void);

#endif

