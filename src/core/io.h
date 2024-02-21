#ifndef NYAS_CORE_IO_H
#define NYAS_CORE_IO_H

#include "nyas_core.h"
#include <stdbool.h>
#include <stddef.h>

typedef int8_t nyas_keystate;
enum nyas_keystate {
	NYAS_KEYSTATE_RELEASED = 0, /* Not pressed */
	NYAS_KEYSTATE_DOWN = 1, /* From released to pressed this frame */
	NYAS_KEYSTATE_UP = 2, /* From pressed to released this frame */
	NYAS_KEYSTATE_PRESSED = 3, /* Pressed */
};

typedef int nyas_mouse_button;
enum nyas_mouse_button {
	NYAS_MOUSE_LEFT = 0,
	NYAS_MOUSE_RIGHT = 1,
	NYAS_MOUSE_MIDDLE = 2,
};

typedef int nyas_key;
enum nyas_key {
	NYAS_KEY_INVALID = 0,
	NYAS_KEY_SPACE = 32,
	NYAS_KEY_APOSTROPHE = 39, /* ' */
	NYAS_KEY_COMMA = 44,      /* , */
	NYAS_KEY_MINUS = 45,      /* - */
	NYAS_KEY_PERIOD = 46,     /* . */
	NYAS_KEY_SLASH = 47,      /* / */
	NYAS_KEY_0 = 48,
	NYAS_KEY_1 = 49,
	NYAS_KEY_2 = 50,
	NYAS_KEY_3 = 51,
	NYAS_KEY_4 = 52,
	NYAS_KEY_5 = 53,
	NYAS_KEY_6 = 54,
	NYAS_KEY_7 = 55,
	NYAS_KEY_8 = 56,
	NYAS_KEY_9 = 57,
	NYAS_KEY_SEMICOLON = 59, /* ; */
	NYAS_KEY_EQUAL = 61,     /* = */
	NYAS_KEY_A = 65,
	NYAS_KEY_B = 66,
	NYAS_KEY_C = 67,
	NYAS_KEY_D = 68,
	NYAS_KEY_E = 69,
	NYAS_KEY_F = 70,
	NYAS_KEY_G = 71,
	NYAS_KEY_H = 72,
	NYAS_KEY_I = 73,
	NYAS_KEY_J = 74,
	NYAS_KEY_K = 75,
	NYAS_KEY_L = 76,
	NYAS_KEY_M = 77,
	NYAS_KEY_N = 78,
	NYAS_KEY_O = 79,
	NYAS_KEY_P = 80,
	NYAS_KEY_Q = 81,
	NYAS_KEY_R = 82,
	NYAS_KEY_S = 83,
	NYAS_KEY_T = 84,
	NYAS_KEY_U = 85,
	NYAS_KEY_V = 86,
	NYAS_KEY_W = 87,
	NYAS_KEY_X = 88,
	NYAS_KEY_Y = 89,
	NYAS_KEY_Z = 90,
	NYAS_KEY_LEFT_BRACKET = 91,  /* [ */
	NYAS_KEY_BACKSLASH = 92,     /* \ */
	NYAS_KEY_RIGHT_BRACKET = 93, /* ] */
	NYAS_KEY_GRAVE_ACCENT = 96,  /* ` */
	NYAS_KEY_WORLD_1 = 161,      /* non-US #1 */
	NYAS_KEY_WORLD_2 = 162,      /* non-US #2 */
	NYAS_KEY_ESCAPE = 256,
	NYAS_KEY_ENTER = 257,
	NYAS_KEY_TAB = 258,
	NYAS_KEY_BACKSPACE = 259,
	NYAS_KEY_INSERT = 260,
	NYAS_KEY_DELETE = 261,
	NYAS_KEY_RIGHT = 262,
	NYAS_KEY_LEFT = 263,
	NYAS_KEY_DOWN = 264,
	NYAS_KEY_UP = 265,
	NYAS_KEY_PAGE_UP = 266,
	NYAS_KEY_PAGE_DOWN = 267,
	NYAS_KEY_HOME = 268,
	NYAS_KEY_END = 269,
	NYAS_KEY_CAPS_LOCK = 280,
	NYAS_KEY_SCROLL_LOCK = 281,
	NYAS_KEY_NUM_LOCK = 282,
	NYAS_KEY_PRINT_SCREEN = 283,
	NYAS_KEY_PAUSE = 284,
	NYAS_KEY_F1 = 290,
	NYAS_KEY_F2 = 291,
	NYAS_KEY_F3 = 292,
	NYAS_KEY_F4 = 293,
	NYAS_KEY_F5 = 294,
	NYAS_KEY_F6 = 295,
	NYAS_KEY_F7 = 296,
	NYAS_KEY_F8 = 297,
	NYAS_KEY_F9 = 298,
	NYAS_KEY_F10 = 299,
	NYAS_KEY_F11 = 300,
	NYAS_KEY_F12 = 301,
	NYAS_KEY_F13 = 302,
	NYAS_KEY_F14 = 303,
	NYAS_KEY_F15 = 304,
	NYAS_KEY_F16 = 305,
	NYAS_KEY_F17 = 306,
	NYAS_KEY_F18 = 307,
	NYAS_KEY_F19 = 308,
	NYAS_KEY_F20 = 309,
	NYAS_KEY_F21 = 310,
	NYAS_KEY_F22 = 311,
	NYAS_KEY_F23 = 312,
	NYAS_KEY_F24 = 313,
	NYAS_KEY_F25 = 314,
	NYAS_KEY_KP_0 = 320,
	NYAS_KEY_KP_1 = 321,
	NYAS_KEY_KP_2 = 322,
	NYAS_KEY_KP_3 = 323,
	NYAS_KEY_KP_4 = 324,
	NYAS_KEY_KP_5 = 325,
	NYAS_KEY_KP_6 = 326,
	NYAS_KEY_KP_7 = 327,
	NYAS_KEY_KP_8 = 328,
	NYAS_KEY_KP_9 = 329,
	NYAS_KEY_KP_DECIMAL = 330,
	NYAS_KEY_KP_DIVIDE = 331,
	NYAS_KEY_KP_MULTIPLY = 332,
	NYAS_KEY_KP_SUBTRACT = 333,
	NYAS_KEY_KP_ADD = 334,
	NYAS_KEY_KP_ENTER = 335,
	NYAS_KEY_KP_EQUAL = 336,
	NYAS_KEY_LEFT_SHIFT = 340,
	NYAS_KEY_LEFT_CONTROL = 341,
	NYAS_KEY_LEFT_ALT = 342,
	NYAS_KEY_LEFT_SUPER = 343,
	NYAS_KEY_RIGHT_SHIFT = 344,
	NYAS_KEY_RIGHT_CONTROL = 345,
	NYAS_KEY_RIGHT_ALT = 346,
	NYAS_KEY_RIGHT_SUPER = 347,
	NYAS_KEY_MENU = 348
};

struct nyas_io {
	nyas_keystate keys[NYAS_KEY_MENU + 1];
	nyas_keystate mouse_button[3];
	void *internal_window;
	struct nyas_point window_size; /* In pixels */
	struct nyas_vec2 mouse_pos;
	struct nyas_vec2 mouse_scroll; /* x horizontal, y vertical */
	bool window_closed;
	bool window_hovered;
	bool window_focused;
	bool show_cursor;
	bool capture_mouse;
	bool capture_keyboard;
};

extern struct nyas_io *nyas_io;

bool nyas_io_init(const char *title, struct nyas_point window_size);
void nyas_io_poll(void);
void nyas_window_swap(void);
int nyas_file_read(const char *path, char **dst, size_t *size);

#endif // NYAS_CORE_IO_H
