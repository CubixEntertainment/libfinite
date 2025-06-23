#ifndef __INPUT_CORE_H__
#define __INPUT_CORE_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libevdev/libevdev.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

enum FiniteControllerType {
    FINITE_CONTROLLER_MAIN,
    FINITE_CONTROLLER_OTHER
}; 

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *isle;
    struct wl_seat *seat;
} FiniteInput;

typedef struct {
    uint16_t id;
    bool isHeld;
    bool isDown;
    bool isUp;
} FiniteKeyState;

typedef struct {
    FiniteInput *input;

    struct wl_keyboard *keyboard;
    struct xkb_context *xkb_ctx;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state  *xkb_state;

    FiniteKeyState keys[256];
    bool keys_down[256];
    bool keys_up[256];
} FiniteKeyboard;

typedef struct {
    FiniteInput *input;

    int ctrl_fd; // controller file
    int ctrl_rc; // libevdev referene to the file and device as a pair
    struct libevdev *device; // the libevdev device

    double lAxis;
    double rAxis;

    bool btns[12]; // temp
    bool btns_down[12]; // temp
    bool btns_up[12]; // temp

} FiniteGamepad;

void islands_key_registry_handle(void *data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
void islands_key_registry_handle_remove(void *data, struct wl_registry* registry, uint32_t id);
void islands_key_map_handle(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
void islands_key_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void islands_key_mod_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void islands_key_enter_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
void islands_key_leave_handler(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);

#endif