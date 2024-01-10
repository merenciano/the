#ifndef NYAS_CORE_IO_H
#define NYAS_CORE_IO_H

#include "nyas_defs.h"
#include <stddef.h>
#include <stdbool.h>

enum nyas_input {
	NYAS_KEY_UP = 0,
	NYAS_KEY_LEFT,
	NYAS_KEY_RIGHT,
	NYAS_KEY_DOWN,
	NYAS_KEY_1,
	NYAS_KEY_2,
	NYAS_KEY_3,
	NYAS_KEY_4,
	NYAS_MOUSE_LEFT,
	NYAS_MOUSE_RIGHT,
	NYAS_INPUT_COUNT
};

typedef struct {
	float mouse_x;
	float mouse_y;
	float scroll;
	unsigned int pressed;
} nyas_input_state;

bool nyas_io_init(const char *title, int width, int height, bool limit_framerate);
void nyas_io_poll(void);

int nyas_window_closed(void);
void nyas_window_swap(void);
nyas_v2i nyas_window_size(void);
int nyas_window_width(void); // TODO: DEPRECATED: nyas_window_size instead
int nyas_window_height(void); // TODO: DEPRECATED: nyas_window_size instead

void nyas_input_read(void);
void nyas_input_set_scroll(float offset); // TODO: update with other input
float nyas_input_scroll(void);
bool nyas_input_pressed(enum nyas_input button);
bool nyas_input_down(enum nyas_input button);
bool nyas_input_up(enum nyas_input button);
float nyas_input_mouse_x(void); // TODO: Merge with y
float nyas_input_mouse_y(void);
void nyas_input_cursor_disable(void); // TODO: Merge with enable
void nyas_input_cursor_enable(void); // TODO: flags for cursor mode and capture
void nyas_input_capture(bool mouse, bool kb);

int nyas_file_read(const char *path, char **dst, size_t *size);

//TODO: Revisar como exponer las variables internas
extern void *internal_window;

#endif // NYAS_CORE_IO_H
