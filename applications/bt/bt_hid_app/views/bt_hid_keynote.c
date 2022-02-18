#include "bt_hid_keynote.h"
#include <furi.h>
#include <furi_hal_bt_hid.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>

struct BtHidKeynote {
    View* view;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool connected;
} BtHidKeynoteModel;

static void bt_hid_keynote_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    canvas_draw_triangle(canvas, x, y, 5, 3, dir);
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_line(canvas, x, y + 6, x, y - 1);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_line(canvas, x, y - 6, x, y + 1);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_line(canvas, x + 6, y, x - 1, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_line(canvas, x - 6, y, x + 1, y);
    }
}

static void bt_hid_keynote_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    BtHidKeynoteModel* model = context;

    // Header
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 9, 3, AlignLeft, AlignTop, "Keynote");
    canvas_set_font(canvas, FontSecondary);

    // Connected status
    if(model->connected) {
        canvas_draw_icon(canvas, 18, 18, &I_Ble_connected_38x34);
        elements_multiline_text_aligned(canvas, 9, 60, AlignLeft, AlignBottom, "Connected");
    } else {
        canvas_draw_icon(canvas, 18, 18, &I_Ble_disconnected_24x34);
        elements_multiline_text_aligned(canvas, 3, 60, AlignLeft, AlignBottom, "Disconnected");
    }

    // Up
    canvas_draw_icon(canvas, 86, 4, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 89, 6, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 95, 10, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 86, 25, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 89, 27, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 95, 35, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 65, 25, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 68, 27, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 72, 33, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 107, 25, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 110, 27, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 118, 33, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 63, 45, &I_Space_65x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 66, 47, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 49, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 91, 56, AlignLeft, AlignBottom, "Space");
}

static void bt_hid_keynote_process_press(BtHidKeynote* bt_hid_keynote, InputEvent* event) {
    with_view_model(
        bt_hid_keynote->view, (BtHidKeynoteModel * model) {
            if(event->key == InputKeyUp) {
                model->up_pressed = true;
                furi_hal_bt_hid_kb_press(KEY_UP_ARROW);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = true;
                furi_hal_bt_hid_kb_press(KEY_DOWN_ARROW);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = true;
                furi_hal_bt_hid_kb_press(KEY_LEFT_ARROW);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = true;
                furi_hal_bt_hid_kb_press(KEY_RIGHT_ARROW);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = true;
                furi_hal_bt_hid_kb_press(KEY_SPACE);
            }
            return true;
        });
}

static void bt_hid_keynote_process_release(BtHidKeynote* bt_hid_keynote, InputEvent* event) {
    with_view_model(
        bt_hid_keynote->view, (BtHidKeynoteModel * model) {
            if(event->key == InputKeyUp) {
                model->up_pressed = false;
                furi_hal_bt_hid_kb_release(KEY_UP_ARROW);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = false;
                furi_hal_bt_hid_kb_release(KEY_DOWN_ARROW);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = false;
                furi_hal_bt_hid_kb_release(KEY_LEFT_ARROW);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = false;
                furi_hal_bt_hid_kb_release(KEY_RIGHT_ARROW);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = false;
                furi_hal_bt_hid_kb_release(KEY_SPACE);
            }
            return true;
        });
}

static bool bt_hid_keynote_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BtHidKeynote* bt_hid_keynote = context;
    bool consumed = false;

    if(event->type == InputTypePress) {
        bt_hid_keynote_process_press(bt_hid_keynote, event);
        consumed = true;
    } else if(event->type == InputTypeRelease) {
        bt_hid_keynote_process_release(bt_hid_keynote, event);
        consumed = true;
    } else if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_hal_hid_kb_release_all();
        }
    }

    return consumed;
}

BtHidKeynote* bt_hid_keynote_alloc() {
    BtHidKeynote* bt_hid_keynote = malloc(sizeof(BtHidKeynote));
    bt_hid_keynote->view = view_alloc();
    view_set_context(bt_hid_keynote->view, bt_hid_keynote);
    view_allocate_model(bt_hid_keynote->view, ViewModelTypeLocking, sizeof(BtHidKeynoteModel));
    view_set_draw_callback(bt_hid_keynote->view, bt_hid_keynote_draw_callback);
    view_set_input_callback(bt_hid_keynote->view, bt_hid_keynote_input_callback);

    return bt_hid_keynote;
}

void bt_hid_keynote_free(BtHidKeynote* bt_hid_keynote) {
    furi_assert(bt_hid_keynote);
    view_free(bt_hid_keynote->view);
    free(bt_hid_keynote);
}

View* bt_hid_keynote_get_view(BtHidKeynote* bt_hid_keynote) {
    furi_assert(bt_hid_keynote);
    return bt_hid_keynote->view;
}

void bt_hid_keynote_set_connected_status(BtHidKeynote* bt_hid_keynote, bool connected) {
    furi_assert(bt_hid_keynote);
    with_view_model(
        bt_hid_keynote->view, (BtHidKeynoteModel * model) {
            model->connected = connected;
            return true;
        });
}
