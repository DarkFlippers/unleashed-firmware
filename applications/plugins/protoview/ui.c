/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"

/* =========================== Subview handling ================================
 * Note that these are not the Flipper subviews, but the subview system
 * implemented inside ProtoView.
 * ========================================================================== */

/* Return the ID of the currently selected subview, of the current
 * view. */
int ui_get_current_subview(ProtoViewApp* app) {
    return app->current_subview[app->current_view];
}

/* Called by view rendering callback that has subviews, to show small triangles
 * facing down/up if there are other subviews the user can access with up
 * and down. */
void ui_show_available_subviews(Canvas* canvas, ProtoViewApp* app, int last_subview) {
    int subview = ui_get_current_subview(app);
    if(subview != 0) canvas_draw_triangle(canvas, 120, 5, 8, 5, CanvasDirectionBottomToTop);
    if(subview != last_subview - 1)
        canvas_draw_triangle(canvas, 120, 59, 8, 5, CanvasDirectionTopToBottom);
}

/* Handle up/down keys when we are in a subview. If the function catched
 * such keypress, it returns true, so that the actual view input callback
 * knows it can just return ASAP without doing anything. */
bool ui_process_subview_updown(ProtoViewApp* app, InputEvent input, int last_subview) {
    int subview = ui_get_current_subview(app);
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
void ui_show_keyboard(
    ProtoViewApp* app,
    char* buffer,
    uint32_t buflen,
    void (*done_callback)(void*)) {
    app->show_text_input = true;
    app->text_input_buffer = buffer;
    app->text_input_buffer_len = buflen;
    app->text_input_done_callback = done_callback;
}

void ui_dismiss_keyboard(ProtoViewApp* app) {
    view_dispatcher_stop(app->view_dispatcher);
}

/* ================================= Alert ================================== */

/* Set an alert message to be shown over any currently active view, for
 * the specified amount of time of 'ttl' milliseconds. */
void ui_show_alert(ProtoViewApp* app, const char* text, uint32_t ttl) {
    app->alert_dismiss_time = furi_get_tick() + furi_ms_to_ticks(ttl);
    snprintf(app->alert_text, ALERT_MAX_LEN, "%s", text);
}

/* Cancel the alert before its time has elapsed. */
void ui_dismiss_alert(ProtoViewApp* app) {
    app->alert_dismiss_time = 0;
}

/* Show the alert if an alert is set. This is called after the currently
 * active view displayed its stuff, so we overwrite the screen with the
 * alert message. */
void ui_draw_alert_if_needed(Canvas* canvas, ProtoViewApp* app) {
    if(app->alert_dismiss_time == 0) {
        /* No active alert. */
        return;
    } else if(app->alert_dismiss_time < furi_get_tick()) {
        /* Alert just expired. */
        ui_dismiss_alert(app);
        return;
    }

    /* Show the alert. A box with black border and a text inside. */
    canvas_set_font(canvas, FontPrimary);
    uint8_t w = canvas_string_width(canvas, app->alert_text);
    uint8_t h = 8; // Font height.
    uint8_t text_x = 64 - (w / 2);
    uint8_t text_y = 32 + 4;
    uint8_t padding = 3;
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(
        canvas, text_x - padding, text_y - padding - h, w + padding * 2, h + padding * 2);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(
        canvas,
        text_x - padding + 1,
        text_y - padding - h + 1,
        w + padding * 2 - 2,
        h + padding * 2 - 2);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(canvas, text_x, text_y, app->alert_text);
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
