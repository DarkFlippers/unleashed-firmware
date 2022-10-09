#include "usb_hid_dirpad.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>

struct UsbHidDirpad {
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
} UsbHidDirpadModel;

static void usb_hid_dirpad_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
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

static void usb_hid_dirpad_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    UsbHidDirpadModel* model = context;

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Dirpad");

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "Hold to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_dirpad_draw_arrow(canvas, 30, 30, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_dirpad_draw_arrow(canvas, 30, 55, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 45, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_dirpad_draw_arrow(canvas, 7, 53, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 45, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_dirpad_draw_arrow(canvas, 53, 53, CanvasDirectionLeftToRight);
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

static void usb_hid_dirpad_process(UsbHidDirpad* usb_hid_dirpad, InputEvent* event) {
    with_view_model(
        usb_hid_dirpad->view,
        UsbHidDirpadModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    furi_hal_hid_kb_press(HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    furi_hal_hid_kb_press(HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    furi_hal_hid_kb_press(HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    furi_hal_hid_kb_press(HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    furi_hal_hid_kb_press(HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    furi_hal_hid_kb_release(HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    furi_hal_hid_kb_release(HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    furi_hal_hid_kb_release(HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    furi_hal_hid_kb_release(HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    furi_hal_hid_kb_release(HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    furi_hal_hid_kb_press(HID_KEYBOARD_DELETE);
                    furi_hal_hid_kb_release(HID_KEYBOARD_DELETE);
                    furi_hal_hid_consumer_key_press(HID_CONSUMER_AC_BACK);
                    furi_hal_hid_consumer_key_release(HID_CONSUMER_AC_BACK);
                }
            }
        },
        true);
}

static bool usb_hid_dirpad_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    UsbHidDirpad* usb_hid_dirpad = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_hal_hid_kb_release_all();
    } else {
        usb_hid_dirpad_process(usb_hid_dirpad, event);
        consumed = true;
    }

    return consumed;
}

UsbHidDirpad* usb_hid_dirpad_alloc() {
    UsbHidDirpad* usb_hid_dirpad = malloc(sizeof(UsbHidDirpad));
    usb_hid_dirpad->view = view_alloc();
    view_set_context(usb_hid_dirpad->view, usb_hid_dirpad);
    view_allocate_model(usb_hid_dirpad->view, ViewModelTypeLocking, sizeof(UsbHidDirpadModel));
    view_set_draw_callback(usb_hid_dirpad->view, usb_hid_dirpad_draw_callback);
    view_set_input_callback(usb_hid_dirpad->view, usb_hid_dirpad_input_callback);

    return usb_hid_dirpad;
}

void usb_hid_dirpad_free(UsbHidDirpad* usb_hid_dirpad) {
    furi_assert(usb_hid_dirpad);
    view_free(usb_hid_dirpad->view);
    free(usb_hid_dirpad);
}

View* usb_hid_dirpad_get_view(UsbHidDirpad* usb_hid_dirpad) {
    furi_assert(usb_hid_dirpad);
    return usb_hid_dirpad->view;
}

void usb_hid_dirpad_set_connected_status(UsbHidDirpad* usb_hid_dirpad, bool connected) {
    furi_assert(usb_hid_dirpad);
    with_view_model(
        usb_hid_dirpad->view, UsbHidDirpadModel * model, { model->connected = connected; }, true);
}
