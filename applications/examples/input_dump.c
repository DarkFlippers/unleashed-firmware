#include <furi.h>
#include <api-hal.h>
#include <stdio.h>
#include <gui/gui.h>
#include <input/input.h>

typedef struct {
    InputEvent input;
} InputDumpEvent;

void input_dump_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Input dump application");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 22, "Press long back to exit");
}

void input_dump_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;
    InputDumpEvent event = {.input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

static const char* input_dump_get_key_name(InputKey key) {
    switch(key) {
    case InputKeyOk:
        return "Ok";
    case InputKeyBack:
        return "Back";
    case InputKeyLeft:
        return "Left";
    case InputKeyRight:
        return "Right";
    case InputKeyUp:
        return "Up";
    case InputKeyDown:
        return "Down";
    default:
        return "Unknown";
    }
}

static const char* input_dump_get_type_name(InputType type) {
    switch(type) {
    case InputTypePress:
        return "Press";
    case InputTypeRelease:
        return "Release";
    case InputTypeShort:
        return "Short";
    case InputTypeLong:
        return "Long";
    case InputTypeRepeat:
        return "Repeat";
    default:
        return "Unknown";
    }
}

int32_t application_input_dump(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(InputDumpEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    furi_check(view_port);
    view_port_draw_callback_set(view_port, input_dump_draw_callback, NULL);
    view_port_input_callback_set(view_port, input_dump_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    printf("[input_dump] waiting for input events\r\n");
    InputDumpEvent event;

    while(1) {
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);

        printf(
            "[input_dump] key: %s type: %s\r\n",
            input_dump_get_key_name(event.input.key),
            input_dump_get_type_name(event.input.type));

        if(event.input.type == InputTypeLong && event.input.key == InputKeyBack) {
            break;
        }
    }

    printf("[input_dump] shutting down, byebye!\r\n");

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    return 0;
}
