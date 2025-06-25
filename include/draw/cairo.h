#ifndef __DRAW_H__
#define __DRAW_H__
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
void finite_draw_set_font(FiniteShell *shell, char *font_name, bool isItalics, bool isBold, int size);
void finite_draw_set_text(FiniteShell *shell, char *text, FiniteColorGroup *color);
void finite_draw_set_draw_position(FiniteShell *shell, double x, double y);
void finite_draw_text_group(FiniteShell *shell, FiniteTextGroup *groups, double x, double y, size_t n);
cairo_font_extents_t finite_draw_get_font_extents(FiniteShell *shell);
cairo_text_extents_t finite_draw_get_text_extents(FiniteShell *shell, char *str);


cairo_pattern_t *finite_draw_pattern_linear(double startX, double startY, double endX, double endY, FiniteGradientPoint *points, size_t n);
cairo_pattern_t *finite_draw_pattern_radial(double startX, double startY, double endX, double endY, FiniteGradientPoint *points, double startRadius, double endRadius, size_t n);

void finite_draw_rect(FiniteShell *shell, double x, double y, double width, double height, FiniteColorGroup *color, cairo_pattern_t *pat);
void finite_draw_rounded_rect(FiniteShell *shell, double x, double y, double width, double height, double radius, FiniteColorGroup *color, cairo_pattern_t *pat, bool withPreserve);
void finite_draw_glow(FiniteShell *shell, double x, double y, double width, double height, double radius, int layers, FiniteColorGroup *color, bool withPreserve);
void finite_draw_stroke(FiniteShell *shell, FiniteColorGroup *color, cairo_pattern_t *pat, int width);
void finite_draw_set_offset(FiniteShell *shell, double x, double y);


void finite_draw_cleanup(FiniteShell *shell);



void finite_draw_create_snapshot(FiniteShell *shell);
void finite_draw_load_snapshot(FiniteShell *shell);


// TODO: Create a button type that can be used to mount interactable events to. Then when handling input, simply tell the window if we're focused or not. 
// ? Would use signals more than likely

#endif