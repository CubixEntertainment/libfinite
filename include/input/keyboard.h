#ifndef __IKEYBOARD_H__
#define __IKEYBOARD_H__

#include <xkbcommon/xkbcommon.h>
#include <strings.h>
#include <linux/input-event-codes.h>
#include "input-core.h"

// FiniteShell should be defined by window.h
typedef struct FiniteShell FiniteShell;

// this is what AI should be used for
typedef enum {
    // Letters
    FINITE_KEY_A,
    FINITE_KEY_B,
    FINITE_KEY_C,
    FINITE_KEY_D,
    FINITE_KEY_E,
    FINITE_KEY_F,
    FINITE_KEY_G,
    FINITE_KEY_H,
    FINITE_KEY_I,
    FINITE_KEY_J,
    FINITE_KEY_K,
    FINITE_KEY_L,
    FINITE_KEY_M,
    FINITE_KEY_N,
    FINITE_KEY_O,
    FINITE_KEY_P,
    FINITE_KEY_Q,
    FINITE_KEY_R,
    FINITE_KEY_S,
    FINITE_KEY_T,
    FINITE_KEY_U,
    FINITE_KEY_V,
    FINITE_KEY_W,
    FINITE_KEY_X,
    FINITE_KEY_Y,
    FINITE_KEY_Z,

    // Numbers
    FINITE_KEY_0,
    FINITE_KEY_1,
    FINITE_KEY_2,
    FINITE_KEY_3,
    FINITE_KEY_4,
    FINITE_KEY_5,
    FINITE_KEY_6,
    FINITE_KEY_7,
    FINITE_KEY_8,
    FINITE_KEY_9,

    // Modifiers
    FINITE_KEY_LEFT_SHIFT,
    FINITE_KEY_RIGHT_SHIFT,
    FINITE_KEY_LEFT_CTRL,
    FINITE_KEY_RIGHT_CTRL,
    FINITE_KEY_LEFT_ALT,
    FINITE_KEY_RIGHT_ALT,
    FINITE_KEY_LEFT_META,
    FINITE_KEY_RIGHT_META,

    // Function Keys
    FINITE_KEY_F1,
    FINITE_KEY_F2,
    FINITE_KEY_F3,
    FINITE_KEY_F4,
    FINITE_KEY_F5,
    FINITE_KEY_F6,
    FINITE_KEY_F7,
    FINITE_KEY_F8,
    FINITE_KEY_F9,
    FINITE_KEY_F10,
    FINITE_KEY_F11,
    FINITE_KEY_F12,

    // Arrows
    FINITE_KEY_UP,
    FINITE_KEY_DOWN,
    FINITE_KEY_LEFT,
    FINITE_KEY_RIGHT,

    // Other Keys
    FINITE_KEY_SPACE,
    FINITE_KEY_ENTER,
    FINITE_KEY_ESCAPE,
    FINITE_KEY_TAB,
    FINITE_KEY_BACKSPACE,
    FINITE_KEY_CAPS_LOCK,

    // Symbols and Punctuation
    FINITE_KEY_MINUS,
    FINITE_KEY_EQUALS,
    FINITE_KEY_LEFT_BRACKET,
    FINITE_KEY_RIGHT_BRACKET,
    FINITE_KEY_BACKSLASH,
    FINITE_KEY_SEMICOLON,
    FINITE_KEY_APOSTROPHE,
    FINITE_KEY_GRAVE,
    FINITE_KEY_COMMA,
    FINITE_KEY_PERIOD,
    FINITE_KEY_SLASH,

    // Meta
    FINITE_KEY_PRINT_SCREEN,
    FINITE_KEY_SCROLL_LOCK,
    FINITE_KEY_PAUSE,
    FINITE_KEY_INSERT,
    FINITE_KEY_DELETE,
    FINITE_KEY_HOME,
    FINITE_KEY_END,
    FINITE_KEY_PAGE_UP,
    FINITE_KEY_PAGE_DOWN,

    // NumPad
    FINITE_KEY_NUM_LOCK,
    FINITE_KEY_KP_DIVIDE,
    FINITE_KEY_KP_MULTIPLY,
    FINITE_KEY_KP_MINUS,
    FINITE_KEY_KP_PLUS,
    FINITE_KEY_KP_ENTER,
    FINITE_KEY_KP_0,
    FINITE_KEY_KP_1,
    FINITE_KEY_KP_2,
    FINITE_KEY_KP_3,
    FINITE_KEY_KP_4,
    FINITE_KEY_KP_5,
    FINITE_KEY_KP_6,
    FINITE_KEY_KP_7,
    FINITE_KEY_KP_8,
    FINITE_KEY_KP_9,
    FINITE_KEY_KP_PERIOD,

    // Sentinel
    FINITE_KEY_INVALID,
    FINITE_KEY_COUNT
} FiniteKey;


typedef struct {
    const char *name;
    FiniteKey key;
    uint16_t evdev_code;
} FiniteKeyMapping;

extern const FiniteKeyMapping finite_key_lookup[];

#define finite_input_keyboard_init(device) finite_input_keyboard_init_debug(__FILE__, __func__, __LINE__, device)
FiniteKeyboard *finite_input_keyboard_init_debug(const char *file, const char *func, int line, struct wl_display *device);

#define finite_key_down(key, board) finite_key_down_debug(__FILE__, __func__, __LINE__, key, board)
bool finite_key_down_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board);

#define finite_key_up(key, board) finite_key_up_debug(__FILE__, __func__, __LINE__, key, board)
bool finite_key_up_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board);

#define finite_key_valid(key) finite_key_valid_debug(__FILE__, __func__, __LINE__, key)
bool finite_key_valid_debug(const char *file, const char *func, int line, FiniteKey key);

#define finite_key_pressed(key, board) finite_key_pressed_debug(__FILE__, __func__, __LINE__, key, board)
bool finite_key_pressed_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board);

// conversion
#define finite_key_string_from_key(key) finite_key_string_from_key_debug(__FILE__, __func__, __LINE__, key)
const char *finite_key_string_from_key_debug(const char *file, const char *func, int line, FiniteKey key);

#define finite_key_from_string(name) finite_key_from_string_debug(__FILE__, __func__, __LINE__, name)
FiniteKey finite_key_from_string_debug(const char *file, const char *func, int line, const char *name);

// lifecycle
#define finite_keyboard_destroy(board) finite_keyboard_destroy_debug(__FILE__, __func__, __LINE__, board)
void finite_keyboard_destroy_debug(const char *file, const char *func, int line, FiniteKeyboard *board);

#define finite_input_poll_keys(board, shell) finite_input_poll_keys_debug(__FILE__, __func__, __LINE__, board, shell)
void finite_input_poll_keys_debug(const char *file, const char *func, int line, FiniteKeyboard *board, FiniteShell *shell);

#endif