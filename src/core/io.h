#ifndef THE_CORE_IO_H
#define THE_CORE_IO_H

#include "common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define THE_PRINT printf
#define THE__LOG_LOC THE_PRINT("%s(%u)", __FILE__, __LINE__)

#define THE_LOG(...)        \
	THE_PRINT("The [Log] ");   \
	THE__LOG_LOC;           \
	THE_PRINT(":\n\t");     \
	THE_PRINT(__VA_ARGS__); \
	THE_PRINT("\n")

#define THE_LOG_WARN(...)     \
	THE_PRINT("The [Warning] "); \
	THE__LOG_LOC;             \
	THE_PRINT(":\n\t");       \
	THE_PRINT(__VA_ARGS__);   \
	THE_PRINT("\n")

#define THE_LOG_ERR(...)    \
	THE_PRINT("The [Error] "); \
	THE__LOG_LOC;           \
	THE_PRINT(":\n\t");     \
	THE_PRINT(__VA_ARGS__); \
	THE_PRINT("\n")

#define the_elapsed(CHRONO) (the_time() - (CHRONO))
#define the_time_sec(CHRONO) ((CHRONO) / 1000000000.0f)
#define the_time_milli(CHRONO) ((CHRONO) / 1000000)
#define the_time_micro(CHRONO) ((CHRONO) / 1000)
#define the_time_nano(CHRONO) (CHRONO)

typedef int64_t the_chrono;
the_chrono the_time(void);

typedef int8_t the_keystate;
enum the_keystate {
	THE_KEYSTATE_RELEASED = 0, /* Not pressed */
	THE_KEYSTATE_DOWN = 1, /* From released to pressed this frame */
	THE_KEYSTATE_UP = 2, /* From pressed to released this frame */
	THE_KEYSTATE_PRESSED = 3, /* Pressed */
};

typedef int the_mouse_button;
enum the_mouse_button {
	THE_MOUSE_LEFT = 0,
	THE_MOUSE_RIGHT = 1,
	THE_MOUSE_MIDDLE = 2,
};

typedef int the_key;
enum the_key {
	THE_KEY_INVALID = 0,
	THE_KEY_SPACE = 32,
	THE_KEY_APOSTROPHE = 39, /* ' */
	THE_KEY_COMMA = 44,      /* , */
	THE_KEY_MINUS = 45,      /* - */
	THE_KEY_PERIOD = 46,     /* . */
	THE_KEY_SLASH = 47,      /* / */
	THE_KEY_0 = 48,
	THE_KEY_1 = 49,
	THE_KEY_2 = 50,
	THE_KEY_3 = 51,
	THE_KEY_4 = 52,
	THE_KEY_5 = 53,
	THE_KEY_6 = 54,
	THE_KEY_7 = 55,
	THE_KEY_8 = 56,
	THE_KEY_9 = 57,
	THE_KEY_SEMICOLON = 59, /* ; */
	THE_KEY_EQUAL = 61,     /* = */
	THE_KEY_A = 65,
	THE_KEY_B = 66,
	THE_KEY_C = 67,
	THE_KEY_D = 68,
	THE_KEY_E = 69,
	THE_KEY_F = 70,
	THE_KEY_G = 71,
	THE_KEY_H = 72,
	THE_KEY_I = 73,
	THE_KEY_J = 74,
	THE_KEY_K = 75,
	THE_KEY_L = 76,
	THE_KEY_M = 77,
	THE_KEY_N = 78,
	THE_KEY_O = 79,
	THE_KEY_P = 80,
	THE_KEY_Q = 81,
	THE_KEY_R = 82,
	THE_KEY_S = 83,
	THE_KEY_T = 84,
	THE_KEY_U = 85,
	THE_KEY_V = 86,
	THE_KEY_W = 87,
	THE_KEY_X = 88,
	THE_KEY_Y = 89,
	THE_KEY_Z = 90,
	THE_KEY_LEFT_BRACKET = 91,  /* [ */
	THE_KEY_BACKSLASH = 92,     /* \ */
	THE_KEY_RIGHT_BRACKET = 93, /* ] */
	THE_KEY_GRAVE_ACCENT = 96,  /* ` */
	THE_KEY_WORLD_1 = 161,      /* non-US #1 */
	THE_KEY_WORLD_2 = 162,      /* non-US #2 */
	THE_KEY_ESCAPE = 256,
	THE_KEY_ENTER = 257,
	THE_KEY_TAB = 258,
	THE_KEY_BACKSPACE = 259,
	THE_KEY_INSERT = 260,
	THE_KEY_DELETE = 261,
	THE_KEY_RIGHT = 262,
	THE_KEY_LEFT = 263,
	THE_KEY_DOWN = 264,
	THE_KEY_UP = 265,
	THE_KEY_PAGE_UP = 266,
	THE_KEY_PAGE_DOWN = 267,
	THE_KEY_HOME = 268,
	THE_KEY_END = 269,
	THE_KEY_CAPS_LOCK = 280,
	THE_KEY_SCROLL_LOCK = 281,
	THE_KEY_NUM_LOCK = 282,
	THE_KEY_PRINT_SCREEN = 283,
	THE_KEY_PAUSE = 284,
	THE_KEY_F1 = 290,
	THE_KEY_F2 = 291,
	THE_KEY_F3 = 292,
	THE_KEY_F4 = 293,
	THE_KEY_F5 = 294,
	THE_KEY_F6 = 295,
	THE_KEY_F7 = 296,
	THE_KEY_F8 = 297,
	THE_KEY_F9 = 298,
	THE_KEY_F10 = 299,
	THE_KEY_F11 = 300,
	THE_KEY_F12 = 301,
	THE_KEY_F13 = 302,
	THE_KEY_F14 = 303,
	THE_KEY_F15 = 304,
	THE_KEY_F16 = 305,
	THE_KEY_F17 = 306,
	THE_KEY_F18 = 307,
	THE_KEY_F19 = 308,
	THE_KEY_F20 = 309,
	THE_KEY_F21 = 310,
	THE_KEY_F22 = 311,
	THE_KEY_F23 = 312,
	THE_KEY_F24 = 313,
	THE_KEY_F25 = 314,
	THE_KEY_KP_0 = 320,
	THE_KEY_KP_1 = 321,
	THE_KEY_KP_2 = 322,
	THE_KEY_KP_3 = 323,
	THE_KEY_KP_4 = 324,
	THE_KEY_KP_5 = 325,
	THE_KEY_KP_6 = 326,
	THE_KEY_KP_7 = 327,
	THE_KEY_KP_8 = 328,
	THE_KEY_KP_9 = 329,
	THE_KEY_KP_DECIMAL = 330,
	THE_KEY_KP_DIVIDE = 331,
	THE_KEY_KP_MULTIPLY = 332,
	THE_KEY_KP_SUBTRACT = 333,
	THE_KEY_KP_ADD = 334,
	THE_KEY_KP_ENTER = 335,
	THE_KEY_KP_EQUAL = 336,
	THE_KEY_LEFT_SHIFT = 340,
	THE_KEY_LEFT_CONTROL = 341,
	THE_KEY_LEFT_ALT = 342,
	THE_KEY_LEFT_SUPER = 343,
	THE_KEY_RIGHT_SHIFT = 344,
	THE_KEY_RIGHT_CONTROL = 345,
	THE_KEY_RIGHT_ALT = 346,
	THE_KEY_RIGHT_SUPER = 347,
	THE_KEY_MENU = 348
};

struct the_io {
	the_keystate keys[THE_KEY_MENU + 1];
	the_keystate mouse_button[3];
	void *internal_window;
	struct the_point window_size; /* In pixels */
	struct the_vec2 mouse_pos;
	struct the_vec2 mouse_scroll; /* x horizontal, y vertical */
	bool window_closed;
	bool window_hovered;
	bool window_focused;
	bool show_cursor;
	bool capture_mouse;
	bool capture_keyboard;
};

extern struct the_io *the_io;

bool the_io_init(const char *title, struct the_point window_size);
void the_io_poll(void);
void the_window_swap(void);
int the_file_read(const char *path, char **dst, size_t *size);

#endif // THE_CORE_IO_H
