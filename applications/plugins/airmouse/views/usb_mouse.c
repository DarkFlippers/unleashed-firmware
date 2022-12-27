#include "usb_mouse.h"
#include "../tracking/main_loop.h"

#include <furi.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>

struct UsbMouse {
    View* view;
    ViewDispatcher* view_dispatcher;
    FuriHalUsbInterface* usb_mode_prev;
};

static void usb_mouse_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "USB Mouse mode");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void usb_mouse_process(UsbMouse* usb_mouse, InputEvent* event) {
    with_view_model(
        usb_mouse->view,
        void* model,
        {
            UNUSED(model);
            if(event->key == InputKeyUp) {
                if(event->type == InputTypePress) {
                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
                }
            } else if(event->key == InputKeyDown) {
                if(event->type == InputTypePress) {
                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    furi_hal_hid_mouse_press(HID_MOUSE_BTN_WHEEL);
                } else if(event->type == InputTypeRelease) {
                    furi_hal_hid_mouse_release(HID_MOUSE_BTN_WHEEL);
                }
            }
        },
        true);
}

static bool usb_mouse_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    UsbMouse* usb_mouse = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        // furi_hal_hid_mouse_release_all();
    } else {
        usb_mouse_process(usb_mouse, event);
        consumed = true;
    }

    return consumed;
}

void usb_mouse_enter_callback(void* context) {
    furi_assert(context);
    UsbMouse* usb_mouse = context;

    usb_mouse->usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);

    tracking_begin();

    view_dispatcher_send_custom_event(usb_mouse->view_dispatcher, 0);
}

bool usb_mouse_move(int8_t dx, int8_t dy, void* context) {
    UNUSED(context);
    return furi_hal_hid_mouse_move(dx, dy);
}

bool usb_mouse_custom_callback(uint32_t event, void* context) {
    UNUSED(event);
    furi_assert(context);
    UsbMouse* usb_mouse = context;

    tracking_step(usb_mouse_move, context);
    furi_delay_ms(3); // Magic! Removing this will break the buttons

    view_dispatcher_send_custom_event(usb_mouse->view_dispatcher, 0);
    return true;
}

void usb_mouse_exit_callback(void* context) {
    furi_assert(context);
    UsbMouse* usb_mouse = context;

    tracking_end();

    furi_hal_usb_set_config(usb_mouse->usb_mode_prev, NULL);
}

UsbMouse* usb_mouse_alloc(ViewDispatcher* view_dispatcher) {
    UsbMouse* usb_mouse = malloc(sizeof(UsbMouse));
    usb_mouse->view = view_alloc();
    usb_mouse->view_dispatcher = view_dispatcher;
    view_set_context(usb_mouse->view, usb_mouse);
    view_set_draw_callback(usb_mouse->view, usb_mouse_draw_callback);
    view_set_input_callback(usb_mouse->view, usb_mouse_input_callback);
    view_set_enter_callback(usb_mouse->view, usb_mouse_enter_callback);
    view_set_custom_callback(usb_mouse->view, usb_mouse_custom_callback);
    view_set_exit_callback(usb_mouse->view, usb_mouse_exit_callback);
    return usb_mouse;
}

void usb_mouse_free(UsbMouse* usb_mouse) {
    furi_assert(usb_mouse);
    view_free(usb_mouse->view);
    free(usb_mouse);
}

View* usb_mouse_get_view(UsbMouse* usb_mouse) {
    furi_assert(usb_mouse);
    return usb_mouse->view;
}
