#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

void vibro_test_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Vibro application");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Press OK turns on vibro");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 34, "Release OK turns off vibro");
}

void vibro_test_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;
    osMessageQueuePut(event_queue, input_event, 0, osWaitForever);
}

int32_t vibro_test_app(void* p) {
    UNUSED(p);
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, vibro_test_draw_callback, NULL);
    view_port_input_callback_set(view_port, vibro_test_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notification = furi_record_open("notification");

    InputEvent event;

    while(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            notification_message(notification, &sequence_reset_vibro);
            notification_message(notification, &sequence_reset_green);
            break;
        }
        if(event.key == InputKeyOk) {
            if(event.type == InputTypePress) {
                notification_message(notification, &sequence_set_vibro_on);
                notification_message(notification, &sequence_set_green_255);
            } else if(event.type == InputTypeRelease) {
                notification_message(notification, &sequence_reset_vibro);
                notification_message(notification, &sequence_reset_green);
            }
        }
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    furi_record_close("notification");
    furi_record_close("gui");

    return 0;
}
