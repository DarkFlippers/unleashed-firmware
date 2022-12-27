#include "usb_hid_mouse.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>
#include <USB_Keyboard_icons.h>

struct UsbHidMouse {
    View* view;
};
#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG 20

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool left_mouse_pressed;
    bool left_mouse_held;
    bool right_mouse_pressed;
    bool connected;
} UsbHidMouseModel;

static void usb_hid_mouse_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    UsbHidMouseModel* model = context;

    // Header
    /*if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }*/
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Mouse");
    canvas_set_font(canvas, FontSecondary);

    if(model->left_mouse_held == true) {
        elements_multiline_text_aligned(canvas, 0, 62, AlignLeft, AlignBottom, "Selecting...");
    } else {
        canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
    }

    // Keypad circles
    canvas_draw_icon(canvas, 64, 8, &I_Circles_47x47);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 81, 9, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 84, 10, &I_Pin_arrow_up_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 81, 41, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 84, 43, &I_Pin_arrow_down_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 65, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 67, 28, &I_Pin_arrow_left_9x7);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 97, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 99, 28, &I_Pin_arrow_right_9x7);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->left_mouse_pressed) {
        canvas_draw_icon(canvas, 81, 25, &I_Ok_btn_pressed_13x13);
    } else {
        canvas_draw_icon(canvas, 83, 27, &I_Left_mouse_icon_9x9);
    }

    // Back
    if(model->right_mouse_pressed) {
        canvas_draw_icon(canvas, 108, 48, &I_Ok_btn_pressed_13x13);
    } else {
        canvas_draw_icon(canvas, 110, 50, &I_Right_mouse_icon_9x9);
    }
}

static void usb_hid_mouse_process(UsbHidMouse* usb_hid_mouse, InputEvent* event) {
    with_view_model(
        usb_hid_mouse->view,
        UsbHidMouseModel * model,
        {
            if(event->key == InputKeyBack) {
                if(event->type == InputTypeShort) {
                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
                } else if(event->type == InputTypePress) {
                    model->right_mouse_pressed = true;
                } else if(event->type == InputTypeRelease) {
                    model->right_mouse_pressed = false;
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypeShort) {
                    // Just release if it was being held before
                    if(!model->left_mouse_held) furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
                    model->left_mouse_held = false;
                } else if(event->type == InputTypeLong) {
                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
                    model->left_mouse_held = true;
                    model->left_mouse_pressed = true;
                } else if(event->type == InputTypePress) {
                    model->left_mouse_pressed = true;
                } else if(event->type == InputTypeRelease) {
                    // Only release if it wasn't a long press
                    if(!model->left_mouse_held) model->left_mouse_pressed = false;
                }

            } else if(event->key == InputKeyRight) {
                if(event->type == InputTypePress) {
                    model->right_pressed = true;
                    furi_hal_hid_mouse_move(MOUSE_MOVE_SHORT, 0);
                } else if(event->type == InputTypeRepeat) {
                    furi_hal_hid_mouse_move(MOUSE_MOVE_LONG, 0);
                } else if(event->type == InputTypeRelease) {
                    model->right_pressed = false;
                }
            } else if(event->key == InputKeyLeft) {
                if(event->type == InputTypePress) {
                    model->left_pressed = true;
                    furi_hal_hid_mouse_move(-MOUSE_MOVE_SHORT, 0);
                } else if(event->type == InputTypeRepeat) {
                    furi_hal_hid_mouse_move(-MOUSE_MOVE_LONG, 0);
                } else if(event->type == InputTypeRelease) {
                    model->left_pressed = false;
                }
            } else if(event->key == InputKeyDown) {
                if(event->type == InputTypePress) {
                    model->down_pressed = true;
                    furi_hal_hid_mouse_move(0, MOUSE_MOVE_SHORT);
                } else if(event->type == InputTypeRepeat) {
                    furi_hal_hid_mouse_move(0, MOUSE_MOVE_LONG);
                } else if(event->type == InputTypeRelease) {
                    model->down_pressed = false;
                }
            } else if(event->key == InputKeyUp) {
                if(event->type == InputTypePress) {
                    model->up_pressed = true;
                    furi_hal_hid_mouse_move(0, -MOUSE_MOVE_SHORT);
                } else if(event->type == InputTypeRepeat) {
                    furi_hal_hid_mouse_move(0, -MOUSE_MOVE_LONG);
                } else if(event->type == InputTypeRelease) {
                    model->up_pressed = false;
                }
            }
        },
        true);
}

static bool usb_hid_mouse_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    UsbHidMouse* usb_hid_mouse = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
        furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
    } else {
        usb_hid_mouse_process(usb_hid_mouse, event);
        consumed = true;
    }

    return consumed;
}

UsbHidMouse* usb_hid_mouse_alloc() {
    UsbHidMouse* usb_hid_mouse = malloc(sizeof(UsbHidMouse));
    usb_hid_mouse->view = view_alloc();
    view_set_context(usb_hid_mouse->view, usb_hid_mouse);
    view_allocate_model(usb_hid_mouse->view, ViewModelTypeLocking, sizeof(UsbHidMouseModel));
    view_set_draw_callback(usb_hid_mouse->view, usb_hid_mouse_draw_callback);
    view_set_input_callback(usb_hid_mouse->view, usb_hid_mouse_input_callback);

    with_view_model(
        usb_hid_mouse->view, UsbHidMouseModel * model, { model->connected = true; }, true);

    return usb_hid_mouse;
}

void usb_hid_mouse_free(UsbHidMouse* usb_hid_mouse) {
    furi_assert(usb_hid_mouse);
    view_free(usb_hid_mouse->view);
    free(usb_hid_mouse);
}

View* usb_hid_mouse_get_view(UsbHidMouse* usb_hid_mouse) {
    furi_assert(usb_hid_mouse);
    return usb_hid_mouse->view;
}
