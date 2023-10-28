#ifndef THE_CORE_EXECUTION_H
#define THE_CORE_EXECUTION_H

#include <stdlib.h>
#include <stdbool.h>

struct THE_Config {
	void (*init_func)(void *context);
	bool (*update_func)(void *context);
	void *context;

	// Memory
	size_t heap_size; // Memory chunk size for every alloc THE does.

	// Window
	const char *window_title;
	int window_width;
	int window_height;
	bool vsync;
};

extern float deltatime;

void THE_Start(struct THE_Config *config);
void THE_End(void);

#endif

