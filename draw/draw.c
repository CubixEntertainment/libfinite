#include "../include/draw/cairo.h"

# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */

/*
    # finite_draw_rounded_rectangle

    Attempts to draw a rounded box.
*/
void finite_draw_rounded_rectangle(FiniteShell *shell, double x, double y, double width, double height, double radius, FiniteColorGroup color, bool withPreserve) {
    double r = radius;
    // if shell.cr doesn't exist allocate one
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    // create the rounded box
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - r, y + r,     r, -M_PI_2, 0);
    cairo_arc(cr, x + width - r, y + height - r, r, 0, M_PI_2);
    cairo_arc(cr, x + r,     y + height - r, r, M_PI_2, M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI, 3 * M_PI_2);
    cairo_close_path(cr);

    cairo_set_source_rgb(cr, color.r, color.g, color.b);
    if (color.a) {
        cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    }

    if (withPreserve) {
        cairo_fill_preserve(cr);
    } else {
        cairo_fill(cr);
    }
}


// void finite_draw_glow(FiniteShell *shell, double x, double y, double width, double height, double radius, int layers) {
//     // if shell.cr doesn't exist allocate one
//     if (!shell->cr) {
//         shell->cr = cairo_create(shell->cairo_surface);
//     }

//     cairo_t *cr = shell->cr;
    
//     for (int i = layers; i > 0; i--) {
//         double alpha = 0.05 + (0.15 * i / layers);  // Increasing opacity
//         double grow = i;  // Expand outward

//         finite_draw_rounded_rectangle(shell, x - grow, y - grow, width + grow * 2, height + grow * 2, radius + grow);
//         cairo_pattern_t *pat = cairo_pattern_create_linear (0.0, (double)(height), (double)width, 0.0);
//         cairo_pattern_add_color_stop_rgba(pat, 0, 0.506, 0.35, 0.137, alpha);
//         cairo_pattern_add_color_stop_rgba(pat, 0.7, 0.195, 0.195, 0.195, alpha);
//         cairo_set_source (cr, pat);
//         cairo_fill(cr);
//     }
// }

/*
    # finite_draw_set_font

    Sets the font that will be used when drawing to text to the window.
*/
void finite_draw_set_font(FiniteShell *shell, char *font_name, bool isItalics, bool isBold, int size) {
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    enum _cairo_font_slant slant;
    enum _cairo_font_weight bold;

    slant = (isItalics = true) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_WEIGHT_NORMAL;
    bold = (isBold = true) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_SLANT_NORMAL;

    cairo_select_font_face(cr, font_name, slant, bold);
    cairo_set_font_size(cr, 24);
}

/*
    # finite_draw_set_text

    Draw text to the screen. 
    
    @note `finite_draw_set_font()` should be called before drawing text for the first time.
*/
void finite_draw_set_text(FiniteShell *shell, char *text, FiniteColorGroup *color) {
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
void finite_draw_text_group(FiniteShell *shell, FiniteTextGroup *groups, double x, double y) {
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;

    cairo_text_extents_t ext;

    size_t n = (&groups)[1] - groups;

    for (int i = 0; i < n; i++) {
        cairo_set_source_rgb(cr, groups[i].r,groups[i].g, groups[i].b);
        // TODO allow alpha on text groups
        cairo_move_to(cr, x, y);
        cairo_show_text(cr, groups[i].text);

        cairo_text_extents(cr, groups[i].text, &ext);
        x += ext.x_advance;
    }
}

/*
    # finite_draw_pattern_linear

    Attempts to draw a linear gradient.

    @param startX,startY The start coordinates of the gradient
    @param endX,endY The end coordinates of the gradient
    @param points A pointer to an array of `FiniteGradientPoint`s
*/
cairo_pattern_t *finite_draw_pattern_linear(double startX, double startY, double endX, double endY, FiniteGradientPoint *points) {
    cairo_pattern_t *pat = cairo_pattern_create_linear(startX, startY, endX, endY);

    size_t n = (&points)[1] - points;

    for (int i = 0; i < n; i++) {
        cairo_pattern_add_color_stop_rgb (pat, points[i].stop, points[i].r, points[i].g, points[i].b);
    }

    return pat;
}

/*
    # finite_draw_rect

    Attempt to draw a rectangle

    @note You can choose to use a gradient pattern or a `FiniteColorGroup` but you may not use both. Having both defined will return an error.
*/ 
void finite_draw_rect(FiniteShell *shell, double x, double y, double width, double height, FiniteColorGroup *color, cairo_pattern_t *pat) {
    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_t *cr = shell->cr;
    
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
            cairo_set_source_rgb(cr, color->r, color->g, color->b);
            cairo_fill(cr);
            if (color->a) {
                cairo_set_source_rgba(cr, color->r, color->g, color->b, color->a);
                cairo_fill(cr);
            }
        } else {
            printf("[Finite] - Unable to fill. (Did you define a color or a pattern?)");
        }
    }
}

bool finite_draw_finish(FiniteShell *shell, int width, int height, int stride, bool withAlpha) {
    cairo_destroy(shell->cr);    
    enum wl_shm_format form = (withAlpha) ? WL_SHM_FORMAT_ARGB8888 : WL_SHM_FORMAT_XRGB8888;
    shell->buffer = wl_shm_pool_create_buffer(shell->pool, 0, width, height, stride, form);
    if (!shell->buffer) {
        printf("[Finite] - Unable to create window geometry with NULL information.\n");
        wl_display_disconnect(shell->display);
        return false;
    }

   wl_surface_attach(shell->isle_surface, shell->buffer, 0,0);
   wl_surface_damage(shell->isle_surface, 0,0, UINT32_MAX, UINT32_MAX); // tell the surface to redraw
   wl_surface_commit(shell->isle_surface);
   return true;
}