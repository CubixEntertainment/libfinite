#ifndef __TEXTBOX_H__
#define __TEXTBOX_H__

#include "draw/cairo.h"
#include "draw/window.h"
#include "input/keyboard.h"


typedef struct FiniteTextbox FiniteTextbox;
typedef struct FiniteTextboxDetails FiniteTextboxDetails;
typedef enum FiniteTextboxCommitType FiniteTextboxCommitType;
typedef enum FiniteTextboxRenderMode FiniteTextboxRenderMode;
typedef enum FiniteTextboxTextAlign FiniteTextboxTextAlign;

// the on_commit and on_finish callbacks expect some type of commit type info
enum FiniteTextboxCommitType {
    TEXTBOX_COMMIT_ADD,
    TEXTBOX_COMMIT_REMOVE,
    TEXTBOX_COMMIT_FINISH,
    TEXTBOX_COMMIT_ENTER,
    TEXTBOX_COMMIT_LEAVE
};

enum FiniteTextboxRenderMode {
    TEXTBOX_RENDER_MODE_BOTH, // libfinite will try to redraw and return the on_commit callback
    TEXTBOX_RENDER_MODE_IMPLICIT, // libfinite will handle everything and will NOT return the on_commit callback
    TEXTBOX_RENDER_MODE_EXPLICIT, // devs will handle everything
};

enum FiniteTextboxTextAlign {
    TEXTBOX_ALIGN_LEFT,
    TEXTBOX_ALIGN_CENTER
};

struct FiniteTextboxDetails {
    double x;
    double y;
    double width;
    double height;

    char *text_font;
    bool isRounded;
    int box_radius;
    int text_size;
    int stroke_weight;

    FiniteTextboxTextAlign text_align;
    FiniteColorGroup background_color;
    FiniteColorGroup box_color;
    FiniteColorGroup text_color;
    FiniteColorGroup stroke_color;
};

struct FiniteTextbox {
    bool focused;
    char *text;
    char *final_text; // what actually shows up

    FiniteShell *shell;
    FiniteTextboxDetails *details;
    FiniteTextboxRenderMode redraw_mode;

    void *data;

    // callbacks for commit and done events
    void (*on_focused)(FiniteTextbox *self, FiniteTextboxCommitType type, void *data);
    void (*on_commit) (FiniteTextbox *self, FiniteTextboxCommitType type, void *data);
    void (*on_done)   (FiniteTextbox *self, FiniteTextboxCommitType type, void *data);
};

#define finite_input_textbox_create(shell, details, mode, data) finite_input_textbox_create_debug(__FILE__, __func__, __LINE__, shell, details, mode, data)
FiniteTextbox *finite_input_textbox_create_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteTextboxDetails *details, FiniteTextboxRenderMode mode, void *data);

#define finite_input_textbox_set_callacks(textbox, on_focused, on_commit, on_done, data) finite_input_textbox_set_callbacks_debug(__FILE__, __func__, __LINE__, textbox, on_focused, on_commit, on_done, data)
void finite_input_textbox_set_callbacks_debug(const char *file, const char *func, int line, FiniteTextbox *textbox, void (*on_focused)(FiniteTextbox *self, FiniteTextboxCommitType type, void *data), void (*on_commit) (FiniteTextbox *self, FiniteTextboxCommitType type, void *data), void (*on_done)   (FiniteTextbox *self, FiniteTextboxCommitType type, void *data));

#define finite_input_textbox_request_input(textbox, kbd) finite_input_textbox_request_input_debug(__FILE__, __func__, __LINE__, textbox, kbd)
void finite_input_textbox_request_input_debug(const char *file, const char *func, int line, FiniteTextbox *textbox, FiniteKeyboard *kbd);

#endif