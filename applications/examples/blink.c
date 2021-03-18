#include <furi.h>
#include <api-hal.h>

#include <gui/gui.h>
#include <input/input.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} BlinkEvent;

void rgb_set(bool r, bool g, bool b) {
    api_hal_light_set(LightRed, r ? 0xFF : 0x00);
    api_hal_light_set(LightGreen, g ? 0xFF : 0x00);
    api_hal_light_set(LightBlue, b ? 0xFF : 0x00);
}

void blink_update(void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    BlinkEvent event = {.type = EventTypeTick};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void blink_draw_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Blink application");
}

void blink_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;

    BlinkEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t application_blink(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(BlinkEvent), NULL);

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    furi_check(view_port);
    view_port_draw_callback_set(view_port, blink_draw_callback, NULL);
    view_port_input_callback_set(view_port, blink_input_callback, event_queue);
    osTimerId_t timer = osTimerNew(blink_update, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, 500);

    // Register view port in GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool blink_color[][3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 0},
        {0, 1, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 0, 0},
    };
    uint8_t state = 0;
    BlinkEvent event;

    while(1) {
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.type == EventTypeKey) {
            if((event.input.type == InputTypeShort) && (event.input.key == InputKeyBack)) {
                rgb_set(0, 0, 0);
                view_port_enabled_set(view_port, false);
                gui_remove_view_port(gui, view_port);
                view_port_free(view_port);
                osMessageQueueDelete(event_queue);
                osTimerDelete(timer);

                return 0;
            }
        } else {
            if(state < sizeof(blink_color) / sizeof(blink_color[0])) {
                state++;
            } else {
                state = 0;
            }
            rgb_set(blink_color[state][0], blink_color[state][1], blink_color[state][2]);
        }
    }

    return 0;
}