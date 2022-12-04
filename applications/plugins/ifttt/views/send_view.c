#include "send_view.h"
#include <furi.h>
#include <gui/elements.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <furi_hal_uart.h>
#include <string.h>
#include <stdio.h>

#define FLIPPERZERO_SERIAL_BAUD 115200

typedef enum ESerialCommand { ESerialCommand_Send } ESerialCommand;

struct SendView {
    View* view;
};

typedef struct {
    bool right_pressed;
    bool connected;
} SendViewModel;

static void Shake(void) {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_single_vibro);
    furi_record_close(RECORD_NOTIFICATION);
}

void send_serial_command_send(ESerialCommand command) {
    uint8_t data[1] = {0};

    char name[10] = "send";
    int length = strlen(name);
    for(int i = 0; i < length; i++) {
        switch(command) {
        case ESerialCommand_Send:
            data[0] = name[i];
            break;
        default:
            return;
        };

        furi_hal_uart_tx(FuriHalUartIdUSART1, data, 1);
    }
}

static void send_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    SendViewModel* model = context;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, "SEND MODULE");
    canvas_draw_line(canvas, 0, 10, 128, 10);
    canvas_draw_str_aligned(canvas, 64, 15, AlignCenter, AlignTop, "Press right to send IFTTT");
    canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignTop, "command or press and hold");
    canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignTop, "back to return to the menu");

    // Right
    if(model->right_pressed) {
    }
}

static void send_view_process(SendView* send_view, InputEvent* event) {
    with_view_model(
        send_view->view,
        SendViewModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                } else if(event->key == InputKeyDown) {
                } else if(event->key == InputKeyLeft) {
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    Shake();
                    send_serial_command_send(ESerialCommand_Send);
                } else if(event->key == InputKeyOk) {
                } else if(event->key == InputKeyBack) {
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                } else if(event->key == InputKeyDown) {
                } else if(event->key == InputKeyLeft) {
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                } else if(event->key == InputKeyOk) {
                } else if(event->key == InputKeyBack) {
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                }
            }
        },
        true);
}

static bool send_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    SendView* send_view = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
    } else {
        send_view_process(send_view, event);
        consumed = true;
    }

    return consumed;
}

SendView* send_view_alloc() {
    SendView* send_view = malloc(sizeof(SendView));
    send_view->view = view_alloc();
    view_set_context(send_view->view, send_view);
    view_allocate_model(send_view->view, ViewModelTypeLocking, sizeof(SendViewModel));
    view_set_draw_callback(send_view->view, send_view_draw_callback);
    view_set_input_callback(send_view->view, send_view_input_callback);
    furi_hal_uart_set_br(FuriHalUartIdUSART1, FLIPPERZERO_SERIAL_BAUD);

    return send_view;
}

void send_view_free(SendView* send_view) {
    furi_assert(send_view);
    view_free(send_view->view);
    free(send_view);
}

View* send_view_get_view(SendView* send_view) {
    furi_assert(send_view);
    return send_view->view;
}

void send_view_set_data(SendView* send_view, bool connected) {
    furi_assert(send_view);
    with_view_model(
        send_view->view, SendViewModel * model, { model->connected = connected; }, true);
}