#include "../include/draw/cairo.h"
#include <unistd.h>

# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */

/*
    # finite_draw_rounded_rect

    Attempts to draw a rounded box.
*/
void finite_draw_rounded_rect(FiniteShell *shell, double x, double y, double width, double height, double radius, FiniteColorGroup *color, cairo_pattern_t *pat, bool withPreserve) {
    double r = radius;
    if (!shell) {
        printf("[Finite] - Can not draw a rounded rect with NULL shell.\n");
        return;
    }

    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    if (color && pat) {
        printf("[Finite] - finite_draw_rect() may not have multiple fill values.\n");
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
        } else {
            printf("[Finite] - Unable to fill. (Did you define a color or a pattern?)");
        }
    }

    if (withPreserve) {
        cairo_fill_preserve(cr);
    } else {
        cairo_fill(cr);
    }
}


// TODO: finite_draw_glow_gradient
void finite_draw_glow(FiniteShell *shell, double x, double y, double width, double height, double radius, int layers, FiniteColorGroup *color, bool withPreserve) {
        if (!shell) {
        printf("[Finite] - Can not draw a glow rect with NULL shell.\n");
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

void finite_draw_stroke(FiniteShell *shell, FiniteColorGroup *color, cairo_pattern_t *pat, int width) {
    if (!shell) {
        printf("[Finite] - Can not draw a rect stroke with NULL shell.\n");
        return;
    }
    
    // if shell.cr doesn't exist throw an error because we can't set a stroke to nothing
    if (!shell->cr) {
       printf("[Finite] - finite_draw_stroke() may not be the first use of shell.cr. To set a stroke create a rect first.\n");
    }

    cairo_t *cr = shell->cr;

    if (color && pat) {
        printf("[Finite] - finite_draw_stroke() may not have multiple fill values.\n");
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
            printf("[Finite] - Unable to fill. (Did you define a color or a pattern?)");
        }
    }

    cairo_set_line_width(cr, width);

    cairo_stroke(cr);
}

void finite_draw_set_offset(FiniteShell *shell, double x, double y) {
    if (!shell) {
        printf("[Finite] - Can not offset with NULL Shell.\n");
        return;
    }

    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_translate(cr, x, y);
}

/*
    # finite_draw_set_font

    Sets the font that will be used when drawing to text to the window.
*/
void finite_draw_set_font(FiniteShell *shell, char *font_name, bool isItalics, bool isBold, int size) {
    if (!shell) {
        printf("[Finite] - Can not draw text with NULL Shell.\n");
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
void finite_draw_set_text(FiniteShell *shell, char *text, FiniteColorGroup *color) {
    if (!shell) {
        printf("[Finite] - Can not draw text with NULL Shell.\n");
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
void finite_draw_set_draw_position(FiniteShell *shell, double x, double y) {
    if (!shell) {
        printf("[Finite] - Can not set draw position with NULL Shell.\n");
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
void finite_draw_text_group(FiniteShell *shell, FiniteTextGroup *groups, double x, double y, size_t n) {
    if (!shell) {
        printf("[Finite] - Can not draw text group with NULL Shell.\n");
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

cairo_font_extents_t finite_draw_get_font_extents(FiniteShell *shell) {
    if (!shell) {
        printf("[Finite] - Can not get font extents with NULL Shell.\n");
    }

    // if no shell.cr throw an error because we can't get extents from to nothing
    if (!shell->cr) {
       printf("[Finite] - finite_draw_get_font_extents() may not be the first use of shell.cr. To get the font extents set a font first.\n");
    }

    cairo_t *cr = shell->cr;

    cairo_font_extents_t fext;
    cairo_font_extents(cr, &fext);

    return fext;
}

cairo_text_extents_t finite_draw_get_text_extents(FiniteShell *shell, char *str) {
    if (!shell) {
        printf("[Finite] - Can not get text extents with NULL Shell.\n");
    }

    // if no shell.cr throw an error because we can't get extents from to nothing
    if (!shell->cr) {
       printf("[Finite] - finite_draw_get_text_extents() may not be the first use of shell.cr. To get the text extents set a font first.\n");
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
cairo_pattern_t *finite_draw_pattern_linear(double startX, double startY, double endX, double endY, FiniteGradientPoint *points, size_t n) {
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
void finite_draw_rect(FiniteShell *shell, double x, double y, double width, double height, FiniteColorGroup *color, cairo_pattern_t *pat) {
    if (!shell || !shell->cairo_surface) {
        printf("[Finite] - Invalid shell or cairo_surface\n");
        return;
    }

    
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;
    printf("Checks: %f %f %f %f\n", width, height, x, y);

    
    if (color && pat) {
        printf("[Finite] - finite_draw_rect() may not have multiple fill values.\n");
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
            printf("[Finite] - Unable to fill. (Did you define a color or a pattern?)\n");
        }
    }
}

void finite_draw_create_snapshot(FiniteShell *shell) {
    if (!shell->cairo_surface) {
        printf("[Finite] - Unable to snapshot window with no cairo_surface\n");
        return;
    }

    cairo_surface_flush(shell->cairo_surface);
    int stride = shell->stride;
    int height = shell->details->height;

    size_t size = stride * height;  
    unsigned char *snap = malloc(size);

    if (!snap) {
        printf("[Finite] - Unable to snapshot window. Alloc size: %ld\n", size);
        return;
    }

    memcpy(snap, cairo_image_surface_get_data(shell->cairo_surface), size);
    if (!snap) {
        printf("[Finite] - Allocation failed\n");
        return;
    }

    shell->snapshot = snap;
}


// ?! you must call finite_draw_finish after loading a snapshot!!
void finite_draw_load_snapshot(FiniteShell *shell) {
    if (!shell || !shell->snapshot) {
        printf("[Finite] - Unable to load snapshop.\n");
        return;
    }

    int stride = shell->stride;
    int height = shell->details->height;
    size_t size = stride * height;

    memcpy(cairo_image_surface_get_data(shell->cairo_surface), shell->snapshot, size);

    cairo_surface_mark_dirty(shell->cairo_surface);
}

bool finite_draw_finish(FiniteShell *shell, int width, int height, int stride, bool withAlpha) {
    printf("Checks: %d %d %d %d\n", width, height, stride, withAlpha);
    cairo_destroy(shell->cr); 
    shell->cr = NULL;

    if (!shell->pool) {
        printf("[Finite] - No SHM pool allocated. Cannot create buffer.\n");
        return false;
    }

    printf("Surface status: %s\n", cairo_status_to_string(cairo_surface_status(shell->cairo_surface)));

    if (shell->buffer) {
        wl_buffer_destroy(shell->buffer);
        shell->buffer = NULL;
    }

    enum wl_shm_format form = WL_SHM_FORMAT_ARGB8888;
    shell->buffer = wl_shm_pool_create_buffer(shell->pool, 0, shell->details->width, shell->details->height, shell->stride, form);

    cairo_surface_flush(shell->cairo_surface);
    cairo_surface_mark_dirty(shell->cairo_surface);

    if (!shell->buffer) {
        printf("[Finite] - Unable to create window geometry with NULL information.\n");
        wl_display_disconnect(shell->display);
        return false;
    }

    wl_surface_attach(shell->isle_surface, shell->buffer, 0,0);
    wl_surface_damage(shell->isle_surface, 0,0, shell->details->width, shell->details->height); // tell the surface to redraw
    wl_surface_commit(shell->isle_surface);
   return true;
}

void finite_draw_cleanup(FiniteShell *shell) {
    printf("[Finite] - Close Requested.\n");
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