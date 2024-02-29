#include "io.h"
#include "mem.h"

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <string.h>
#include <time.h>

#define NYAS_MOUSE_BUTTON_UPDATE(MBTN) \
	io.mouse_button[(MBTN)] =          \
	  ((io.mouse_button[(MBTN)] << 1) | glfwGetMouseButton(io.internal_window, (MBTN))) & 3

#define NYAS_KEY_UPDATE(KEY) \
	(io.keys[(KEY)] = ((io.keys[(KEY)] << 1) | glfwGetKey(io.internal_window, (KEY))) & 3)

struct nyas_io io = { .internal_window = NULL };
struct nyas_io *nyas_io = &io;

static inline void
nyas_input_update(void)
{
	NYAS_KEY_UPDATE(NYAS_KEY_INVALID);
	NYAS_KEY_UPDATE(NYAS_KEY_SPACE);
	NYAS_KEY_UPDATE(NYAS_KEY_APOSTROPHE);
	NYAS_KEY_UPDATE(NYAS_KEY_COMMA);
	NYAS_KEY_UPDATE(NYAS_KEY_MINUS);
	NYAS_KEY_UPDATE(NYAS_KEY_PERIOD);
	NYAS_KEY_UPDATE(NYAS_KEY_SLASH);
	NYAS_KEY_UPDATE(NYAS_KEY_0);
	NYAS_KEY_UPDATE(NYAS_KEY_1);
	NYAS_KEY_UPDATE(NYAS_KEY_2);
	NYAS_KEY_UPDATE(NYAS_KEY_3);
	NYAS_KEY_UPDATE(NYAS_KEY_4);
	NYAS_KEY_UPDATE(NYAS_KEY_5);
	NYAS_KEY_UPDATE(NYAS_KEY_6);
	NYAS_KEY_UPDATE(NYAS_KEY_7);
	NYAS_KEY_UPDATE(NYAS_KEY_8);
	NYAS_KEY_UPDATE(NYAS_KEY_9);
	NYAS_KEY_UPDATE(NYAS_KEY_SEMICOLON);
	NYAS_KEY_UPDATE(NYAS_KEY_EQUAL);
	NYAS_KEY_UPDATE(NYAS_KEY_A);
	NYAS_KEY_UPDATE(NYAS_KEY_B);
	NYAS_KEY_UPDATE(NYAS_KEY_C);
	NYAS_KEY_UPDATE(NYAS_KEY_D);
	NYAS_KEY_UPDATE(NYAS_KEY_E);
	NYAS_KEY_UPDATE(NYAS_KEY_F);
	NYAS_KEY_UPDATE(NYAS_KEY_G);
	NYAS_KEY_UPDATE(NYAS_KEY_H);
	NYAS_KEY_UPDATE(NYAS_KEY_I);
	NYAS_KEY_UPDATE(NYAS_KEY_J);
	NYAS_KEY_UPDATE(NYAS_KEY_K);
	NYAS_KEY_UPDATE(NYAS_KEY_L);
	NYAS_KEY_UPDATE(NYAS_KEY_M);
	NYAS_KEY_UPDATE(NYAS_KEY_N);
	NYAS_KEY_UPDATE(NYAS_KEY_O);
	NYAS_KEY_UPDATE(NYAS_KEY_P);
	NYAS_KEY_UPDATE(NYAS_KEY_Q);
	NYAS_KEY_UPDATE(NYAS_KEY_R);
	NYAS_KEY_UPDATE(NYAS_KEY_S);
	NYAS_KEY_UPDATE(NYAS_KEY_T);
	NYAS_KEY_UPDATE(NYAS_KEY_U);
	NYAS_KEY_UPDATE(NYAS_KEY_V);
	NYAS_KEY_UPDATE(NYAS_KEY_W);
	NYAS_KEY_UPDATE(NYAS_KEY_X);
	NYAS_KEY_UPDATE(NYAS_KEY_Y);
	NYAS_KEY_UPDATE(NYAS_KEY_Z);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT_BRACKET);
	NYAS_KEY_UPDATE(NYAS_KEY_BACKSLASH);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT_BRACKET);
	NYAS_KEY_UPDATE(NYAS_KEY_GRAVE_ACCENT);
	NYAS_KEY_UPDATE(NYAS_KEY_WORLD_1);
	NYAS_KEY_UPDATE(NYAS_KEY_WORLD_2);
	NYAS_KEY_UPDATE(NYAS_KEY_ESCAPE);
	NYAS_KEY_UPDATE(NYAS_KEY_ENTER);
	NYAS_KEY_UPDATE(NYAS_KEY_TAB);
	NYAS_KEY_UPDATE(NYAS_KEY_BACKSPACE);
	NYAS_KEY_UPDATE(NYAS_KEY_INSERT);
	NYAS_KEY_UPDATE(NYAS_KEY_DELETE);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT);
	NYAS_KEY_UPDATE(NYAS_KEY_DOWN);
	NYAS_KEY_UPDATE(NYAS_KEY_UP);
	NYAS_KEY_UPDATE(NYAS_KEY_PAGE_UP);
	NYAS_KEY_UPDATE(NYAS_KEY_PAGE_DOWN);
	NYAS_KEY_UPDATE(NYAS_KEY_HOME);
	NYAS_KEY_UPDATE(NYAS_KEY_END);
	NYAS_KEY_UPDATE(NYAS_KEY_CAPS_LOCK);
	NYAS_KEY_UPDATE(NYAS_KEY_SCROLL_LOCK);
	NYAS_KEY_UPDATE(NYAS_KEY_NUM_LOCK);
	NYAS_KEY_UPDATE(NYAS_KEY_PRINT_SCREEN);
	NYAS_KEY_UPDATE(NYAS_KEY_PAUSE);
	NYAS_KEY_UPDATE(NYAS_KEY_F1);
	NYAS_KEY_UPDATE(NYAS_KEY_F2);
	NYAS_KEY_UPDATE(NYAS_KEY_F3);
	NYAS_KEY_UPDATE(NYAS_KEY_F4);
	NYAS_KEY_UPDATE(NYAS_KEY_F5);
	NYAS_KEY_UPDATE(NYAS_KEY_F6);
	NYAS_KEY_UPDATE(NYAS_KEY_F7);
	NYAS_KEY_UPDATE(NYAS_KEY_F8);
	NYAS_KEY_UPDATE(NYAS_KEY_F9);
	NYAS_KEY_UPDATE(NYAS_KEY_F10);
	NYAS_KEY_UPDATE(NYAS_KEY_F11);
	NYAS_KEY_UPDATE(NYAS_KEY_F12);
	NYAS_KEY_UPDATE(NYAS_KEY_F13);
	NYAS_KEY_UPDATE(NYAS_KEY_F14);
	NYAS_KEY_UPDATE(NYAS_KEY_F15);
	NYAS_KEY_UPDATE(NYAS_KEY_F16);
	NYAS_KEY_UPDATE(NYAS_KEY_F17);
	NYAS_KEY_UPDATE(NYAS_KEY_F18);
	NYAS_KEY_UPDATE(NYAS_KEY_F19);
	NYAS_KEY_UPDATE(NYAS_KEY_F20);
	NYAS_KEY_UPDATE(NYAS_KEY_F21);
	NYAS_KEY_UPDATE(NYAS_KEY_F22);
	NYAS_KEY_UPDATE(NYAS_KEY_F23);
	NYAS_KEY_UPDATE(NYAS_KEY_F24);
	NYAS_KEY_UPDATE(NYAS_KEY_F25);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_0);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_1);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_2);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_3);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_4);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_5);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_6);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_7);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_8);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_9);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_DECIMAL);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_DIVIDE);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_MULTIPLY);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_SUBTRACT);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_ADD);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_ENTER);
	NYAS_KEY_UPDATE(NYAS_KEY_KP_EQUAL);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT_SHIFT);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT_CONTROL);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT_ALT);
	NYAS_KEY_UPDATE(NYAS_KEY_LEFT_SUPER);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT_SHIFT);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT_CONTROL);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT_ALT);
	NYAS_KEY_UPDATE(NYAS_KEY_RIGHT_SUPER);
	NYAS_KEY_UPDATE(NYAS_KEY_MENU);

	NYAS_MOUSE_BUTTON_UPDATE(NYAS_MOUSE_LEFT);
	NYAS_MOUSE_BUTTON_UPDATE(NYAS_MOUSE_RIGHT);
	NYAS_MOUSE_BUTTON_UPDATE(NYAS_MOUSE_MIDDLE);

	double x, y;
	glfwGetCursorPos(io.internal_window, &x, &y);
	io.mouse_pos = (struct nyas_vec2){ (float)x, (float)y };
}

static void
nyas__scrollcallback(GLFWwindow *window, double x_offset, double y_offset)
{
	(void)window; // Unused
	io.mouse_scroll.x = (float)x_offset;
	io.mouse_scroll.y = (float)y_offset;
}

static void
nyas__cursor_enter_callback(GLFWwindow *window, int entered)
{
	(void)window; // Unused
	io.window_hovered = entered;
}

static void
nyas__window_focus_callback(GLFWwindow *window, int focused)
{
	(void)window; // Unused
	io.window_focused = focused;
}

bool
nyas_io_init(const char *title, struct nyas_point window_size)
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
	glfwSetScrollCallback(io.internal_window, nyas__scrollcallback);
	glfwSetCursorEnterCallback(io.internal_window, nyas__cursor_enter_callback);
	glfwSetWindowFocusCallback(io.internal_window, nyas__window_focus_callback);
	glfwSetInputMode(io.internal_window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetInputMode(io.internal_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

	io.show_cursor = true;
	io.capture_mouse = true;
	io.capture_keyboard = true;

	nyas_io_poll();
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
	NYAS_ASSERT(io.internal_window && "The IO system is uninitalized");
	io.mouse_scroll = (struct nyas_vec2){ 0.0f, 0.0f };
	glfwPollEvents();
	nyas_input_update();
	io.window_closed = glfwWindowShouldClose(io.internal_window);
	glfwGetWindowSize(io.internal_window, &io.window_size.x, &io.window_size.y);

	if (io.show_cursor) {
		glfwSetInputMode(io.internal_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else {
		glfwSetInputMode(io.internal_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void
nyas_window_swap(void)
{
	NYAS_ASSERT(io.internal_window && "The IO system is uninitalized");
	glfwSwapBuffers(io.internal_window);
}

nyas_chrono
nyas_time(void)
{
	struct timespec time;
	clock_gettime(2, &time);
	return time.tv_nsec + time.tv_sec * 1000000000;
}
