#ifndef THE_CORE_IO_H
#define THE_CORE_IO_H

#include "thefinitions.h"
#include <stdint.h>
#include <stdbool.h>

enum THE_Input {
	THE_KEY_UP = 0,
	THE_KEY_LEFT,
	THE_KEY_RIGHT,
	THE_KEY_DOWN,
	THE_KEY_1,
	THE_KEY_2,
	THE_KEY_3,
	THE_KEY_4,
	THE_MOUSE_LEFT,
	THE_MOUSE_RIGHT,
	THE_INPUT_COUNT
};

typedef struct {
	float mouse_x;
	float mouse_y;
	float scroll;
	uint32_t input_bitmap;
} THE_InputState;

bool THE_IOInit(const char *title, int32_t width, int32_t height, bool limit_framerate);
void THE_IOPollEvents(void);

int32_t THE_WindowShouldClose(void);
void THE_WindowSwapBuffers(void);
int32_t THE_WindowGetWidth(void);
int32_t THE_WindowGetHeight(void);

void THE_InputUpdate(void);
void THE_InputSetScroll(float offset);
float THE_InputGetScroll(void);
bool THE_InputIsButtonPressed(enum THE_Input button);
bool THE_InputIsButtonDown(enum THE_Input button);
bool THE_InputIsButtonUp(enum THE_Input button);
float THE_InputGetMouseX(void);
float THE_InputGetMouseY(void);
void THE_InputDisableCursor(void);
void THE_InputEnableCursor(void);
void THE_InputCapture(bool mouse, bool kb);

#endif

