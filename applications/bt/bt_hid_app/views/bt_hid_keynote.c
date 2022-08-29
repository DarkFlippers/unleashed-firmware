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
    bool back_pressed;
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
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Keynote");

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "Hold to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 30, 30, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 30, 55, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 45, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 7, 53, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 45, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    bt_hid_keynote_draw_arrow(canvas, 53, 53, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 63, 25, &I_Space_65x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 66, 27, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 29, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 91, 36, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 63, 45, &I_Space_65x18);
    if(model->back_pressed) {
        elements_slightly_rounded_box(canvas, 66, 47, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 49, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 91, 57, AlignLeft, AlignBottom, "Back");
}

static void bt_hid_keynote_process(BtHidKeynote* bt_hid_keynote, InputEvent* event) {
    with_view_model(
        bt_hid_keynote->view, (BtHidKeynoteModel * model) {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    furi_hal_bt_hid_kb_press(HID_KEYBOARD_DELETE);
                    furi_hal_bt_hid_kb_release(HID_KEYBOARD_DELETE);
                    furi_hal_bt_hid_consumer_key_press(HID_CONSUMER_AC_BACK);
                    furi_hal_bt_hid_consumer_key_release(HID_CONSUMER_AC_BACK);
                }
            }
            return true;
        });
}

static bool bt_hid_keynote_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BtHidKeynote* bt_hid_keynote = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_hal_bt_hid_kb_release_all();
    } else {
        bt_hid_keynote_process(bt_hid_keynote, event);
        consumed = true;
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
