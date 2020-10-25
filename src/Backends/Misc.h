// Released under the MIT licence.
// See LICENCE.txt for details.

#pragma once

#include <stddef.h>
#include <string>

#include "../Attributes.h"

enum
{
	// Based on US QWERTY
	BACKEND_KEYBOARD_A,
	BACKEND_KEYBOARD_B,
	BACKEND_KEYBOARD_C,
	BACKEND_KEYBOARD_D,
	BACKEND_KEYBOARD_E,
	BACKEND_KEYBOARD_F,
	BACKEND_KEYBOARD_G,
	BACKEND_KEYBOARD_H,
	BACKEND_KEYBOARD_I,
	BACKEND_KEYBOARD_J,
	BACKEND_KEYBOARD_K,
	BACKEND_KEYBOARD_L,
	BACKEND_KEYBOARD_M,
	BACKEND_KEYBOARD_N,
	BACKEND_KEYBOARD_O,
	BACKEND_KEYBOARD_P,
	BACKEND_KEYBOARD_Q,
	BACKEND_KEYBOARD_R,
	BACKEND_KEYBOARD_S,
	BACKEND_KEYBOARD_T,
	BACKEND_KEYBOARD_U,
	BACKEND_KEYBOARD_V,
	BACKEND_KEYBOARD_W,
	BACKEND_KEYBOARD_X,
	BACKEND_KEYBOARD_Y,
	BACKEND_KEYBOARD_Z,
	BACKEND_KEYBOARD_0,
	BACKEND_KEYBOARD_1,
	BACKEND_KEYBOARD_2,
	BACKEND_KEYBOARD_3,
	BACKEND_KEYBOARD_4,
	BACKEND_KEYBOARD_5,
	BACKEND_KEYBOARD_6,
	BACKEND_KEYBOARD_7,
	BACKEND_KEYBOARD_8,
	BACKEND_KEYBOARD_9,
	BACKEND_KEYBOARD_F1,
	BACKEND_KEYBOARD_F2,
	BACKEND_KEYBOARD_F3,
	BACKEND_KEYBOARD_F4,
	BACKEND_KEYBOARD_F5,
	BACKEND_KEYBOARD_F6,
	BACKEND_KEYBOARD_F7,
	BACKEND_KEYBOARD_F8,
	BACKEND_KEYBOARD_F9,
	BACKEND_KEYBOARD_F10,
	BACKEND_KEYBOARD_F11,
	BACKEND_KEYBOARD_F12,
	BACKEND_KEYBOARD_UP,
	BACKEND_KEYBOARD_DOWN,
	BACKEND_KEYBOARD_LEFT,
	BACKEND_KEYBOARD_RIGHT,
	BACKEND_KEYBOARD_ESCAPE,
	BACKEND_KEYBOARD_BACK_QUOTE,
	BACKEND_KEYBOARD_TAB,
	BACKEND_KEYBOARD_CAPS_LOCK,
	BACKEND_KEYBOARD_LEFT_SHIFT,
	BACKEND_KEYBOARD_LEFT_CTRL,
	BACKEND_KEYBOARD_LEFT_ALT,
	BACKEND_KEYBOARD_SPACE,
	BACKEND_KEYBOARD_RIGHT_ALT,
	BACKEND_KEYBOARD_RIGHT_CTRL,
	BACKEND_KEYBOARD_RIGHT_SHIFT,
	BACKEND_KEYBOARD_ENTER,
	BACKEND_KEYBOARD_BACKSPACE,
	BACKEND_KEYBOARD_MINUS,
	BACKEND_KEYBOARD_EQUALS,
	BACKEND_KEYBOARD_LEFT_BRACKET,
	BACKEND_KEYBOARD_RIGHT_BRACKET,
	BACKEND_KEYBOARD_BACK_SLASH,
	BACKEND_KEYBOARD_SEMICOLON,
	BACKEND_KEYBOARD_APOSTROPHE,
	BACKEND_KEYBOARD_COMMA,
	BACKEND_KEYBOARD_PERIOD,
	BACKEND_KEYBOARD_FORWARD_SLASH,
	BACKEND_KEYBOARD_TOTAL
};

typedef struct Backend_DisplayMode
{
	unsigned int width;
	unsigned int height;
	unsigned int refresh_rate;
} Backend_DisplayMode;

bool Backend_Init(void (*drag_and_drop_callback)(const char *path), void (*window_focus_callback)(bool focus));
void Backend_Deinit(void);
void Backend_PostWindowCreation(void);
bool Backend_GetPaths(std::string *module_path, std::string *data_path);
void Backend_HideMouse(void);
void Backend_SetWindowIcon(const unsigned char *rgb_pixels, size_t width, size_t height);
void Backend_SetCursor(const unsigned char *rgba_pixels, size_t width, size_t height);
void Backend_EnableDragAndDrop(void);
bool Backend_SystemTask(bool active);
void Backend_GetKeyboardState(bool *keyboard_state);
void Backend_ShowMessageBox(const char *title, const char *message);
ATTRIBUTE_FORMAT_PRINTF(1, 2) void Backend_PrintError(const char *format, ...);
ATTRIBUTE_FORMAT_PRINTF(1, 2) void Backend_PrintInfo(const char *format, ...);
unsigned long Backend_GetTicks(void);
void Backend_Delay(unsigned int ticks);
void Backend_GetDisplayMode(Backend_DisplayMode *display_mode);
bool Backend_IsConsole(void);
