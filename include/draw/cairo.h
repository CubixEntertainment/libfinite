#ifndef __CAIRO_H__
#define __CAIRO_H__
#include "window.h"
#include <cairo/cairo.h>

typedef struct {
    double r;
    double g;
    double b;
    double a;
    const char *text;
} FiniteTextGroup;

typedef struct {
    double r;
    double g;
    double b;
    double a;
} FiniteColorGroup;

typedef struct {
    double stop;
    double r;
    double g;
    double b;
    double a;
} FiniteGradientPoint;


// cairo wrappers
#define finite_draw_set_font(shell, font_name, isItalics, isBold, size) finite_draw_set_font_debug(__FILE__, __func__, __LINE__, shell, font_name, isItalics, isBold, size)
void finite_draw_set_font_debug(const char *file, const char *func, int line, FiniteShell *shell, char *font_name, bool isItalics, bool isBold, int size);

#define finite_draw_set_text(shell, text, color) finite_draw_set_text_debug(__FILE__, __func__, __LINE__, shell, text, color)
void finite_draw_set_text_debug(const char *file, const char *func, int line, FiniteShell *shell, char *text, FiniteColorGroup *color);

#define finite_draw_set_draw_position(shell, x, y) finite_draw_set_draw_position_debug(__FILE__, __func__, __LINE__, shell, x, y)
void finite_draw_set_draw_position_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y);

#define finite_draw_text_group(shell, groups, x, y, n) finite_draw_text_group_debug(__FILE__, __func__, __LINE__, shell, groups, x, y, n)
void finite_draw_text_group_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteTextGroup *groups, double x, double y, size_t n);

#define finite_draw_get_font_extents(shell) finite_draw_get_font_extents_debug(__FILE__, __func__, __LINE__, shell)
cairo_font_extents_t finite_draw_get_font_extents_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_draw_get_text_extents(shell, str) finite_draw_get_text_extents_debug(__FILE__, __func__, __LINE__, shell, str)
cairo_text_extents_t finite_draw_get_text_extents_debug(const char *file, const char *func, int line, FiniteShell *shell, char *str);

#define finite_draw_pattern_linear(startX, startY, endX, endY, points, n) finite_draw_pattern_linear_debug(__FILE__, __func__, __LINE__, startX, startY, endX, endY, points, n)
cairo_pattern_t *finite_draw_pattern_linear_debug(const char *file, const char *func, int line, double startX, double startY, double endX, double endY, FiniteGradientPoint *points, size_t n);

#define finite_draw_pattern_radial(startX, startY, endX, endY, points, startRadius, endRadius, n) finite_draw_pattern_radial_debug(__FILE__, __func__, __LINE__, startX, startY, endX, endY, points, startRadius, endRadius, n)
cairo_pattern_t *finite_draw_pattern_radial_debug(const char *file, const char *func, int line, double startX, double startY, double endX, double endY, FiniteGradientPoint *points, double startRadius, double endRadius, size_t n);

#define finite_draw_rect(shell, x, y, width, height, color, pat) finite_draw_rect_debug(__FILE__, __func__, __LINE__, shell, x, y, width, height, color, pat)
void finite_draw_rect_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, FiniteColorGroup *color, cairo_pattern_t *pat);

#define finite_draw_rounded_rect(shell, x, y, width, height, radius, color, pat, withPreserve) finite_draw_rounded_rect_debug(__FILE__, __func__, __LINE__, shell, x, y, width, height, radius, color, pat, withPreserve)
void finite_draw_rounded_rect_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, double radius, FiniteColorGroup *color, cairo_pattern_t *pat, bool withPreserve);

#define finite_draw_glow(shell, x, y, width, height, radius, layers, color, withPreserve) finite_draw_glow_debug(__FILE__, __func__, __LINE__, shell, x, y, width, height, radius, layer, color, withPreserve)
void finite_draw_glow_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y, double width, double height, double radius, int layers, FiniteColorGroup *color, bool withPreserve);

#define finite_draw_stroke(shell, color, pat, width) finite_draw_stroke_debug(__FILE__, __func__, __LINE__, shell, color, pat, width)
void finite_draw_stroke_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteColorGroup *color, cairo_pattern_t *pat, int width);

#define finite_draw_set_offset(shell, x, y) finite_draw_set_offset(__FILE__, __func__, __LINE__, shell, x, y)
void finite_draw_set_offset_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y);

#define finite_draw_reset_offset(shell, x, y) finite_draw_reset_offset(__FILE__, __func__, __LINE__, shell, x, y)
void finite_draw_reset_offset_debug(const char *file, const char *func, int line, FiniteShell *shell, double x, double y);

#define finite_draw_png(shell, path, x, y, width, height, fillOnFail) finite_draw_png_debug(__FILE__, __func__, __LINE__, shell, path, x, y, width, height, fillOnFail)
void finite_draw_png_debug(const char *file, const char *func, int line, FiniteShell *shell, const char *path, double x, double y, double width, double height,  FiniteColorGroup *fillOnFail);
#define finite_draw_cleanup(shell) finite_draw_cleanup_debug(__FILE__, __func__, __LINE__, shell)
void finite_draw_cleanup_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_draw_create_snapshot(shell) finite_draw_create_snapshot_debug(__FILE__, __func__, __LINE__, shell)
void finite_draw_create_snapshot_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_draw_load_snapshot(shell) finite_draw_load_snapshot_debug(__FILE__, __func__, __LINE__, shell)
void finite_draw_load_snapshot_debug(const char *file, const char *func, int line, FiniteShell *shell);

#define finite_draw_hex_to_color_group(hex) finite_draw_hex_to_color_group_debug(__FILE__, __func__, __LINE__, hex)
FiniteColorGroup finite_draw_hex_to_color_group_debug(const char *file, const char *func, int line, char hex[7]);

#endif
