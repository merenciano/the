#include "io.h"
#include "log.h"
#include "mem.h"

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
	(void)window; // Unused
	(void)x_offset; // Unused
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

	glfwMakeContextCurrent(internal_window);
#ifndef __EMSCRIPTEN__
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

	if (!limit_framerate) {
		glfwSwapInterval(0);
	}

	glfwSetScrollCallback(internal_window, scrollcallback);

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

int
nyas_file_read(const char *path, char **dst, size_t *size)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		NYAS_LOG_ERR("File open failed for %s.", path);
		return NYAS_ERR_FILE;
	}

	fseek(f, 0L, SEEK_END);
	*size = ftell(f) + 1;
	rewind(f);

	*dst = nyas_alloc(*size);
	if (!*dst) {
		NYAS_LOG_ERR("Alloc (%lu bytes) failed.", *size);
		fclose(f);
		return NYAS_ERR_ALLOC;
	}

	if (fread(*dst, *size - 1, 1, f) != 1) {
		NYAS_LOG_ERR("File read failed for %s.", path);
		nyas_free(*dst);
		fclose(f);
		return NYAS_ERR_FILE;
	}

	fclose(f);
	(*dst)[*size - 1] = '\0';
	return NYAS_OK;
}

void
nyas_io_poll(void)
{
	glfwPollEvents();
}

// OUTPUT  ------------------------------

int
nyas_window_closed(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return glfwWindowShouldClose(internal_window);
}

void
nyas_window_swap(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSwapBuffers(internal_window);
}

nyas_v2i
nyas_window_size(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	nyas_v2i sz;
	glfwGetWindowSize(internal_window, &sz.x, &sz.y);
	return sz;
}

int
nyas_window_width(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	int width;
	glfwGetWindowSize(internal_window, &width, NULL);
	return width;
}

int
nyas_window_height(void)
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
nyas_input_scroll(void)
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
nyas_input_mouse_x(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return curr.mouse_x;
}

float
nyas_input_mouse_y(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	return curr.mouse_y;
}

void
nyas_input_read(void)
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
nyas_input_cursor_disable(void)
{
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSetInputMode(internal_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void
nyas_input_cursor_mode(int mode)
{
	(void)mode;
	NYAS_ASSERT(internal_window && "The IO system is uninitalized");
	glfwSetInputMode(internal_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void
nyas_input_capture(bool mouse, bool kb)
{
	capture_kb = kb;
	capture_mouse = mouse;
}
