#include "hid_keynote.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidKeynote"

struct HidKeynote {
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
} HidKeynoteModel;

static void hid_keynote_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
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

static void hid_keynote_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidKeynoteModel* model = context;

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

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "Hold to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, 30, 30, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, 30, 55, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 45, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, 7, 53, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 45, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, 53, 53, CanvasDirectionLeftToRight);
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

static void hid_keynote_draw_vertical_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidKeynoteModel* model = context;

    // Header
    canvas_set_font(canvas, FontPrimary);
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }

        elements_multiline_text_aligned(canvas, 20, 3, AlignLeft, AlignTop, "Keynote");
    } else {
        elements_multiline_text_aligned(canvas, 12, 3, AlignLeft, AlignTop, "Keynote");
    }

    canvas_draw_icon(canvas, 2, 18, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 15, 19, AlignLeft, AlignTop, "Hold to exit");

    const uint8_t x_2 = 23;
    const uint8_t x_1 = 2;
    const uint8_t x_3 = 44;

    const uint8_t y_1 = 44;
    const uint8_t y_2 = 65;

    // Up
    canvas_draw_icon(canvas, x_2, y_1, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_1 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, x_2 + 9, y_1 + 6, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, x_2, y_2, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, x_2 + 9, y_2 + 10, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, x_1, y_2, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, x_1 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, x_1 + 7, y_2 + 8, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, x_3, y_2, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, x_3 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_keynote_draw_arrow(canvas, x_3 + 11, y_2 + 8, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 2, 86, &I_Space_60x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 5, 88, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 11, 90, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 26, 98, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 2, 107, &I_Space_60x18);
    if(model->back_pressed) {
        elements_slightly_rounded_box(canvas, 5, 109, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 11, 111, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 26, 119, AlignLeft, AlignBottom, "Back");
}

static void hid_keynote_process(HidKeynote* hid_keynote, InputEvent* event) {
    with_view_model(
        hid_keynote->view,
        HidKeynoteModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    hid_hal_keyboard_press(hid_keynote->hid, HID_KEYBOARD_DELETE);
                    hid_hal_keyboard_release(hid_keynote->hid, HID_KEYBOARD_DELETE);
                    hid_hal_consumer_key_press(hid_keynote->hid, HID_CONSUMER_AC_BACK);
                    hid_hal_consumer_key_release(hid_keynote->hid, HID_CONSUMER_AC_BACK);
                }
            }
        },
        true);
}

static bool hid_keynote_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidKeynote* hid_keynote = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_keyboard_release_all(hid_keynote->hid);
    } else {
        hid_keynote_process(hid_keynote, event);
        consumed = true;
    }

    return consumed;
}

HidKeynote* hid_keynote_alloc(Hid* hid) {
    HidKeynote* hid_keynote = malloc(sizeof(HidKeynote));
    hid_keynote->view = view_alloc();
    hid_keynote->hid = hid;
    view_set_context(hid_keynote->view, hid_keynote);
    view_allocate_model(hid_keynote->view, ViewModelTypeLocking, sizeof(HidKeynoteModel));
    view_set_draw_callback(hid_keynote->view, hid_keynote_draw_callback);
    view_set_input_callback(hid_keynote->view, hid_keynote_input_callback);

    with_view_model(
        hid_keynote->view, HidKeynoteModel * model, { model->transport = hid->transport; }, true);

    return hid_keynote;
}

void hid_keynote_free(HidKeynote* hid_keynote) {
    furi_assert(hid_keynote);
    view_free(hid_keynote->view);
    free(hid_keynote);
}

View* hid_keynote_get_view(HidKeynote* hid_keynote) {
    furi_assert(hid_keynote);
    return hid_keynote->view;
}

void hid_keynote_set_connected_status(HidKeynote* hid_keynote, bool connected) {
    furi_assert(hid_keynote);
    with_view_model(
        hid_keynote->view, HidKeynoteModel * model, { model->connected = connected; }, true);
}

void hid_keynote_set_orientation(HidKeynote* hid_keynote, bool vertical) {
    furi_assert(hid_keynote);

    if(vertical) {
        view_set_draw_callback(hid_keynote->view, hid_keynote_draw_vertical_callback);
        view_set_orientation(hid_keynote->view, ViewOrientationVerticalFlip);

    } else {
        view_set_draw_callback(hid_keynote->view, hid_keynote_draw_callback);
        view_set_orientation(hid_keynote->view, ViewOrientationHorizontal);
    }
}
