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
    bool back_mouse_pressed;
    HidTransport transport;
} HidTikTokModel;

static void hid_tiktok_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidTikTokModel* model = context;

    // Header
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "TikTok");
    canvas_set_font(canvas, FontSecondary);

    // Keypad circles
    canvas_draw_icon(canvas, 58, 3, &I_OutCircles);

    // Pause
    if(model->back_mouse_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 107, 33, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 113, 37, &I_Pause_icon_9x9);
    canvas_set_color(canvas, ColorBlack);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 68, 6, &I_S_UP);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 80, 8, &I_Arr_up_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 68, 36, &I_S_DOWN);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 80, 40, &I_Arr_dwn_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 61, 13, &I_S_LEFT);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 64, 25, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 91, 13, &I_S_RIGHT);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 95, 25, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 74, 19, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 78, 25, &I_Like_def_11x9);
    canvas_set_color(canvas, ColorBlack);

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
    hid_hal_mouse_move(hid_tiktok->hid, 40, 120);
    hid_hal_mouse_move(hid_tiktok->hid, 0, 120);
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
    } else if(event->key == InputKeyBack) {
        model->back_mouse_pressed = true;
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
    } else if(event->key == InputKeyBack) {
        model->back_mouse_pressed = false;
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
                    furi_delay_ms(25);
                    hid_hal_mouse_release(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(100);
                    hid_hal_mouse_press(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(25);
                    hid_hal_mouse_release(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    // Swipe to previous video
                    hid_hal_mouse_scroll(hid_tiktok->hid, 19);
                    consumed = true;
                } else if(event->key == InputKeyUp) {
                    // Swipe to new video
                    hid_hal_mouse_scroll(hid_tiktok->hid, -19);
                    consumed = true;
                } else if(event->key == InputKeyBack) {
                    // Pause
                    hid_hal_mouse_press(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_tiktok->hid, HID_MOUSE_BTN_LEFT);
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

    with_view_model(
        hid_tiktok->view, HidTikTokModel * model, { model->transport = bt_hid->transport; }, true);

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
