#include "hid_mouse.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMouse"

struct HidMouse {
    View* view;
    Hid* hid;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool left_mouse_pressed;
    bool left_mouse_held;
    bool right_mouse_pressed;
    bool connected;
    HidTransport transport;
} HidMouseModel;

static void hid_mouse_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMouseModel* model = context;

    // Header
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }
    }

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
    canvas_draw_icon(canvas, 58, 3, &I_OutCircles_70x51);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 68, 6, &I_S_UP_31x15);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 80, 8, &I_Pin_arrow_up_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 68, 36, &I_S_DOWN_31x15);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 80, 40, &I_Pin_arrow_down_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 61, 13, &I_S_LEFT_15x31);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 63, 25, &I_Pin_arrow_left_9x7);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 91, 13, &I_S_RIGHT_15x31);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 95, 25, &I_Pin_arrow_right_9x7);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->left_mouse_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 74, 19, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 79, 24, &I_Left_mouse_icon_9x9);
    canvas_set_color(canvas, ColorBlack);

    // Back
    if(model->right_mouse_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 107, 33, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 112, 38, &I_Right_mouse_icon_9x9);
    canvas_set_color(canvas, ColorBlack);
}

static void hid_mouse_process(HidMouse* hid_mouse, InputEvent* event) {
    with_view_model(
        hid_mouse->view,
        HidMouseModel * model,
        {
            if(event->key == InputKeyBack) {
                if(event->type == InputTypeShort) {
                    hid_hal_mouse_press(hid_mouse->hid, HID_MOUSE_BTN_RIGHT);
                    hid_hal_mouse_release(hid_mouse->hid, HID_MOUSE_BTN_RIGHT);
                } else if(event->type == InputTypePress) {
                    model->right_mouse_pressed = true;
                } else if(event->type == InputTypeRelease) {
                    model->right_mouse_pressed = false;
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypeShort) {
                    // Just release if it was being held before
                    if(!model->left_mouse_held)
                        hid_hal_mouse_press(hid_mouse->hid, HID_MOUSE_BTN_LEFT);
                    hid_hal_mouse_release(hid_mouse->hid, HID_MOUSE_BTN_LEFT);
                    model->left_mouse_held = false;
                } else if(event->type == InputTypeLong) {
                    hid_hal_mouse_press(hid_mouse->hid, HID_MOUSE_BTN_LEFT);
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
                    hid_hal_mouse_move(hid_mouse->hid, MOUSE_MOVE_SHORT, 0);
                } else if(event->type == InputTypeRepeat) {
                    hid_hal_mouse_move(hid_mouse->hid, MOUSE_MOVE_LONG, 0);
                } else if(event->type == InputTypeRelease) {
                    model->right_pressed = false;
                }
            } else if(event->key == InputKeyLeft) {
                if(event->type == InputTypePress) {
                    model->left_pressed = true;
                    hid_hal_mouse_move(hid_mouse->hid, -MOUSE_MOVE_SHORT, 0);
                } else if(event->type == InputTypeRepeat) {
                    hid_hal_mouse_move(hid_mouse->hid, -MOUSE_MOVE_LONG, 0);
                } else if(event->type == InputTypeRelease) {
                    model->left_pressed = false;
                }
            } else if(event->key == InputKeyDown) {
                if(event->type == InputTypePress) {
                    model->down_pressed = true;
                    hid_hal_mouse_move(hid_mouse->hid, 0, MOUSE_MOVE_SHORT);
                } else if(event->type == InputTypeRepeat) {
                    hid_hal_mouse_move(hid_mouse->hid, 0, MOUSE_MOVE_LONG);
                } else if(event->type == InputTypeRelease) {
                    model->down_pressed = false;
                }
            } else if(event->key == InputKeyUp) {
                if(event->type == InputTypePress) {
                    model->up_pressed = true;
                    hid_hal_mouse_move(hid_mouse->hid, 0, -MOUSE_MOVE_SHORT);
                } else if(event->type == InputTypeRepeat) {
                    hid_hal_mouse_move(hid_mouse->hid, 0, -MOUSE_MOVE_LONG);
                } else if(event->type == InputTypeRelease) {
                    model->up_pressed = false;
                }
            }
        },
        true);
}

static bool hid_mouse_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMouse* hid_mouse = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_mouse_release_all(hid_mouse->hid);
    } else {
        hid_mouse_process(hid_mouse, event);
        consumed = true;
    }

    return consumed;
}

HidMouse* hid_mouse_alloc(Hid* hid) {
    HidMouse* hid_mouse = malloc(sizeof(HidMouse));
    hid_mouse->view = view_alloc();
    hid_mouse->hid = hid;
    view_set_context(hid_mouse->view, hid_mouse);
    view_allocate_model(hid_mouse->view, ViewModelTypeLocking, sizeof(HidMouseModel));
    view_set_draw_callback(hid_mouse->view, hid_mouse_draw_callback);
    view_set_input_callback(hid_mouse->view, hid_mouse_input_callback);

    with_view_model(
        hid_mouse->view, HidMouseModel * model, { model->transport = hid->transport; }, true);

    return hid_mouse;
}

void hid_mouse_free(HidMouse* hid_mouse) {
    furi_assert(hid_mouse);
    view_free(hid_mouse->view);
    free(hid_mouse);
}

View* hid_mouse_get_view(HidMouse* hid_mouse) {
    furi_assert(hid_mouse);
    return hid_mouse->view;
}

void hid_mouse_set_connected_status(HidMouse* hid_mouse, bool connected) {
    furi_assert(hid_mouse);
    with_view_model(
        hid_mouse->view, HidMouseModel * model, { model->connected = connected; }, true);
}
