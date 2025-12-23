#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct FiniteShell FiniteShell;
typedef struct FiniteGamepad FiniteGamepad;

typedef enum {
    // letters
    FINITE_BTN_A,
    FINITE_BTN_B,
    FINITE_BTN_X,
    FINITE_BTN_Y,

    //dpad
    FINITE_BTN_LEFT,
    FINITE_BTN_RIGHT,
    FINITE_BTN_DOWN,
    FINITE_BTN_UP,

    // Triggers
    FINITE_BTN_RIGHT_SHOULDER,
    FINITE_BTN_RIGHT_TRIGGER,
    FINITE_BTN_RIGHT_SPECIAL, // the r4 or rz on  some controlleres
    FINITE_BTN_LEFT_SHOULDER,
    FINITE_BTN_LEFT_TRIGGER,
    FINITE_BTN_LEFT_SPECIAL,

    // Joystick buttons
    FINITE_BTN_LEFT_JOYSTICK,
    FINITE_BTN_RIGHT_JOYSTICK,

    // Meta
    FINITE_BTN_START,
    FINITE_BTN_SELECT,
    FINITE_BTN_HOME,

    FINITE_BTN_NONE = INT16_MAX // ignore key
} FiniteGamepadKey;

typedef enum {
    FINITE_JOYSTICK_LEFT_X,
    FINITE_JOYSTICK_LEFT_Y,
    FINITE_JOYSTICK_RIGHT_X,
    FINITE_JOYSTICK_RIGHT_Y
} FiniteJoystickType;

typedef struct {
    char *name;
    uint16_t xAxis;
    double xValue; // value between -1 and 1
    int xMin;
    int xMax;
    int xFlat; // deadzone
    uint16_t yAxis;
    double yValue;
    int yMin;
    int yMax;
    int yFlat;
} FiniteJoystick;

// dpads are analog for some reason.
typedef struct {
    uint16_t xAxis;
    int xValue;
    uint16_t yAxis;
    int yValue;
} FiniteDpad;

// analog triggers
typedef struct {
    uint16_t axis;
    double value;
} FiniteTrigger;

typedef struct {
    const char *name;
    FiniteGamepadKey key;
    uint16_t evdev_code;
} FiniteGamepadKeyMapping;

extern const FiniteGamepadKeyMapping finite_gamepad_key_lookup[];

#define finite_gamepad_init(shell) finite_gamepad_init_debug(__FILE__, __func__, __LINE__, shell)
bool finite_gamepad_init_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_gamepad_key_valid(key) finite_gamepad_key_valid_debug(__FILE__, __func__, __LINE__, key)
bool finite_gamepad_key_valid_debug(const char *file, const char *func, int line, FiniteGamepadKey key);

#define finite_gamepad_key_down(id, shell, key) finite_gamepad_key_down_debug(__FILE__, __func__, __LINE__, id, shell, key)
bool finite_gamepad_key_down_debug(const char *file, const char *func, int line, int id, FiniteShell *shell, FiniteGamepadKey key);

#define finite_gamepad_key_up(id, shell, key) finite_gamepad_key_up_debug(__FILE__, __func__, __LINE__, id, shell, key)
bool finite_gamepad_key_up_debug(const char *file, const char *func, int line, int id, FiniteShell *shell,  FiniteGamepadKey key);

#define finite_gamepad_key_pressed(id, shell, key) finite_gamepad_key_pressed_debug(__FILE__, __func__, __LINE__, id, shell, key)
bool finite_gamepad_key_pressed_debug(const char *file, const char *func, int line, int id, FiniteShell *shell,  FiniteGamepadKey key);

#define finite_gamepad_key_from_string(name) finite_gamepad_key_from_string_debug(__FILE__, __func__, __LINE__, name)
FiniteGamepadKey finite_gamepad_key_from_string_debug(const char *file, const char *func, int line, const char *name);

#define finite_gamepad_key_string_from_key(key) finite_gamepad_key_string_from_key_debug(__FILE__, __func__, __LINE__, key)
const char *finite_gamepad_key_string_from_key_debug(const char *file, const char *func, int line, FiniteGamepadKey key);

#define finite_gamepad_joystick_get_value(id, shell, type) finite_gamepad_joystick_get_value_debug(__FILE__, __func__, __LINE__, id, shell, type)
double finite_gamepad_joystick_get_value_debug(const char *file, const char *func, int line, int id, FiniteShell *shell, FiniteJoystickType type);


#endif