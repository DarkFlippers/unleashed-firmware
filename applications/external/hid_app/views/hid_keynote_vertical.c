#include "hid_keynote_vertical.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidKeynoteVertical"

struct HidKeynoteVertical {
    View* view;
    Hid* hid;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
    HidTransport transport;
} HidKeynoteVerticalModel;

static void
    hid_keynote_vertical_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
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

static void hid_keynote_vertical_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidKeynoteVerticalModel* model = context;

    // Header
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Keynote");
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(
        canvas, 24, 14, AlignLeft, AlignTop, "Vertical              Up --->");

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "Hold to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_vertical_draw_arrow(canvas, 30, 30, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_vertical_draw_arrow(canvas, 30, 55, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 35, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 37, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_vertical_draw_arrow(canvas, 7, 43, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 35, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 37, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_vertical_draw_arrow(canvas, 53, 43, CanvasDirectionLeftToRight);
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

static void
    hid_keynote_vertical_process(HidKeynoteVertical* hid_keynote_vertical, InputEvent* event) {
    with_view_model(
        hid_keynote_vertical->view,
        HidKeynoteVerticalModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    hid_hal_keyboard_press(hid_keynote_vertical->hid, HID_KEYBOARD_DELETE);
                    hid_hal_keyboard_release(hid_keynote_vertical->hid, HID_KEYBOARD_DELETE);
                    hid_hal_consumer_key_press(hid_keynote_vertical->hid, HID_CONSUMER_AC_BACK);
                    hid_hal_consumer_key_release(hid_keynote_vertical->hid, HID_CONSUMER_AC_BACK);
                }
            }
        },
        true);
}

static bool hid_keynote_vertical_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidKeynoteVertical* hid_keynote_vertical = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_keyboard_release_all(hid_keynote_vertical->hid);
    } else {
        hid_keynote_vertical_process(hid_keynote_vertical, event);
        consumed = true;
    }

    return consumed;
}

HidKeynoteVertical* hid_keynote_vertical_alloc(Hid* hid) {
    HidKeynoteVertical* hid_keynote_vertical = malloc(sizeof(HidKeynoteVertical));
    hid_keynote_vertical->view = view_alloc();
    hid_keynote_vertical->hid = hid;
    view_set_context(hid_keynote_vertical->view, hid_keynote_vertical);
    view_allocate_model(
        hid_keynote_vertical->view, ViewModelTypeLocking, sizeof(HidKeynoteVerticalModel));
    view_set_draw_callback(hid_keynote_vertical->view, hid_keynote_vertical_draw_callback);
    view_set_input_callback(hid_keynote_vertical->view, hid_keynote_vertical_input_callback);

    with_view_model(
        hid_keynote_vertical->view,
        HidKeynoteVerticalModel * model,
        { model->transport = hid->transport; },
        true);

    return hid_keynote_vertical;
}

void hid_keynote_vertical_free(HidKeynoteVertical* hid_keynote_vertical) {
    furi_assert(hid_keynote_vertical);
    view_free(hid_keynote_vertical->view);
    free(hid_keynote_vertical);
}

View* hid_keynote_vertical_get_view(HidKeynoteVertical* hid_keynote_vertical) {
    furi_assert(hid_keynote_vertical);
    return hid_keynote_vertical->view;
}

void hid_keynote_vertical_set_connected_status(
    HidKeynoteVertical* hid_keynote_vertical,
    bool connected) {
    furi_assert(hid_keynote_vertical);
    with_view_model(
        hid_keynote_vertical->view,
        HidKeynoteVerticalModel * model,
        { model->connected = connected; },
        true);
}
