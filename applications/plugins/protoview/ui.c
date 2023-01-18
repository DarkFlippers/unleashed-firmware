/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

/* =========================== Subview handling ================================
 * Note that these are not the Flipper subviews, but the subview system
 * implemented inside ProtoView.
 * ========================================================================== */

/* Return the ID of the currently selected subview, of the current
 * view. */
int get_current_subview(ProtoViewApp* app) {
    return app->current_subview[app->current_view];
}

/* Called by view rendering callback that has subviews, to show small triangles
 * facing down/up if there are other subviews the user can access with up
 * and down. */
void show_available_subviews(Canvas* canvas, ProtoViewApp* app, int last_subview) {
    int subview = get_current_subview(app);
    if(subview != 0) canvas_draw_triangle(canvas, 120, 5, 8, 5, CanvasDirectionBottomToTop);
    if(subview != last_subview - 1)
        canvas_draw_triangle(canvas, 120, 59, 8, 5, CanvasDirectionTopToBottom);
}

/* Handle up/down keys when we are in a subview. If the function catched
 * such keypress, it returns true, so that the actual view input callback
 * knows it can just return ASAP without doing anything. */
bool process_subview_updown(ProtoViewApp* app, InputEvent input, int last_subview) {
    int subview = get_current_subview(app);
    if(input.type == InputTypePress) {
        if(input.key == InputKeyUp) {
            if(subview != 0) app->current_subview[app->current_view]--;
            return true;
        } else if(input.key == InputKeyDown) {
            if(subview != last_subview - 1) app->current_subview[app->current_view]++;
            return true;
        }
    }
    return false;
}

/* ============================= Text input ====================================
 * Normally we just use our own private UI widgets. However for the text input
 * widget, that is quite complex, visualizes a keyboard and must be standardized
 * for user coherent experience, we use the one provided by the Flipper
 * framework. The following two functions allow to show the keyboard to get
 * text and later dismiss it.
 * ========================================================================== */

/* Show the keyboard, take the user input and store it into the specified
 * 'buffer' of 'buflen' total bytes. When the user is done, the done_callback
 * is called passing the application context to it. Such callback needs
 * to do whatever it wants with the input buffer and dismissi the keyboard
 * calling: dismiss_keyboard(app);
 *
 * Note: if the buffer is not a null-termined zero string, what it contains will
 * be used as initial input for the user. */
void show_keyboard(ProtoViewApp* app, char* buffer, uint32_t buflen, void (*done_callback)(void*)) {
    app->show_text_input = true;
    app->text_input_buffer = buffer;
    app->text_input_buffer_len = buflen;
    app->text_input_done_callback = done_callback;
}

void dismiss_keyboard(ProtoViewApp* app) {
    view_dispatcher_stop(app->view_dispatcher);
}

/* =========================== Canvas extensions ============================ */

void canvas_draw_str_with_border(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    const char* str,
    Color text_color,
    Color border_color) {
    struct {
        uint8_t x;
        uint8_t y;
    } dir[8] = {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}};

    /* Rotate in all the directions writing the same string to create a
     * border, then write the actual string in the other color in the
     * middle. */
    canvas_set_color(canvas, border_color);
    for(int j = 0; j < 8; j++) canvas_draw_str(canvas, x + dir[j].x, y + dir[j].y, str);
    canvas_set_color(canvas, text_color);
    canvas_draw_str(canvas, x, y, str);
    canvas_set_color(canvas, ColorBlack);
}
