#include "hid_media.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <extra_profiles/hid_profile.h>
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMedia"

struct HidMedia {
    View* view;
    Hid* hid;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool connected;
} HidMediaModel;

static void hid_media_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    canvas_draw_triangle(canvas, x, y, 5, 3, dir);
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_dot(canvas, x, y - 1);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_dot(canvas, x, y + 1);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_dot(canvas, x - 1, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_dot(canvas, x + 1, y);
    }
}

static void hid_media_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMediaModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Media");
    canvas_set_font(canvas, FontSecondary);

    // Keypad circles
    canvas_draw_icon(canvas, 75, 9, &I_Dpad_49x46);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 93, 10, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 96, 13, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 93, 41, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 96, 44, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 77, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 82, 31, CanvasDirectionRightToLeft);
    hid_media_draw_arrow(canvas, 86, 31, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 109, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 112, 31, CanvasDirectionLeftToRight);
    hid_media_draw_arrow(canvas, 116, 31, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_draw_icon(canvas, 93, 25, &I_Pressed_Button_13x13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 96, 31, CanvasDirectionLeftToRight);
    canvas_draw_line(canvas, 100, 29, 100, 33);
    canvas_draw_line(canvas, 102, 29, 102, 33);
    canvas_set_color(canvas, ColorBlack);

    // Exit
    canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
}

static void hid_media_process_press(HidMedia* hid_media, InputEvent* event) {
    with_view_model(
        hid_media->view,
        HidMediaModel * model,
        {
            if(event->key == InputKeyUp) {
                model->up_pressed = true;
                hid_hal_consumer_key_press(hid_media->hid, HID_CONSUMER_VOLUME_INCREMENT);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = true;
                hid_hal_consumer_key_press(hid_media->hid, HID_CONSUMER_VOLUME_DECREMENT);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = true;
                hid_hal_consumer_key_press(hid_media->hid, HID_CONSUMER_SCAN_PREVIOUS_TRACK);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = true;
                hid_hal_consumer_key_press(hid_media->hid, HID_CONSUMER_SCAN_NEXT_TRACK);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = true;
                hid_hal_consumer_key_press(hid_media->hid, HID_CONSUMER_PLAY_PAUSE);
            }
        },
        true);
}

static void hid_media_process_release(HidMedia* hid_media, InputEvent* event) {
    with_view_model(
        hid_media->view,
        HidMediaModel * model,
        {
            if(event->key == InputKeyUp) {
                model->up_pressed = false;
                hid_hal_consumer_key_release(hid_media->hid, HID_CONSUMER_VOLUME_INCREMENT);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = false;
                hid_hal_consumer_key_release(hid_media->hid, HID_CONSUMER_VOLUME_DECREMENT);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = false;
                hid_hal_consumer_key_release(hid_media->hid, HID_CONSUMER_SCAN_PREVIOUS_TRACK);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = false;
                hid_hal_consumer_key_release(hid_media->hid, HID_CONSUMER_SCAN_NEXT_TRACK);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = false;
                hid_hal_consumer_key_release(hid_media->hid, HID_CONSUMER_PLAY_PAUSE);
            }
        },
        true);
}

static bool hid_media_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMedia* hid_media = context;
    bool consumed = false;

    if(event->type == InputTypePress) {
        hid_media_process_press(hid_media, event);
        consumed = true;
    } else if(event->type == InputTypeRelease) {
        hid_media_process_release(hid_media, event);
        consumed = true;
    } else if(event->type == InputTypeShort) {
        consumed = true;
    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyBack) {
            hid_hal_consumer_key_release_all(hid_media->hid);
        }
    }

    return consumed;
}

HidMedia* hid_media_alloc(Hid* hid) {
    HidMedia* hid_media = malloc(sizeof(HidMedia));
    hid_media->view = view_alloc();
    hid_media->hid = hid;
    view_set_context(hid_media->view, hid_media);
    view_allocate_model(hid_media->view, ViewModelTypeLocking, sizeof(HidMediaModel));
    view_set_draw_callback(hid_media->view, hid_media_draw_callback);
    view_set_input_callback(hid_media->view, hid_media_input_callback);
    return hid_media;
}

void hid_media_free(HidMedia* hid_media) {
    furi_assert(hid_media);
    view_free(hid_media->view);
    free(hid_media);
}

View* hid_media_get_view(HidMedia* hid_media) {
    furi_assert(hid_media);
    return hid_media->view;
}

void hid_media_set_connected_status(HidMedia* hid_media, bool connected) {
    furi_assert(hid_media);
    with_view_model(
        hid_media->view, HidMediaModel * model, { model->connected = connected; }, true);
}
