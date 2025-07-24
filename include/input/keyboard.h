#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

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

const FiniteKeyMapping finite_key_lookup[] = {
    // Letters
    { "A", FINITE_KEY_A, KEY_A }, { "B", FINITE_KEY_B, KEY_B }, { "C", FINITE_KEY_C, KEY_C },
    { "D", FINITE_KEY_D, KEY_D }, { "E", FINITE_KEY_E, KEY_E }, { "F", FINITE_KEY_F, KEY_F },
    { "G", FINITE_KEY_G, KEY_G }, { "H", FINITE_KEY_H, KEY_H }, { "I", FINITE_KEY_I, KEY_I },
    { "J", FINITE_KEY_J, KEY_J }, { "K", FINITE_KEY_K, KEY_K }, { "L", FINITE_KEY_L, KEY_L },
    { "M", FINITE_KEY_M, KEY_M }, { "N", FINITE_KEY_N, KEY_N }, { "O", FINITE_KEY_O, KEY_O },
    { "P", FINITE_KEY_P, KEY_P }, { "Q", FINITE_KEY_Q, KEY_Q }, { "R", FINITE_KEY_R, KEY_R },
    { "S", FINITE_KEY_S, KEY_S }, { "T", FINITE_KEY_T, KEY_T }, { "U", FINITE_KEY_U, KEY_U },
    { "V", FINITE_KEY_V, KEY_V }, { "W", FINITE_KEY_W, KEY_W }, { "X", FINITE_KEY_X, KEY_X },
    { "Y", FINITE_KEY_Y, KEY_Y }, { "Z", FINITE_KEY_Z, KEY_Z },

    // Digits
    { "0", FINITE_KEY_0, KEY_0 }, { "1", FINITE_KEY_1, KEY_1 }, { "2", FINITE_KEY_2,KEY_2 },
    { "3", FINITE_KEY_3, KEY_3 }, { "4", FINITE_KEY_4, KEY_4 }, { "5", FINITE_KEY_5, KEY_5 },
    { "6", FINITE_KEY_6, KEY_6 }, { "7", FINITE_KEY_7, KEY_7 }, { "8", FINITE_KEY_8, KEY_8 },
    { "9", FINITE_KEY_9, KEY_9 },

    // Modifiers
    { "LeftShift",  FINITE_KEY_LEFT_SHIFT, KEY_LEFTSHIFT },   { "RightShift", FINITE_KEY_RIGHT_SHIFT, KEY_RIGHTSHIFT },
    { "LeftCtrl",   FINITE_KEY_LEFT_CTRL, KEY_LEFTCTRL },    { "RightCtrl",  FINITE_KEY_RIGHT_CTRL, KEY_RIGHTCTRL },
    { "LeftAlt",    FINITE_KEY_LEFT_ALT, KEY_LEFTALT },     { "RightAlt",   FINITE_KEY_RIGHT_ALT, KEY_RIGHTALT },
    { "LeftMeta",   FINITE_KEY_LEFT_META, KEY_LEFTMETA },    { "RightMeta",  FINITE_KEY_RIGHT_META, KEY_RIGHTMETA },

    // Arrows
    { "Up",    FINITE_KEY_UP, KEY_UP },
    { "Down",  FINITE_KEY_DOWN, KEY_DOWN },
    { "Left",  FINITE_KEY_LEFT, KEY_LEFT },
    { "Right", FINITE_KEY_RIGHT, KEY_RIGHT },

    // Function keys
    { "F1", FINITE_KEY_F1, KEY_F1 }, { "F2", FINITE_KEY_F2, KEY_F2 }, { "F3", FINITE_KEY_F3, KEY_F3 },
    { "F4", FINITE_KEY_F4, KEY_F4 }, { "F5", FINITE_KEY_F5, KEY_F5 }, { "F6", FINITE_KEY_F6, KEY_F6 },
    { "F7", FINITE_KEY_F7, KEY_F7 }, { "F8", FINITE_KEY_F8, KEY_F8 }, { "F9", FINITE_KEY_F9, KEY_F9 },
    { "F10", FINITE_KEY_F10, KEY_F10 }, { "F11", FINITE_KEY_F11, KEY_F11 }, { "F12", FINITE_KEY_F12, KEY_F12 },

    // Editing / navigation
    { "Enter",      FINITE_KEY_ENTER, KEY_ENTER },
    { "Escape",     FINITE_KEY_ESCAPE, KEY_ESC },
    { "Backspace",  FINITE_KEY_BACKSPACE, KEY_BACKSPACE },
    { "Tab",        FINITE_KEY_TAB, KEY_TAB },
    { "Space",      FINITE_KEY_SPACE, KEY_SPACE },
    { "CapsLock",   FINITE_KEY_CAPS_LOCK, KEY_CAPSLOCK },
    { "Insert",     FINITE_KEY_INSERT, KEY_INSERT },
    { "Delete",     FINITE_KEY_DELETE, KEY_DELETE },
    { "Home",       FINITE_KEY_HOME, KEY_HOME },
    { "End",        FINITE_KEY_END, KEY_END },
    { "PageUp",     FINITE_KEY_PAGE_UP, KEY_PAGEUP },
    { "PageDown",   FINITE_KEY_PAGE_DOWN, KEY_PAGEDOWN },

    // Meta
    { "PrintScreen", FINITE_KEY_PRINT_SCREEN, KEY_PRINT },
    { "ScrollLock",  FINITE_KEY_SCROLL_LOCK, KEY_SCROLLLOCK },
    { "Pause",       FINITE_KEY_PAUSE, KEY_PAUSE },

    // Symbols and punctuation
    { "Minus",        FINITE_KEY_MINUS, KEY_MINUS },
    { "Equals",       FINITE_KEY_EQUALS, KEY_EQUAL },
    { "LeftBracket",  FINITE_KEY_LEFT_BRACKET, KEY_LEFTBRACE },
    { "RightBracket", FINITE_KEY_RIGHT_BRACKET, KEY_RIGHTBRACE },
    { "Backslash",    FINITE_KEY_BACKSLASH, KEY_BACKSLASH },
    { "Semicolon",    FINITE_KEY_SEMICOLON, KEY_SEMICOLON },
    { "Apostrophe",   FINITE_KEY_APOSTROPHE, KEY_APOSTROPHE },
    { "Grave",        FINITE_KEY_GRAVE, KEY_GRAVE },
    { "Comma",        FINITE_KEY_COMMA, KEY_COMMA },
    { "Period",       FINITE_KEY_PERIOD, KEY_DOT },
    { "Slash",        FINITE_KEY_SLASH, KEY_SLASH },

    // Numpad
    { "NumLock",      FINITE_KEY_NUM_LOCK, KEY_NUMLOCK },
    { "KpDivide",     FINITE_KEY_KP_DIVIDE, KEY_KPSLASH },
    { "KpMultiply",   FINITE_KEY_KP_MULTIPLY, KEY_KPASTERISK },
    { "KpMinus",      FINITE_KEY_KP_MINUS, KEY_KPMINUS },
    { "KpPlus",       FINITE_KEY_KP_PLUS, KEY_KPPLUS },
    { "KpEnter",      FINITE_KEY_KP_ENTER, KEY_KPENTER },
    { "Kp0",          FINITE_KEY_KP_0, KEY_KP0 },
    { "Kp1",          FINITE_KEY_KP_1, KEY_KP1 },
    { "Kp2",          FINITE_KEY_KP_2, KEY_KP2 },
    { "Kp3",          FINITE_KEY_KP_3, KEY_KP3 },
    { "Kp4",          FINITE_KEY_KP_4, KEY_KP4 },
    { "Kp5",          FINITE_KEY_KP_5, KEY_KP5 },
    { "Kp6",          FINITE_KEY_KP_6, KEY_KP6 },
    { "Kp7",          FINITE_KEY_KP_7, KEY_KP7 },
    { "Kp8",          FINITE_KEY_KP_8, KEY_KP8 },
    { "Kp9",          FINITE_KEY_KP_9, KEY_KP9 },
    { "KpPeriod",     FINITE_KEY_KP_PERIOD, KEY_KPDOT },
};

#define finite_input_keyboard_init(device) finite_input_keyboard_init_debug(__FILE__, __func__, __LINE__, device)
FiniteKeyboard *finite_input_keyboard_init_debug(const char *file, const char *func, int line, struct wl_display *device);

#define finite_key_down(key, board) finite_key_down_debug(__FILE__, __func__, __LINE__, key, board)
bool finite_key_down(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board);

#define finite_key_up(key, board) finite_key_up_debug(__FILE__, __func__, __LINE__, key, board)
bool finite_key_up(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board);

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