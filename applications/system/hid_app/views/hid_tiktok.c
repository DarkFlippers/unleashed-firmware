#include "hid_tiktok.h"
#include "../hid.h"
#include <gui/elements.h>

#include "hid_icons.h"

#define TAG "HidTikTok"

struct HidTikTok {
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
    bool is_cursor_set;
} HidTikTokModel;

static void hid_tiktok_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidTikTokModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "TikTok");
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
    canvas_draw_icon(canvas, 96, 12, &I_Arr_up_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 93, 41, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 96, 43, &I_Arr_dwn_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 77, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 81, 29, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 110, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 112, 29, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 91, 24, &I_Like_pressed_17x16);
        canvas_set_bitmap_mode(canvas, false);
    } else {
        canvas_draw_icon(canvas, 93, 27, &I_Like_def_13x11);
    }
    // Exit
    canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
}

static void hid_tiktok_reset_cursor(HidTikTok* hid_tiktok) {
    // Set cursor to the phone's left up corner
    // Delays to guarantee one packet per connection interval
    for(size_t i = 0; i < 8; i++) {
        hid_hal_mouse_move(hid_tiktok->hid, -127, -127);
        furi_delay_ms(50);
    }
    // Move cursor from the corner
    hid_hal_mouse_move(hid_tiktok->hid, 20, 120);
    furi_delay_ms(50);
}

static void
    hid_tiktok_process_press(HidTikTok* hid_tiktok, HidTikTokModel* model, InputEvent* event) {
    if(event->key == InputKeyUp) {
        model->up_pressed = true;
    } else if(event->key == InputKeyDown) {
        model->down_pressed = true;
    } else if(event->key == InputKeyLeft) {
        model->left_pressed = true;
        hid_hal_consumer_key_press(hid_tiktok->hid, HID_CONSUMER_VOLUME_DECREMENT);
    } else if(event->key == InputKeyRight) {
        model->right_pressed = true;
        hid_hal_consumer_key_press(hid_tiktok->hid, HID_CONSUMER_VOLUME_INCREMENT);
    } else if(event->key == InputKeyOk) {
        model->ok_pressed = true;
    }
}

static void
    hid_tiktok_process_release(HidTikTok* hid_tiktok, HidTikTokModel* model, InputEvent* event) {
    if(event->key == InputKeyUp) {
        model->up_pressed = false;
    } else if(event->key == InputKeyDown) {
        model->down_pressed = false;
    } else if(event->key == InputKeyLeft) {
        model->left_pressed = false;
        hid_hal_consumer_key_release(hid_tiktok->hid, HID_CONSUMER_VOLUME_DECREMENT);
    } else if(event->key == InputKeyRight) {
        model->right_pressed = false;
        hid_hal_consumer_key_release(hid_tiktok->hid, HID_CONSUMER_VOLUME_INCREMENT);
    } else if(event->key == InputKeyOk) {
        model->ok_pressed = false;
    }
}

static bool hid_tiktok_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidTikTok* hid_tiktok = context;
    bool consumed = false;

    with_view_model(
        hid_tiktok->view,
        HidTikTokModel * model,
        {
            if(event->type == InputTypePress) {
                hid_tiktok_process_press(hid_tiktok, model, event);
                if(model->connected && !model->is_cursor_set) {
                    hid_tiktok_reset_cursor(hid_tiktok);
                    model->is_cursor_set = true;
                }
                consumed = true;
            } else if(event->type == InputTypeRelease) {
                hid_tiktok_process_release(hid_tiktok, model, event);
                consumed = true;
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyOk) {
                    hid_hal_mouse_press(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_press(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    consumed = true;
                } else if(event->key == InputKeyUp) {
                    // Emulate up swipe
                    hid_hal_mouse_scroll(hid_tiktok->hid, -6);
                    hid_hal_mouse_scroll(hid_tiktok->hid, -12);
                    hid_hal_mouse_scroll(hid_tiktok->hid, -19);
                    hid_hal_mouse_scroll(hid_tiktok->hid, -12);
                    hid_hal_mouse_scroll(hid_tiktok->hid, -6);
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    // Emulate down swipe
                    hid_hal_mouse_scroll(hid_tiktok->hid, 6);
                    hid_hal_mouse_scroll(hid_tiktok->hid, 12);
                    hid_hal_mouse_scroll(hid_tiktok->hid, 19);
                    hid_hal_mouse_scroll(hid_tiktok->hid, 12);
                    hid_hal_mouse_scroll(hid_tiktok->hid, 6);
                    consumed = true;
                } else if(event->key == InputKeyBack) {
                    hid_hal_consumer_key_release_all(hid_tiktok->hid);
                    consumed = true;
                }
            } else if(event->type == InputTypeLong) {
                if(event->key == InputKeyBack) {
                    hid_hal_consumer_key_release_all(hid_tiktok->hid);
                    model->is_cursor_set = false;
                    consumed = false;
                }
            }
        },
        true);

    return consumed;
}

HidTikTok* hid_tiktok_alloc(Hid* bt_hid) {
    HidTikTok* hid_tiktok = malloc(sizeof(HidTikTok));
    hid_tiktok->hid = bt_hid;
    hid_tiktok->view = view_alloc();
    view_set_context(hid_tiktok->view, hid_tiktok);
    view_allocate_model(hid_tiktok->view, ViewModelTypeLocking, sizeof(HidTikTokModel));
    view_set_draw_callback(hid_tiktok->view, hid_tiktok_draw_callback);
    view_set_input_callback(hid_tiktok->view, hid_tiktok_input_callback);

    return hid_tiktok;
}

void hid_tiktok_free(HidTikTok* hid_tiktok) {
    furi_assert(hid_tiktok);
    view_free(hid_tiktok->view);
    free(hid_tiktok);
}

View* hid_tiktok_get_view(HidTikTok* hid_tiktok) {
    furi_assert(hid_tiktok);
    return hid_tiktok->view;
}

void hid_tiktok_set_connected_status(HidTikTok* hid_tiktok, bool connected) {
    furi_assert(hid_tiktok);
    with_view_model(
        hid_tiktok->view,
        HidTikTokModel * model,
        {
            model->connected = connected;
            model->is_cursor_set = false;
        },
        true);
}
