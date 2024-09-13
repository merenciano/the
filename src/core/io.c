#include "io.h"
#include "mem.h"
#include <bits/time.h>

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <string.h>
#include <time.h>

#define THE_MOUSE_BUTTON_UPDATE(MBTN) \
	io.mouse_button[(MBTN)] =          \
	  ((io.mouse_button[(MBTN)] << 1) | glfwGetMouseButton(io.internal_window, (MBTN))) & 3

#define THE_KEY_UPDATE(KEY) \
	(io.keys[(KEY)] = ((io.keys[(KEY)] << 1) | glfwGetKey(io.internal_window, (KEY))) & 3)

struct the_io io = { .internal_window = NULL };
struct the_io *the_io = &io;

static inline void
the_input_update(void)
{
	THE_KEY_UPDATE(THE_KEY_INVALID);
	THE_KEY_UPDATE(THE_KEY_SPACE);
	THE_KEY_UPDATE(THE_KEY_APOSTROPHE);
	THE_KEY_UPDATE(THE_KEY_COMMA);
	THE_KEY_UPDATE(THE_KEY_MINUS);
	THE_KEY_UPDATE(THE_KEY_PERIOD);
	THE_KEY_UPDATE(THE_KEY_SLASH);
	THE_KEY_UPDATE(THE_KEY_0);
	THE_KEY_UPDATE(THE_KEY_1);
	THE_KEY_UPDATE(THE_KEY_2);
	THE_KEY_UPDATE(THE_KEY_3);
	THE_KEY_UPDATE(THE_KEY_4);
	THE_KEY_UPDATE(THE_KEY_5);
	THE_KEY_UPDATE(THE_KEY_6);
	THE_KEY_UPDATE(THE_KEY_7);
	THE_KEY_UPDATE(THE_KEY_8);
	THE_KEY_UPDATE(THE_KEY_9);
	THE_KEY_UPDATE(THE_KEY_SEMICOLON);
	THE_KEY_UPDATE(THE_KEY_EQUAL);
	THE_KEY_UPDATE(THE_KEY_A);
	THE_KEY_UPDATE(THE_KEY_B);
	THE_KEY_UPDATE(THE_KEY_C);
	THE_KEY_UPDATE(THE_KEY_D);
	THE_KEY_UPDATE(THE_KEY_E);
	THE_KEY_UPDATE(THE_KEY_F);
	THE_KEY_UPDATE(THE_KEY_G);
	THE_KEY_UPDATE(THE_KEY_H);
	THE_KEY_UPDATE(THE_KEY_I);
	THE_KEY_UPDATE(THE_KEY_J);
	THE_KEY_UPDATE(THE_KEY_K);
	THE_KEY_UPDATE(THE_KEY_L);
	THE_KEY_UPDATE(THE_KEY_M);
	THE_KEY_UPDATE(THE_KEY_N);
	THE_KEY_UPDATE(THE_KEY_O);
	THE_KEY_UPDATE(THE_KEY_P);
	THE_KEY_UPDATE(THE_KEY_Q);
	THE_KEY_UPDATE(THE_KEY_R);
	THE_KEY_UPDATE(THE_KEY_S);
	THE_KEY_UPDATE(THE_KEY_T);
	THE_KEY_UPDATE(THE_KEY_U);
	THE_KEY_UPDATE(THE_KEY_V);
	THE_KEY_UPDATE(THE_KEY_W);
	THE_KEY_UPDATE(THE_KEY_X);
	THE_KEY_UPDATE(THE_KEY_Y);
	THE_KEY_UPDATE(THE_KEY_Z);
	THE_KEY_UPDATE(THE_KEY_LEFT_BRACKET);
	THE_KEY_UPDATE(THE_KEY_BACKSLASH);
	THE_KEY_UPDATE(THE_KEY_RIGHT_BRACKET);
	THE_KEY_UPDATE(THE_KEY_GRAVE_ACCENT);
	THE_KEY_UPDATE(THE_KEY_WORLD_1);
	THE_KEY_UPDATE(THE_KEY_WORLD_2);
	THE_KEY_UPDATE(THE_KEY_ESCAPE);
	THE_KEY_UPDATE(THE_KEY_ENTER);
	THE_KEY_UPDATE(THE_KEY_TAB);
	THE_KEY_UPDATE(THE_KEY_BACKSPACE);
	THE_KEY_UPDATE(THE_KEY_INSERT);
	THE_KEY_UPDATE(THE_KEY_DELETE);
	THE_KEY_UPDATE(THE_KEY_RIGHT);
	THE_KEY_UPDATE(THE_KEY_LEFT);
	THE_KEY_UPDATE(THE_KEY_DOWN);
	THE_KEY_UPDATE(THE_KEY_UP);
	THE_KEY_UPDATE(THE_KEY_PAGE_UP);
	THE_KEY_UPDATE(THE_KEY_PAGE_DOWN);
	THE_KEY_UPDATE(THE_KEY_HOME);
	THE_KEY_UPDATE(THE_KEY_END);
	THE_KEY_UPDATE(THE_KEY_CAPS_LOCK);
	THE_KEY_UPDATE(THE_KEY_SCROLL_LOCK);
	THE_KEY_UPDATE(THE_KEY_NUM_LOCK);
	THE_KEY_UPDATE(THE_KEY_PRINT_SCREEN);
	THE_KEY_UPDATE(THE_KEY_PAUSE);
	THE_KEY_UPDATE(THE_KEY_F1);
	THE_KEY_UPDATE(THE_KEY_F2);
	THE_KEY_UPDATE(THE_KEY_F3);
	THE_KEY_UPDATE(THE_KEY_F4);
	THE_KEY_UPDATE(THE_KEY_F5);
	THE_KEY_UPDATE(THE_KEY_F6);
	THE_KEY_UPDATE(THE_KEY_F7);
	THE_KEY_UPDATE(THE_KEY_F8);
	THE_KEY_UPDATE(THE_KEY_F9);
	THE_KEY_UPDATE(THE_KEY_F10);
	THE_KEY_UPDATE(THE_KEY_F11);
	THE_KEY_UPDATE(THE_KEY_F12);
	THE_KEY_UPDATE(THE_KEY_F13);
	THE_KEY_UPDATE(THE_KEY_F14);
	THE_KEY_UPDATE(THE_KEY_F15);
	THE_KEY_UPDATE(THE_KEY_F16);
	THE_KEY_UPDATE(THE_KEY_F17);
	THE_KEY_UPDATE(THE_KEY_F18);
	THE_KEY_UPDATE(THE_KEY_F19);
	THE_KEY_UPDATE(THE_KEY_F20);
	THE_KEY_UPDATE(THE_KEY_F21);
	THE_KEY_UPDATE(THE_KEY_F22);
	THE_KEY_UPDATE(THE_KEY_F23);
	THE_KEY_UPDATE(THE_KEY_F24);
	THE_KEY_UPDATE(THE_KEY_F25);
	THE_KEY_UPDATE(THE_KEY_KP_0);
	THE_KEY_UPDATE(THE_KEY_KP_1);
	THE_KEY_UPDATE(THE_KEY_KP_2);
	THE_KEY_UPDATE(THE_KEY_KP_3);
	THE_KEY_UPDATE(THE_KEY_KP_4);
	THE_KEY_UPDATE(THE_KEY_KP_5);
	THE_KEY_UPDATE(THE_KEY_KP_6);
	THE_KEY_UPDATE(THE_KEY_KP_7);
	THE_KEY_UPDATE(THE_KEY_KP_8);
	THE_KEY_UPDATE(THE_KEY_KP_9);
	THE_KEY_UPDATE(THE_KEY_KP_DECIMAL);
	THE_KEY_UPDATE(THE_KEY_KP_DIVIDE);
	THE_KEY_UPDATE(THE_KEY_KP_MULTIPLY);
	THE_KEY_UPDATE(THE_KEY_KP_SUBTRACT);
	THE_KEY_UPDATE(THE_KEY_KP_ADD);
	THE_KEY_UPDATE(THE_KEY_KP_ENTER);
	THE_KEY_UPDATE(THE_KEY_KP_EQUAL);
	THE_KEY_UPDATE(THE_KEY_LEFT_SHIFT);
	THE_KEY_UPDATE(THE_KEY_LEFT_CONTROL);
	THE_KEY_UPDATE(THE_KEY_LEFT_ALT);
	THE_KEY_UPDATE(THE_KEY_LEFT_SUPER);
	THE_KEY_UPDATE(THE_KEY_RIGHT_SHIFT);
	THE_KEY_UPDATE(THE_KEY_RIGHT_CONTROL);
	THE_KEY_UPDATE(THE_KEY_RIGHT_ALT);
	THE_KEY_UPDATE(THE_KEY_RIGHT_SUPER);
	THE_KEY_UPDATE(THE_KEY_MENU);

	THE_MOUSE_BUTTON_UPDATE(THE_MOUSE_LEFT);
	THE_MOUSE_BUTTON_UPDATE(THE_MOUSE_RIGHT);
	THE_MOUSE_BUTTON_UPDATE(THE_MOUSE_MIDDLE);

	double x, y;
	glfwGetCursorPos(io.internal_window, &x, &y);
	io.mouse_pos = (struct the_vec2){ (float)x, (float)y };
}

static void
the__scrollcallback(GLFWwindow *window, double x_offset, double y_offset)
{
	(void)window; // Unused
	io.mouse_scroll.x = (float)x_offset;
	io.mouse_scroll.y = (float)y_offset;
}

static void
the__cursor_enter_callback(GLFWwindow *window, int entered)
{
	(void)window; // Unused
	io.window_hovered = entered;
}

static void
the__window_focus_callback(GLFWwindow *window, int focused)
{
	(void)window; // Unused
	io.window_focused = focused;
}

bool
the_io_init(const char *title, struct the_point window_size)
{
	memset(&io, 0, sizeof(io));
	if (!glfwInit()) {
		return false;
	}

	io.internal_window = glfwCreateWindow(window_size.x, window_size.y, title, NULL, NULL);
	if (!io.internal_window) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(io.internal_window);
#ifndef __EMSCRIPTEN__
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

	glfwSwapInterval(1);
	glfwSetScrollCallback(io.internal_window, the__scrollcallback);
	glfwSetCursorEnterCallback(io.internal_window, the__cursor_enter_callback);
	glfwSetWindowFocusCallback(io.internal_window, the__window_focus_callback);
	glfwSetInputMode(io.internal_window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetInputMode(io.internal_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

	io.show_cursor = true;
	io.capture_mouse = true;
	io.capture_keyboard = true;

	the_io_poll();
	return true;
}

int
the_file_read(const char *path, char **dst, size_t *size)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		THE_LOG_ERR("File open failed for %s.", path);
		return THE_ERR_FILE;
	}

	fseek(f, 0L, SEEK_END);
	*size = ftell(f) + 1;
	rewind(f);

	*dst = the_alloc(*size);
	if (!*dst) {
		THE_LOG_ERR("Alloc (%lu bytes) failed.", *size);
		fclose(f);
		return THE_ERR_ALLOC;
	}

	if (fread(*dst, *size - 1, 1, f) != 1) {
		THE_LOG_ERR("File read failed for %s.", path);
		the_free(*dst);
		fclose(f);
		return THE_ERR_FILE;
	}

	fclose(f);
	(*dst)[*size - 1] = '\0';
	return THE_OK;
}

void
the_io_poll(void)
{
	THE_ASSERT(io.internal_window && "The IO system is uninitalized");
	io.mouse_scroll = (struct the_vec2){ 0.0f, 0.0f };
	glfwPollEvents();
	the_input_update();
	io.window_closed = glfwWindowShouldClose(io.internal_window);
	glfwGetWindowSize(io.internal_window, &io.window_size.x, &io.window_size.y);

	if (io.show_cursor) {
		glfwSetInputMode(io.internal_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
		glfwSetInputMode(io.internal_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void
the_window_swap(void)
{
	THE_ASSERT(io.internal_window && "The IO system is uninitalized");
	glfwSwapBuffers(io.internal_window);
}

the_time
the_now(void)
{
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return (the_time){.sec = time.tv_sec, .ns = time.tv_nsec};
}

int64_t 
the_elapsed(the_time from, the_time to)
{
	return 1000000000 * (to.sec - from.sec) + to.ns - from.ns;
}
