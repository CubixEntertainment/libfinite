#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <cairo/cairo.h>
#include "xdg-shell-client-protocol.h" // from wayland scanner
#include "layer-shell-client-protocol.h" // from wayland scanner

typedef struct FiniteShell FiniteShell;

typedef struct FiniteBtn FiniteBtn;

typedef enum {
    FINITE_DIRECTION_UP,
    FINITE_DIRECTION_DOWN,
    FINITE_DIRECTION_LEFT,
    FINITE_DIRECTION_RIGHT,
    FINITE_DIRECTION_LEFT_UP,
    FINITE_DIRECTION_LEFT_DOWN,
    FINITE_DIRECTION_RIGHT_UP,
    FINITE_DIRECTION_RIGHT_DOWN
} FiniteDirectionType;

typedef struct {
    int left;
    int right;
    int down;
    int up;
    // optional diagonals
    int leftUp;
    int rightUp;
    int leftDown;
    int rightDown;
} FiniteBtnRelationShips;

struct FiniteBtn {
    FiniteBtn *self;
    bool isActive;
    int id;
    FiniteBtnRelationShips *relations;
    void *data;
    FiniteShell *link;
    void (*on_select_callback)(FiniteBtn *self, int id, void *data);
    void (*on_focus_callback)(FiniteBtn *self, int id, void *data);
    void (*on_unfocus_callback)(FiniteBtn *self, int id, void *data);
};

/*
    # FiniteWindowInfo

    FiniteWindowInfo refers to specific rendering details based on size and position

    @param output The associated wl_output.
    @param xPos Defaults to 0
    @param yPos Defaults to 0
    @param width Defaults to the screen's resolution as windows are expected to be fullscreen
    @param height Defaults to the screen's resolution as windows are expected to be fullscreen

*/
typedef struct {
    struct wl_output *output;
    int32_t xPos;
    int32_t yPos;
    int32_t width;
    int32_t height;
} FiniteWindowInfo;


/*
    # FiniteShell

    FiniteShell refers to a single window instance.

    
    @param display The assoicated wl_display of the window. By default this is the display of "wayland-0" which is the expected in production.
    @param registry The assoicated wl_registry of the window. The registry itself is relatively worthless to non-power users but is used for cleanup purposes.
    @param output The associated wl_output of the window. In an Islands environment it should be the first available window.
    @param shm The associated shared memory buffer of the window provided by wayland. The wl_shm struct is worthless to non-power users and is included for cleanup purposes
    @param pool The shared memory pool used to draw things to the screen. Should be the result of either finite_shm_create_pool or as a power user, wl_shm_create_pool from wayland.
    @param shm_fd The file pointer to a shared memory buffer. This should be defined to the return value of `finite_shm_allocate_shm_file`
    @param pool_size The size of the data pool the memory buffer can use. This should be used as the param for `finite_shm_allocate_shm_file`
    @param pool_data The mmaped memory of the file buffer.
    @param buffer The shared memory buffer that is attached to the surface. Should only be set after drawing is complete.
    @param isle The Islands compositor instance. This value is worthless to non-power users and is included for clean up purposes.
    @param base The xdg_wm_base struct used to get and set information about the window.
    @param surface The xdg_surface of the window that provides thw window with a space to be drawn to.
    @param window The actual window itself.
    @param details A FiniteWindowInfo storing rendering information.
    @param cairo_surface A cairo_surface where things are drawn to.
*/
struct FiniteShell{
    int shm_fd;
    int pool_size;
    int stride;
    uint8_t *pool_data;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_output *output;
    struct wl_shm *shm;
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    struct wl_compositor *isle;
    struct wl_surface *isle_surface;
    struct xdg_wm_base *base;
    struct xdg_surface *surface;
    struct xdg_toplevel *window;
    struct zwlr_layer_shell_v1 *shell;
    struct zwlr_layer_surface_v1 *layer_surface;
    FiniteWindowInfo *details;
    cairo_t *cr;
    cairo_surface_t *cairo_surface;
    unsigned char *snapshot; // refers to a single item

    // an array of buttons that we can navigate through
    FiniteBtn **btns;
    int _btns; // _ vars are indexes
    int activeButton;

    // any extra data which you can set yourself
    void *data;
};

void islands_registry_handle(void *data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
void islands_registry_handle_remove(void *data, struct wl_registry* registry, uint32_t id);
void islands_output_handle(void *data, struct wl_output *output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform);
void islands_mode_handle(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
void islands_scale_handle(void *data, struct wl_output *wl_output, int32_t factor);
void islands_done_handle(void *data, struct wl_output *wl_output);
void window_configure_handle(void *data,struct xdg_toplevel *xdg_toplevel,int32_t width,int32_t height,struct wl_array *states);
void surface_configure_handle(void *data, struct xdg_surface *xdg_surface, uint32_t serial);
void window_close_handle(void *data, struct xdg_toplevel *xdg_toplevel);
void window_bounds_handle(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height);
void window_capable_handle(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities);

#define finite_shell_init(device) finite_shell_init_debug(__FILE__, __func__, __LINE__, device)
FiniteShell *finite_shell_init_debug(const char *file, const char *func, int line, char *device);

#define finite_window_init(shell) finite_window_init_debug(__FILE__, __func__, __LINE__, shell)
void finite_window_init_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_overlay_init(shell, layer, name) finite_overlay_init_debug(__FILE__, __func__, __LINE__, shell, layer, name)
void finite_overlay_init_debug(const char *file, const char *func, int line, FiniteShell *shell, int layer, char *name);

#define finite_window_size_set(shell, xPos, yPos, width, height) finite_window_size_set_debug(__FILE__, __func__, __LINE__, shell, xPos, yPos, width, height)
void finite_window_size_set_debug(const char *file, const char *func, int line, FiniteShell *shell, int xPos, int yPos, int width, int height);

#define finite_draw_finish(shell, width, height, stride, withAlpha) finite_draw_finish_debug(__FILE__, __func__, __LINE__, shell, width, height, stride, withAlpha)
bool finite_draw_finish_debug(const char *file, const char *func, int line, FiniteShell *shell, int width, int height, int stride, bool withAlpha);

#define finite_button_create(shell, on_select_callback, on_focus_callback, on_unfocus_callback, data) finite_button_create_debug(__FILE__, __func__, __LINE__, shell, on_select_callback, on_focus_callback, on_unfocus_callback, data)
FiniteBtn *finite_button_create_debug(const char *file, const char *func, int line, FiniteShell *shell, void (*on_select_callback)(FiniteBtn *self, int id, void *data), void (*on_focus_callback)(FiniteBtn *self, int id, void *data), void (*on_unfocus_callback)(FiniteBtn *self, int id, void *data), void *data);

#define finite_button_create_relation(shell, btn, dir, relation) finite_button_create_relation_debug(__FILE__, __func__, __LINE__, shell, btn, dir, relation)
void finite_button_create_relation_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteBtn *btn, FiniteDirectionType dir, int relation);

#define finite_button_delete(shell, id) finite_button_delete_debug(__FILE__, __func__, __LINE__, shell, id)
void finite_button_delete_debug(const char *file, const char *func, int line, FiniteShell *shell, int id);

#define finite_button_delete_all(shell) finite_button_delete_all_debug(__FILE__, __func__, __LINE__, shell)
void finite_button_delete_all_debug(const char *file, const char *func, int line, FiniteShell *shell);

#endif