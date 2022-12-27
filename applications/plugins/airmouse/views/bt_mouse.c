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

typedef struct ButtonEvent {
    int8_t button;
    bool state;
} ButtonEvent;

#define BTN_EVT_QUEUE_SIZE 32

struct BtMouse {
    View* view;
    ViewDispatcher* view_dispatcher;
    Bt* bt;
    NotificationApp* notifications;
    FuriMutex* mutex;
    FuriThread* thread;
    bool connected;

    // Current mouse state
    uint8_t btn;
    int dx;
    int dy;
    int wheel;

    // Circular buffer;
    // (qhead == qtail) means either empty or overflow.
    // We'll ignore overflow and treat it as empty.
    int qhead;
    int qtail;
    ButtonEvent queue[BTN_EVT_QUEUE_SIZE];
};

#define BT_MOUSE_FLAG_INPUT_EVENT (1UL << 0)
#define BT_MOUSE_FLAG_KILL_THREAD (1UL << 1)
#define BT_MOUSE_FLAG_ALL (BT_MOUSE_FLAG_INPUT_EVENT | BT_MOUSE_FLAG_KILL_THREAD)

#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG 20

static void bt_mouse_notify_event(BtMouse* bt_mouse) {
    FuriThreadId thread_id = furi_thread_get_id(bt_mouse->thread);
    furi_assert(thread_id);
    furi_thread_flags_set(thread_id, BT_MOUSE_FLAG_INPUT_EVENT);
}

static void bt_mouse_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Bluetooth Mouse mode");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void bt_mouse_button_state(BtMouse* bt_mouse, int8_t button, bool state) {
    ButtonEvent event;
    event.button = button;
    event.state = state;

    if(bt_mouse->connected) {
        furi_mutex_acquire(bt_mouse->mutex, FuriWaitForever);
        bt_mouse->queue[bt_mouse->qtail++] = event;
        bt_mouse->qtail %= BTN_EVT_QUEUE_SIZE;
        furi_mutex_release(bt_mouse->mutex);
        bt_mouse_notify_event(bt_mouse);
    }
}

static void bt_mouse_process(BtMouse* bt_mouse, InputEvent* event) {
    with_view_model(
        bt_mouse->view,
        void* model,
        {
            UNUSED(model);
            if(event->key == InputKeyUp) {
                if(event->type == InputTypePress) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_LEFT, true);
                } else if(event->type == InputTypeRelease) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_LEFT, false);
                }
            } else if(event->key == InputKeyDown) {
                if(event->type == InputTypePress) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_RIGHT, true);
                } else if(event->type == InputTypeRelease) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_RIGHT, false);
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_WHEEL, true);
                } else if(event->type == InputTypeRelease) {
                    bt_mouse_button_state(bt_mouse, HID_MOUSE_BTN_WHEEL, false);
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

    bt_mouse->connected = (status == BtStatusConnected);
    if(bt_mouse->connected) {
        notification_internal_message(bt_mouse->notifications, &sequence_set_blue_255);
        tracking_begin();
        view_dispatcher_send_custom_event(bt_mouse->view_dispatcher, 0);
    } else {
        tracking_end();
        notification_internal_message(bt_mouse->notifications, &sequence_reset_blue);
    }

    //with_view_model(
    //    bt_mouse->view, void * model, { model->connected = connected; }, true);
}

bool bt_mouse_move(int8_t dx, int8_t dy, void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = context;

    if(bt_mouse->connected) {
        furi_mutex_acquire(bt_mouse->mutex, FuriWaitForever);
        bt_mouse->dx += dx;
        bt_mouse->dy += dy;
        furi_mutex_release(bt_mouse->mutex);
        bt_mouse_notify_event(bt_mouse);
    }

    return true;
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
}

bool bt_mouse_custom_callback(uint32_t event, void* context) {
    UNUSED(event);
    furi_assert(context);
    BtMouse* bt_mouse = context;

    tracking_step(bt_mouse_move, context);
    furi_delay_ms(3); // Magic! Removing this will break the buttons

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

static int8_t clamp(int t) {
    if(t < -128) {
        return -128;
    } else if(t > 127) {
        return 127;
    }
    return t;
}

static int32_t bt_mouse_thread_callback(void* context) {
    furi_assert(context);
    BtMouse* bt_mouse = (BtMouse*)context;

    while(1) {
        uint32_t flags =
            furi_thread_flags_wait(BT_MOUSE_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        if(flags & BT_MOUSE_FLAG_KILL_THREAD) {
            break;
        }
        if(flags & BT_MOUSE_FLAG_INPUT_EVENT) {
            furi_mutex_acquire(bt_mouse->mutex, FuriWaitForever);

            ButtonEvent event;
            bool send_buttons = false;
            if(bt_mouse->qhead != bt_mouse->qtail) {
                event = bt_mouse->queue[bt_mouse->qhead++];
                bt_mouse->qhead %= BTN_EVT_QUEUE_SIZE;
                send_buttons = true;
            }

            int8_t dx = clamp(bt_mouse->dx);
            bt_mouse->dx -= dx;
            int8_t dy = clamp(bt_mouse->dy);
            bt_mouse->dy -= dy;
            int8_t wheel = clamp(bt_mouse->wheel);
            bt_mouse->wheel -= wheel;

            furi_mutex_release(bt_mouse->mutex);

            if(bt_mouse->connected && send_buttons) {
                if(event.state) {
                    furi_hal_bt_hid_mouse_press(event.button);
                } else {
                    furi_hal_bt_hid_mouse_release(event.button);
                }
            }

            if(bt_mouse->connected && (dx != 0 || dy != 0)) {
                furi_hal_bt_hid_mouse_move(dx, dy);
            }

            if(bt_mouse->connected && wheel != 0) {
                furi_hal_bt_hid_mouse_scroll(wheel);
            }
        }
    }

    return 0;
}

void bt_mouse_thread_start(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    bt_mouse->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    bt_mouse->thread = furi_thread_alloc();
    furi_thread_set_name(bt_mouse->thread, "BtSender");
    furi_thread_set_stack_size(bt_mouse->thread, 1024);
    furi_thread_set_context(bt_mouse->thread, bt_mouse);
    furi_thread_set_callback(bt_mouse->thread, bt_mouse_thread_callback);
    furi_thread_start(bt_mouse->thread);
}

void bt_mouse_thread_stop(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    FuriThreadId thread_id = furi_thread_get_id(bt_mouse->thread);
    furi_assert(thread_id);
    furi_thread_flags_set(thread_id, BT_MOUSE_FLAG_KILL_THREAD);
    furi_thread_join(bt_mouse->thread);
    furi_thread_free(bt_mouse->thread);
    furi_mutex_free(bt_mouse->mutex);
}

BtMouse* bt_mouse_alloc(ViewDispatcher* view_dispatcher) {
    BtMouse* bt_mouse = malloc(sizeof(BtMouse));
    memset(bt_mouse, 0, sizeof(BtMouse));

    bt_mouse->view = view_alloc();
    bt_mouse->view_dispatcher = view_dispatcher;
    view_set_context(bt_mouse->view, bt_mouse);
    view_set_draw_callback(bt_mouse->view, bt_mouse_draw_callback);
    view_set_input_callback(bt_mouse->view, bt_mouse_input_callback);
    view_set_enter_callback(bt_mouse->view, bt_mouse_enter_callback);
    view_set_custom_callback(bt_mouse->view, bt_mouse_custom_callback);
    view_set_exit_callback(bt_mouse->view, bt_mouse_exit_callback);
    bt_mouse_thread_start(bt_mouse);
    return bt_mouse;
}

void bt_mouse_free(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    bt_mouse_thread_stop(bt_mouse);
    view_free(bt_mouse->view);
    free(bt_mouse);
}

View* bt_mouse_get_view(BtMouse* bt_mouse) {
    furi_assert(bt_mouse);
    return bt_mouse->view;
}
