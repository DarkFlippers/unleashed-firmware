#include "bt_mouse.h"
#include "../tracking/main_loop.h"

#include <furi.h>
#include <furi_hal_bt.h>
#include <furi_hal_bt_hid.h>
#include <furi_hal_usb_hid.h>
#include <bt/bt_service/bt.h>
#include <gui/elements.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

struct BtMouse {
    View* view;
    ViewDispatcher* view_dispatcher;
    Bt* bt;
    NotificationApp* notifications;
};

#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG 20

static void bt_mouse_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Bluetooth Mouse mode");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void bt_mouse_process(BtMouse* bt_mouse, InputEvent* event) {
    with_view_model(
        bt_mouse->view,
        void* model,
        {
            UNUSED(model);
            if(event->key == InputKeyUp) {
                if(event->type == InputTypePress) {
                    furi_hal_bt_hid_mouse_press(HID_MOUSE_BTN_LEFT);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_bt_hid_mouse_release(HID_MOUSE_BTN_LEFT);
                }
            } else if(event->key == InputKeyDown) {
                if(event->type == InputTypePress) {
                    furi_hal_bt_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_bt_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    furi_hal_bt_hid_mouse_press(HID_MOUSE_BTN_WHEEL);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_bt_hid_mouse_release(HID_MOUSE_BTN_WHEEL);
                }
            }
        },
        true);
}

static bool bt_mouse_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_hal_bt_hid_mouse_release_all();
    } else {
        bt_mouse_process(bt_mouse, event);
        consumed = true;
    }

    return consumed;
}

void bt_mouse_connection_status_changed_callback(BtStatus status, void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = context;

    bool connected = (status == BtStatusConnected);
    if(connected) {
        notification_internal_message(bt_mouse->notifications, &sequence_set_blue_255);
    } else {
        notification_internal_message(bt_mouse->notifications, &sequence_reset_blue);
    }

    //with_view_model(
    //    bt_mouse->view, void * model, { model->connected = connected; }, true);
}

void bt_mouse_enter_callback(void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = context;

    bt_mouse->bt = furi_record_open(RECORD_BT);
    bt_mouse->notifications = furi_record_open(RECORD_NOTIFICATION);
    bt_set_status_changed_callback(
        bt_mouse->bt, bt_mouse_connection_status_changed_callback, bt_mouse);
    furi_assert(bt_set_profile(bt_mouse->bt, BtProfileHidKeyboard));
    furi_hal_bt_start_advertising();

    tracking_begin();

    view_dispatcher_send_custom_event(bt_mouse->view_dispatcher, 0);
}

bool bt_mouse_custom_callback(uint32_t event, void* context) {
    UNUSED(event);
    furi_assert(context);
    BtMouse* bt_mouse = context;

    tracking_step(furi_hal_bt_hid_mouse_move);

    view_dispatcher_send_custom_event(bt_mouse->view_dispatcher, 0);
    return true;
}

void bt_mouse_exit_callback(void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = context;

    tracking_end();

    notification_internal_message(bt_mouse->notifications, &sequence_reset_blue);

    furi_hal_bt_stop_advertising();
    bt_set_profile(bt_mouse->bt, BtProfileSerial);

    furi_record_close(RECORD_NOTIFICATION);
    bt_mouse->notifications = NULL;
    furi_record_close(RECORD_BT);
    bt_mouse->bt = NULL;
}

BtMouse* bt_mouse_alloc(ViewDispatcher* view_dispatcher) {
    BtMouse* bt_mouse = malloc(sizeof(BtMouse));
    bt_mouse->view = view_alloc();
    bt_mouse->view_dispatcher = view_dispatcher;
    view_set_context(bt_mouse->view, bt_mouse);
    view_set_draw_callback(bt_mouse->view, bt_mouse_draw_callback);
    view_set_input_callback(bt_mouse->view, bt_mouse_input_callback);
    view_set_enter_callback(bt_mouse->view, bt_mouse_enter_callback);
    view_set_custom_callback(bt_mouse->view, bt_mouse_custom_callback);
    view_set_exit_callback(bt_mouse->view, bt_mouse_exit_callback);
    return bt_mouse;
}

void bt_mouse_free(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    view_free(bt_mouse->view);
    free(bt_mouse);
}

View* bt_mouse_get_view(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    return bt_mouse->view;
}
