#include "io.h"

#include <assert.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

void *internal_window = NULL;
static nyas_input_state curr;
static nyas_input_state prev;
static bool capture_kb;
static bool capture_mouse;

static int32_t nyas_io_to_glfw[NYAS_INPUT_COUNT] = {
	GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_S,
	GLFW_KEY_SPACE, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_LEFT_SHIFT,
	GLFW_MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_2
};

static void
scrollcallback(GLFWwindow *window, double x_offset, double y_offset)
{
	nyas_input_set_scroll((float)y_offset);
}

bool
nyas_io_init(const char *title, int width, int height, bool limit_framerate)
{
	if (!glfwInit() || internal_window) { // glfwInit fail or window init already
		return false;
	}

	internal_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!internal_window) {
		glfwTerminate();
		return false;
	}

	// I call this function here because nyas only supports one window
	glfwMakeContextCurrent(internal_window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

	if (!limit_framerate) {
		glfwSwapInterval(0);
	}

	glfwSetScrollCallback(internal_window, scrollcallback);
	//nyas_UIToolsInit(internal_window);

	curr.pressed = 0;
	prev.pressed = 0;
	curr.scroll = 0.0f;
	prev.scroll = 0.0f;
	curr.mouse_x = 0.0f;
	curr.mouse_y = 0.0f;
	prev.mouse_x = 0.0f;
	prev.mouse_y = 0.0f;
	capture_kb = true;
	capture_mouse = true;

	return true;
}

void
nyas_io_poll()
{
	glfwPollEvents();
}

// OUTPUT  ------------------------------

int
nyas_window_closed()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return glfwWindowShouldClose(internal_window);
}

void
nyas_window_swap()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSwapBuffers(internal_window);
}

int *
nyas_window_size(int *out)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwGetWindowSize(internal_window, out, out + 1);
	return out;
}

int
nyas_window_width()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	int width;
	glfwGetWindowSize(internal_window, &width, NULL);
	return width;
}

int
nyas_window_height()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	int height;
	glfwGetWindowSize(internal_window, NULL, &height);
	return height;
}

// INPUT ------------------------------

void
nyas_input_set_scroll(float offset)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	if (capture_mouse) {
		curr.scroll = offset;
	}
}

float
nyas_input_scroll()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return prev.scroll;
}

bool
nyas_input_pressed(enum nyas_input button)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	if (!capture_kb) {
		return false;
	}

	return curr.pressed & (1 << button);
}

bool
nyas_input_down(enum nyas_input button)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	if (!nyas_input_pressed(button)) {
		return false;
	}

	return !(prev.pressed & (1 << button));
}

bool
nyas_input_up(enum nyas_input button)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");

	if (!capture_kb) {
		return false;
	}

	if (nyas_input_pressed(button)) {
		return false;
	}

	return prev.pressed & (1 << button);
}

float
nyas_input_mouse_x()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return curr.mouse_x;
}

float
nyas_input_mouse_y()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return curr.mouse_y;
}

void
nyas_input_read()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	prev = curr;
	curr.scroll = 0.0f;
	curr.pressed = 0;
	for (int32_t i = 0; i < NYAS_MOUSE_LEFT; ++i) {
		curr.pressed |= ((bool)glfwGetKey(internal_window, nyas_io_to_glfw[i]) << i);
	}
	curr.pressed |= ((bool)glfwGetMouseButton(internal_window,
	                                          nyas_io_to_glfw[NYAS_MOUSE_LEFT]) << NYAS_MOUSE_LEFT);
	curr.pressed |= ((bool)glfwGetMouseButton(internal_window,
	                            nyas_io_to_glfw[NYAS_MOUSE_RIGHT]) << NYAS_MOUSE_RIGHT);

	double x, y;
	glfwGetCursorPos(internal_window, &x, &y);
	curr.mouse_x = (float)x;
	curr.mouse_y = (float)y;
}

void
nyas_input_cursor_disable()
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSetInputMode(internal_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void
nyas_input_cursor_mode(int mode)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSetInputMode(internal_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void
nyas_input_capture(bool mouse, bool kb)
{
	capture_kb = kb;
	capture_mouse = mouse;
}
