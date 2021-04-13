#include <furi.h>
#include <gui/gui.h>
#include <api-hal-version.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

typedef struct {
} State;

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;
    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    /*

    char level[20];
    uint32_t current_level = dolphin_state_get_level(dolphin->state);
    uint32_t prev_cap = dolphin_state_xp_to_levelup(dolphin->state, current_level - 1, false);
    uint32_t exp = (dolphin_state_xp_to_levelup(dolphin->state, current_level, true) * 63) /
                   (dolphin_state_xp_to_levelup(dolphin->state, current_level, false) - prev_cap);
    */
    canvas_clear(canvas);

    // multipass
    canvas_draw_icon_name(canvas, 0, 0, I_PassportLeft_6x47);
    canvas_draw_icon_name(canvas, 0, 47, I_PassportBottom_128x17);
    canvas_draw_line(canvas, 6, 0, 125, 0);
    canvas_draw_line(canvas, 127, 2, 127, 47);
    canvas_draw_dot(canvas, 126, 1);

    //portrait frame
    canvas_draw_line(canvas, 9, 6, 9, 53);
    canvas_draw_line(canvas, 10, 5, 52, 5);
    canvas_draw_line(canvas, 55, 8, 55, 53);
    canvas_draw_line(canvas, 10, 54, 54, 54);
    canvas_draw_line(canvas, 53, 5, 55, 7);

    // portrait
    canvas_draw_icon_name(canvas, 14, 11, I_DolphinOkay_41x43);
    canvas_draw_line(canvas, 59, 18, 124, 18);
    canvas_draw_line(canvas, 59, 31, 124, 31);
    canvas_draw_line(canvas, 59, 44, 124, 44);

    canvas_draw_str(canvas, 59, 15, api_hal_version_get_name_ptr());
    canvas_draw_str(canvas, 59, 28, "Mood: OK");
    /*
    snprintf(level, 20, "Level: %ld", current_level);

    canvas_draw_str(canvas, 59, 41, level);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 123 - exp, 48, exp + 1, 6);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 123 - exp, 48, 123 - exp, 54);
    */

    release_mutex((ValueMutex*)ctx, state);
}

int32_t passport(void* p) {
    State _state;
    ValueMutex state_mutex;

    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(AppEvent), NULL);
    furi_check(event_queue);

    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\r\n");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    AppEvent event;
    while(1) {
        State* state = (State*)acquire_mutex_block(&state_mutex);
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 25);
        if(event_status == osOK) {
            if(event.type == EventTypeKey && event.value.input.type == InputTypeShort) {
                release_mutex(&state_mutex, state);
                break;
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, state);
    }

    gui_remove_view_port(gui, view_port);

    view_port_free(view_port);

    furi_record_close("gui");

    delete_mutex(&state_mutex);

    osMessageQueueDelete(event_queue);

    return 0;
}
