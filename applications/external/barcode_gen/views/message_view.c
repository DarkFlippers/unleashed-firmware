#include "../barcode_app.h"
#include "message_view.h"

static void app_draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);

    MessageViewModel* message_view_model = ctx;

    canvas_clear(canvas);
    if(message_view_model->message != NULL) {
        canvas_draw_str_aligned(
            canvas, 62, 30, AlignCenter, AlignCenter, message_view_model->message);
    }

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 100, 52, 28, 12);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_str_aligned(canvas, 114, 58, AlignCenter, AlignCenter, "OK");
}

static bool app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    MessageView* message_view_object = ctx;

    if(input_event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(
            message_view_object->barcode_app->view_dispatcher, MainMenuView);
    }
    if(input_event->type == InputTypeShort) {
        if(input_event->key == InputKeyOk) {
            view_dispatcher_switch_to_view(
                message_view_object->barcode_app->view_dispatcher, MainMenuView);
        }
    }

    return true;
}

MessageView* message_view_allocate(BarcodeApp* barcode_app) {
    furi_assert(barcode_app);

    MessageView* message_view_object = malloc(sizeof(MessageView));

    message_view_object->view = view_alloc();
    message_view_object->barcode_app = barcode_app;

    view_set_context(message_view_object->view, message_view_object);
    view_allocate_model(message_view_object->view, ViewModelTypeLocking, sizeof(MessageViewModel));
    view_set_draw_callback(message_view_object->view, app_draw_callback);
    view_set_input_callback(message_view_object->view, app_input_callback);

    return message_view_object;
}

void message_view_free(MessageView* message_view_object) {
    furi_assert(message_view_object);

    view_free(message_view_object->view);
    free(message_view_object);
}

View* message_get_view(MessageView* message_view_object) {
    furi_assert(message_view_object);
    return message_view_object->view;
}