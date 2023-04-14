#include "hid_ytshorts.h"
#include "../hid.h"
#include <gui/elements.h>

#include "hid_icons.h"

#define TAG "HidYTShorts"

struct HidYTShorts {
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
} HidYTShortsModel;

static void hid_ytshorts_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidYTShortsModel* model = context;

    // Header
    if(model->transport == HidTransportBle) {
        if(model->connected) {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
        } else {
            canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
        }
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Shorts");
    canvas_set_font(canvas, FontSecondary);

    // Keypad circles
    canvas_draw_icon(canvas, 66, 8, &I_Circles_47x47);

    // Pause
    if(model->back_mouse_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 106, 46, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 108, 48, &I_Pause_icon_9x9);
    canvas_set_color(canvas, ColorBlack);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 83, 9, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 86, 11, &I_Arr_up_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 83, 41, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 86, 44, &I_Arr_dwn_7x9);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 67, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 71, 29, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, 1);
        canvas_draw_icon(canvas, 99, 25, &I_Pressed_Button_13x13);
        canvas_set_bitmap_mode(canvas, 0);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 101, 29, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_draw_icon(canvas, 81, 23, &I_Like_pressed_17x17);
    } else {
        canvas_draw_icon(canvas, 84, 27, &I_Like_def_11x9);
    }
    // Exit
    canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
}

static void hid_ytshorts_reset_cursor(HidYTShorts* hid_ytshorts) {
    // Set cursor to the phone's left up corner
    // Delays to guarantee one packet per connection interval
    for(size_t i = 0; i < 8; i++) {
        hid_hal_mouse_move(hid_ytshorts->hid, -127, -127);
        furi_delay_ms(50);
    }
    // Move cursor from the corner
    hid_hal_mouse_move(hid_ytshorts->hid, 40, 120);
    hid_hal_mouse_move(hid_ytshorts->hid, 0, 120);
    furi_delay_ms(50);
}

static void hid_ytshorts_process_press(
    HidYTShorts* hid_ytshorts,
    HidYTShortsModel* model,
    InputEvent* event) {
    if(event->key == InputKeyUp) {
        model->up_pressed = true;
    } else if(event->key == InputKeyDown) {
        model->down_pressed = true;
    } else if(event->key == InputKeyLeft) {
        model->left_pressed = true;
        hid_hal_consumer_key_press(hid_ytshorts->hid, HID_CONSUMER_VOLUME_DECREMENT);
    } else if(event->key == InputKeyRight) {
        model->right_pressed = true;
        hid_hal_consumer_key_press(hid_ytshorts->hid, HID_CONSUMER_VOLUME_INCREMENT);
    } else if(event->key == InputKeyOk) {
        model->ok_pressed = true;
    } else if(event->key == InputKeyBack) {
        model->back_mouse_pressed = true;
    }
}

static void hid_ytshorts_process_release(
    HidYTShorts* hid_ytshorts,
    HidYTShortsModel* model,
    InputEvent* event) {
    if(event->key == InputKeyUp) {
        model->up_pressed = false;
    } else if(event->key == InputKeyDown) {
        model->down_pressed = false;
    } else if(event->key == InputKeyLeft) {
        model->left_pressed = false;
        hid_hal_consumer_key_release(hid_ytshorts->hid, HID_CONSUMER_VOLUME_DECREMENT);
    } else if(event->key == InputKeyRight) {
        model->right_pressed = false;
        hid_hal_consumer_key_release(hid_ytshorts->hid, HID_CONSUMER_VOLUME_INCREMENT);
    } else if(event->key == InputKeyOk) {
        model->ok_pressed = false;
    } else if(event->key == InputKeyBack) {
        model->back_mouse_pressed = false;
    }
}

static bool hid_ytshorts_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidYTShorts* hid_ytshorts = context;
    bool consumed = false;

    with_view_model(
        hid_ytshorts->view,
        HidYTShortsModel * model,
        {
            if(event->type == InputTypePress) {
                hid_ytshorts_process_press(hid_ytshorts, model, event);
                if(model->connected && !model->is_cursor_set) {
                    hid_ytshorts_reset_cursor(hid_ytshorts);
                    model->is_cursor_set = true;
                }
                consumed = true;
            } else if(event->type == InputTypeRelease) {
                hid_ytshorts_process_release(hid_ytshorts, model, event);
                consumed = true;
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyOk) {
                    hid_hal_mouse_press(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_press(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    consumed = true;
                } else if(event->key == InputKeyDown) {
                    // Swipe to new video
                    hid_hal_mouse_scroll(hid_ytshorts->hid, 6);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, 8);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, 10);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, 8);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, 6);
                    consumed = true;
                } else if(event->key == InputKeyUp) {
                    // Swipe to previous video
                    hid_hal_mouse_scroll(hid_ytshorts->hid, -6);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, -8);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, -10);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, -8);
                    hid_hal_mouse_scroll(hid_ytshorts->hid, -6);
                    consumed = true;
                } else if(event->key == InputKeyBack) {
                    // Pause
                    hid_hal_mouse_press(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    hid_hal_mouse_release(hid_ytshorts->hid, HID_MOUSE_BTN_LEFT);
                    furi_delay_ms(50);
                    consumed = true;
                }
            } else if(event->type == InputTypeLong) {
                if(event->key == InputKeyBack) {
                    hid_hal_consumer_key_release_all(hid_ytshorts->hid);
                    model->is_cursor_set = false;
                    consumed = false;
                }
            }
        },
        true);

    return consumed;
}

HidYTShorts* hid_ytshorts_alloc(Hid* bt_hid) {
    HidYTShorts* hid_ytshorts = malloc(sizeof(HidYTShorts));
    hid_ytshorts->hid = bt_hid;
    hid_ytshorts->view = view_alloc();
    view_set_context(hid_ytshorts->view, hid_ytshorts);
    view_allocate_model(hid_ytshorts->view, ViewModelTypeLocking, sizeof(HidYTShortsModel));
    view_set_draw_callback(hid_ytshorts->view, hid_ytshorts_draw_callback);
    view_set_input_callback(hid_ytshorts->view, hid_ytshorts_input_callback);

    with_view_model(
        hid_ytshorts->view,
        HidYTShortsModel * model,
        { model->transport = bt_hid->transport; },
        true);

    return hid_ytshorts;
}

void hid_ytshorts_free(HidYTShorts* hid_ytshorts) {
    furi_assert(hid_ytshorts);
    view_free(hid_ytshorts->view);
    free(hid_ytshorts);
}

View* hid_ytshorts_get_view(HidYTShorts* hid_ytshorts) {
    furi_assert(hid_ytshorts);
    return hid_ytshorts->view;
}

void hid_ytshorts_set_connected_status(HidYTShorts* hid_ytshorts, bool connected) {
    furi_assert(hid_ytshorts);
    with_view_model(
        hid_ytshorts->view,
        HidYTShortsModel * model,
        {
            model->connected = connected;
            model->is_cursor_set = false;
        },
        true);
}
