#include "../include/draw/cairo.h"
#include "../include/log.h"
#include <unistd.h>

# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */

/*
    # finite_draw_rounded_rect

    Attempts to draw a rounded box.
*/
void finite_draw_rounded_rect_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, double radius, FiniteColorGroup *color, cairo_pattern_t *pat, bool withPreserve) {
    double r = radius;
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw a rounded rect with NULL shell.");
        return;
    }

    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    if (color && pat) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_rounded_rect() may not have multiple fill values.");
        return;
    }

    // create the rounded box
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - r, y + r,     r, -M_PI_2, 0);
    cairo_arc(cr, x + width - r, y + height - r, r, 0, M_PI_2);
    cairo_arc(cr, x + r,     y + height - r, r, M_PI_2, M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI, 3 * M_PI_2);
    cairo_close_path(cr);

    if (pat) {
        cairo_set_source(cr, pat);
    } else {
        if (color) {
            if (color->a) {
                cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);
            } else {
                cairo_set_source_rgb(cr, color->r, color->g, color->b);
            }

            if (withPreserve) {
                cairo_fill_preserve(cr);
            } else {
                cairo_fill(cr);
            }
        } else {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Unable to fill. (Did you intend to not define a color or a pattern?)");
        }
    }
}


// TODO: finite_draw_glow_gradient
void finite_draw_glow_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, double radius, int layers, FiniteColorGroup *color, bool withPreserve) {
        if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw a glow rect with NULL shell.");
        return;
    }
    
    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }
    
    for (int i = layers; i > 0; i--) {
        double alpha = 1 - (0.15 * i / layers);  // Decreasing opacity
        double grow = i;  // Expand outward

        FiniteColorGroup color2 = {
            .r = color->r,
            .g = color->g,
            .b = color->b,
            .a = alpha
        };
        

        finite_draw_rounded_rect(shell, x - grow, y - grow, width + grow * 2, height + grow * 2, radius + grow, &color2, NULL, withPreserve);
    }
}

void finite_draw_stroke_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteColorGroup *color, cairo_pattern_t *pat, int width) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw a rect stroke with NULL shell.");
        return;
    }
    
    // if shell.cr doesn't exist throw an error because we can't set a stroke to nothing
    if (!shell->cr) {
       finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_stroke() may not be the first use of shell.cr. To set a stroke create a rect first.");
    }

    cairo_t *cr = shell->cr;

    if (color && pat) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_stroke() may not have multiple fill values.");
        return;
    }

    if (pat) {
        cairo_set_source(cr, pat);
    } else {
        if (color) {
            if (color->a) {
                cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);
            } else {
                cairo_set_source_rgb(cr, color->r, color->g, color->b);
            }
        } else {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to fill. (Did you define a color or a pattern?)");
        }
    }

    cairo_set_line_width(cr, width);

    cairo_stroke(cr);
}

void finite_draw_set_offset_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not offset with NULL Shell.");
        return;
    }

    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_translate(cr, x, y);
}

void finite_draw_reset_offset_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y) {
    // math is hard
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not reset offset with NULL Shell.");
        return;
    }

    if (!shell->cr) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not reset offset with NULL cr.");
        return;
    }

    cairo_t *cr = shell->cr;


    cairo_translate(cr, -x, -y);
}

/*
    # finite_draw_set_font

    Sets the font that will be used when drawing to text to the window.
*/
void finite_draw_set_font_debug(const char *file, const char *func, int line, FiniteShell *shell, char *font_name, bool isItalics, bool isBold, int size) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw text with NULL Shell.");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    enum _cairo_font_slant slant;
    enum _cairo_font_weight bold;

    slant = (isItalics) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
    bold = (isBold) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

    cairo_select_font_face(cr, font_name, slant, bold);
    cairo_set_font_size(cr, size);
}

/*
    # finite_draw_set_text

    Draw text to the screen. 
    
    @note `finite_draw_set_font()` should be called before drawing text for the first time.
*/
void finite_draw_set_text_debug(const char *file, const char *func, int line, FiniteShell *shell, char *text, FiniteColorGroup *color) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw text with NULL Shell.");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_set_source_rgb(cr, color->r, color->g, color->b);
    if (color->a) {
        cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);
    }
    cairo_show_text(cr, text);
}

/*
    # finite_draw_set_draw_position

    Change the position of where to draw things to.

    @note x and y are relative to the window
*/
void finite_draw_set_draw_position_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not set draw position with NULL Shell.");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_move_to(cr, x, y);
}

/*
    # finite_draw_text_group

    A `FiniteTextGroup` is a group of text objects meant to be rendered inline with various colors or font effects.

    @param shell The assoicated shell.
    @param groups A pointer to an array of `FiniteTextGroup`s
    @param x,y The x and Y positions of where the text should start rendering. It ignore any value set by `finite_draw_set_draw_position()`

    @note `finite_draw_set_draw_position` does not impact this function.
*/
void finite_draw_text_group_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteTextGroup *groups, double x, double y, size_t n) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not draw text group with NULL Shell.");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_text_extents_t ext;

    for (int i = 0; i < n; i++) {
        cairo_set_source_rgb(cr, groups[i].r,groups[i].g, groups[i].b);
        // TODO allow alpha on text groups
        cairo_move_to(cr, x, y);
        cairo_show_text(cr, groups[i].text);

        cairo_text_extents(cr, groups[i].text, &ext);
        x += ext.x_advance;
    }
}

cairo_font_extents_t finite_draw_get_font_extents_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not get font extents with NULL Shell.");
    }

    // if no shell.cr throw an error because we can't get extents from to nothing
    if (!shell->cr) {
       finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_get_font_extents() may not be the first use of shell.cr. To get the font extents set a font first.");
    }

    cairo_t *cr = shell->cr;

    cairo_font_extents_t fext;
    cairo_font_extents(cr, &fext);

    return fext;
}

cairo_text_extents_t finite_draw_get_text_extents_debug(const char *file, const char *func, int line, FiniteShell *shell, char *str) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not get text extents with NULL Shell.");
    }

    // if no shell.cr throw an error because we can't get extents from to nothing
    if (!shell->cr) {
       finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_get_text_extents() may not be the first use of shell.cr. To get the text extents set a font first.");
    }

    cairo_t *cr = shell->cr;

    cairo_text_extents_t ext;
    cairo_text_extents(cr, str, &ext);
    return ext;
}

/*
    # finite_draw_pattern_linear

    Attempts to draw a linear gradient.

    @param startX,startY The start coordinates of the gradient
    @param endX,endY The end coordinates of the gradient
    @param points A pointer to an array of `FiniteGradientPoint`s
*/
cairo_pattern_t *finite_draw_pattern_linear_debug(const char *file, const char *func, int line, double startX, double startY, double endX, double endY, FiniteGradientPoint *points, size_t n) {
    cairo_pattern_t *pat = cairo_pattern_create_linear(startX, startY, endX, endY);

    for (int i = 0; i < n; i++) {
        if (points[i].a) {
            cairo_pattern_add_color_stop_rgba(pat, points[i].stop, points[i].r, points[i].g, points[i].b, points[i].a);
        } else {
            cairo_pattern_add_color_stop_rgb (pat, points[i].stop, points[i].r, points[i].g, points[i].b);
        }
    }

    return pat;
}

/*
    # finite_draw_rect

    Attempt to draw a rectangle

    @note You can choose to use a gradient pattern or a `FiniteColorGroup` but you may not use both. Having both defined will return an error.
*/ 
void finite_draw_rect_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, FiniteColorGroup *color, cairo_pattern_t *pat) {
    if (!shell || !shell->cairo_surface) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Invalid shell or cairo_surface");
    }

    
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;
    FINITE_LOG("Checks: %f %f %f %f", width, height, x, y);

    
    if (color && pat) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "finite_draw_rect() may not have multiple fill values.");
        return;
    }

    cairo_rectangle(cr, x, y, width, height);

    if (pat) {
        cairo_set_source(cr, pat);
        cairo_fill(cr);
    } else {
        if (color) {
            if (color->a) {
                cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);
            } else {
                cairo_set_source_rgb(cr, color->r, color->g, color->b);
            }
            cairo_fill(cr);
        } else {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Unable to fill. (Did you intend to not define a color or a pattern?)");
        }
    }
}

void finite_draw_create_snapshot_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    if (!shell->cairo_surface) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to snapshot window with no cairo_surface");
        return;
    }

    cairo_surface_flush(shell->cairo_surface);
    int stride = shell->stride;
    int height = shell->details->height;

    size_t size = stride * height;  
    unsigned char *snap = malloc(size);

    if (!snap) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to snapshot window. Alloc size: %ld", size);
        return;
    }

    memcpy(snap, cairo_image_surface_get_data(shell->cairo_surface), size);
    if (!snap) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Allocation failed");
        return;
    }

    shell->snapshot = snap;
}


// ?! you must call finite_draw_finish after loading a snapshot!!
void finite_draw_load_snapshot_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    if (!shell || !shell->snapshot) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to load snapshop.");
        return;
    }

    int stride = shell->stride;
    int height = shell->details->height;
    size_t size = stride * height;

    memcpy(cairo_image_surface_get_data(shell->cairo_surface), shell->snapshot, size);

    cairo_surface_mark_dirty(shell->cairo_surface);
}

void finite_draw_png_debug(const char *file, const char *func, int line, FiniteShell *shell, const char *path, double x, double y) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to draw png on NULL shell");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_surface_t *image = cairo_image_surface_create_from_png(path);
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Image not created");
        return;
    }

    cairo_image_surface_get_height(image);

    cairo_set_source_surface(cr, image, x, y);
    cairo_paint(cr);
    cairo_surface_destroy(image);
}

bool finite_draw_finish_debug(const char *file, const char *func, int line, FiniteShell *shell, int width, int height, int stride, bool withAlpha) {
    FINITE_LOG("Checks: %d %d %d %d", width, height, stride, withAlpha);
    cairo_destroy(shell->cr); 
    shell->cr = NULL;

    if (!shell->pool) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "No SHM pool allocated. Cannot create buffer.");
        return false;
    }

    FINITE_LOG("Surface status: %s", cairo_status_to_string(cairo_surface_status(shell->cairo_surface)));

    if (shell->buffer) {
        wl_buffer_destroy(shell->buffer);
        shell->buffer = NULL;
    }

    enum wl_shm_format form = WL_SHM_FORMAT_ARGB8888;
    shell->buffer = wl_shm_pool_create_buffer(shell->pool, 0, width, height, shell->stride, form);

    cairo_surface_flush(shell->cairo_surface);
    cairo_surface_mark_dirty(shell->cairo_surface);

    if (!shell->buffer) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create window geometry with NULL information.");
        wl_display_disconnect(shell->display);
        return false;
    }

    wl_surface_attach(shell->isle_surface, shell->buffer, 0,0);
    wl_surface_damage(shell->isle_surface, 0,0, width, height); // tell the surface to redraw
    wl_surface_commit(shell->isle_surface);
   return true;
}

void finite_draw_cleanup_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Close Requested.");
    cairo_surface_destroy(shell->cairo_surface);
    munmap(shell->pool_data, shell->pool_size);
    close(shell->shm_fd);
    wl_buffer_destroy(shell->buffer);
    wl_shm_pool_destroy(shell->pool);
    wl_shm_destroy(shell->shm);
    wl_display_disconnect(shell->display);
    free(shell->pool_data);
    free(shell->details);
    free(shell);
}