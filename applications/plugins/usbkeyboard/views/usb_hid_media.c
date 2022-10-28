#include "usb_hid_media.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>
#include <USB_Keyboard_icons.h>

struct UsbHidMedia {
    View* view;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool connected;
} UsbHidMediaModel;

static void usb_hid_media_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
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

static void usb_hid_media_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    UsbHidMediaModel* model = context;

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Media");
    canvas_set_font(canvas, FontSecondary);

    // Keypad circles
    canvas_draw_icon(canvas, 76, 8, &I_Circles_47x47);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 93, 9, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 96, 12, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 93, 41, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 96, 45, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 77, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_media_draw_arrow(canvas, 82, 31, CanvasDirectionRightToLeft);
    usb_hid_media_draw_arrow(canvas, 86, 31, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 109, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_media_draw_arrow(canvas, 112, 31, CanvasDirectionLeftToRight);
    usb_hid_media_draw_arrow(canvas, 116, 31, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_draw_icon(canvas, 93, 25, &I_Pressed_Button_13x13);
        canvas_set_color(canvas, ColorWhite);
    }
    usb_hid_media_draw_arrow(canvas, 96, 31, CanvasDirectionLeftToRight);
    canvas_draw_line(canvas, 100, 29, 100, 33);
    canvas_draw_line(canvas, 102, 29, 102, 33);
    canvas_set_color(canvas, ColorBlack);

    // Exit
    canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
}

static void usb_hid_media_process_press(UsbHidMedia* usb_hid_media, InputEvent* event) {
    with_view_model(
        usb_hid_media->view,
        UsbHidMediaModel * model,
        {
            if(event->key == InputKeyUp) {
                model->up_pressed = true;
                furi_hal_hid_consumer_key_press(HID_CONSUMER_VOLUME_INCREMENT);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = true;
                furi_hal_hid_consumer_key_press(HID_CONSUMER_VOLUME_DECREMENT);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = true;
                furi_hal_hid_consumer_key_press(HID_CONSUMER_SCAN_PREVIOUS_TRACK);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = true;
                furi_hal_hid_consumer_key_press(HID_CONSUMER_SCAN_NEXT_TRACK);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = true;
                furi_hal_hid_consumer_key_press(HID_CONSUMER_PLAY_PAUSE);
            }
        },
        true);
}

static void hid_media_process_release(UsbHidMedia* usb_hid_media, InputEvent* event) {
    with_view_model(
        usb_hid_media->view,
        UsbHidMediaModel * model,
        {
            if(event->key == InputKeyUp) {
                model->up_pressed = false;
                furi_hal_hid_consumer_key_release(HID_CONSUMER_VOLUME_INCREMENT);
            } else if(event->key == InputKeyDown) {
                model->down_pressed = false;
                furi_hal_hid_consumer_key_release(HID_CONSUMER_VOLUME_DECREMENT);
            } else if(event->key == InputKeyLeft) {
                model->left_pressed = false;
                furi_hal_hid_consumer_key_release(HID_CONSUMER_SCAN_PREVIOUS_TRACK);
            } else if(event->key == InputKeyRight) {
                model->right_pressed = false;
                furi_hal_hid_consumer_key_release(HID_CONSUMER_SCAN_NEXT_TRACK);
            } else if(event->key == InputKeyOk) {
                model->ok_pressed = false;
                furi_hal_hid_consumer_key_release(HID_CONSUMER_PLAY_PAUSE);
            }
        },
        true);
}

static bool usb_hid_media_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    UsbHidMedia* usb_hid_media = context;
    bool consumed = false;

    if(event->type == InputTypePress) {
        usb_hid_media_process_press(usb_hid_media, event);
        consumed = true;
    } else if(event->type == InputTypeRelease) {
        hid_media_process_release(usb_hid_media, event);
        consumed = true;
    } else if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_hal_hid_kb_release_all();
        }
    }

    return consumed;
}

UsbHidMedia* usb_hid_media_alloc() {
    UsbHidMedia* usb_hid_media = malloc(sizeof(UsbHidMedia));
    usb_hid_media->view = view_alloc();
    view_set_context(usb_hid_media->view, usb_hid_media);
    view_allocate_model(usb_hid_media->view, ViewModelTypeLocking, sizeof(UsbHidMediaModel));
    view_set_draw_callback(usb_hid_media->view, usb_hid_media_draw_callback);
    view_set_input_callback(usb_hid_media->view, usb_hid_media_input_callback);

    with_view_model(
        usb_hid_media->view, UsbHidMediaModel * model, { model->connected = true; }, true);

    return usb_hid_media;
}

void usb_hid_media_free(UsbHidMedia* usb_hid_media) {
    furi_assert(usb_hid_media);
    view_free(usb_hid_media->view);
    free(usb_hid_media);
}

View* usb_hid_media_get_view(UsbHidMedia* usb_hid_media) {
    furi_assert(usb_hid_media);
    return usb_hid_media->view;
}

void usb_hid_media_set_connected_status(UsbHidMedia* usb_hid_media, bool connected) {
    furi_assert(usb_hid_media);
    with_view_model(
        usb_hid_media->view, UsbHidMediaModel * model, { model->connected = connected; }, true);
}
