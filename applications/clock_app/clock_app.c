#include <furi.h>
#include <furi_hal.h>

#include <gui/elements.h>

#include <gui/gui.h>
#include <input/input.h>

#define TAG "Clock"
#define CLOCK_DATE_FORMAT "%.4d-%.2d-%.2d"
#define CLOCK_TIME_FORMAT "%.2d:%.2d:%.2d"

bool timerStarted=false;
int timerSecs=0;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriHalRtcDateTime datetime;
} ClockState;

static void clock_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue); 

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void clock_render_callback(Canvas* const canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    ClockState* state = (ClockState*)acquire_mutex((ValueMutex*)ctx, 25);
 
    char strings[3][20];
    int curMin = (timerSecs/60);
    int curSec = timerSecs-(curMin *60);

    sprintf(strings[0], CLOCK_DATE_FORMAT, state->datetime.year, state->datetime.month, state->datetime.day);
    sprintf(strings[1], CLOCK_TIME_FORMAT, state->datetime.hour, state->datetime.minute, state->datetime.second);
    sprintf(strings[2], "%.2d:%.2d", curMin , curSec);

    release_mutex((ValueMutex*)ctx, state);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, strings[1]);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, strings[0]);

    // elements_button_left(canvas, "Alarms");
    // elements_button_right(canvas, "Settings");
    elements_button_center(canvas, "Reset");
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, strings[2]);
    canvas_set_font(canvas, FontSecondary);
    if(timerStarted) {
        elements_button_left(canvas, "Stop");
    } else {
        elements_button_left(canvas, "Start");
    }
}

static void clock_state_init(ClockState* const state) {
    furi_hal_rtc_get_datetime(&state->datetime);
}

// Runs every 1000ms by default
static void clock_tick(void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    if(timerStarted) {
        timerSecs=timerSecs+1;
    }
    // It's OK to loose this event if system overloaded
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t clock_app_old(void* p) {
    UNUSED(p);
    timerStarted=false;
    timerSecs=0;
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(PluginEvent), NULL);
    ClockState* plugin_state = malloc(sizeof(ClockState));
    clock_state_init(plugin_state);
    ValueMutex state_mutex;
    if (!init_mutex(&state_mutex, plugin_state, sizeof(ClockState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, clock_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, clock_input_callback, event_queue);
    osTimerId_t timer = osTimerNew(clock_tick, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, osKernelGetTickFreq());

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Main loop
    PluginEvent event;
    for (bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        ClockState* plugin_state = (ClockState*)acquire_mutex_block(&state_mutex);

        if (event_status == osOK) {
            // press events
            if (event.type == EventTypeKey) {
                if (event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(timerStarted) timerSecs=timerSecs+5;
                        break;
                    case InputKeyDown:
                        if(timerStarted) timerSecs=timerSecs-5;
                        break;
                    case InputKeyRight:
                        break;
                    case InputKeyLeft:
                        if(timerStarted) {
                            timerStarted=false;
                        } else {
                            timerStarted=true;
                        }
                        break;
                    case InputKeyOk:
                        timerSecs=0;
                        break;
                    case InputKeyBack:
                        // Exit the plugin
                        processing = false;
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                furi_hal_rtc_get_datetime(&plugin_state->datetime);
            }
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    osTimerDelete(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    
    return 0;
}
