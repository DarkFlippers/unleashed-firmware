#include "flipper_v2.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
    // add your events type
} EventType;

typedef struct {
    union {
        InputEvent input;
        // add your events payload
    } value;
    EventType type;
} Event;

typedef struct {
    // describe state here
} State;

static void render_callback(CanvasApi* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas->clear(canvas);

    // draw your app

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = (QueueHandle_t)ctx;

    Event event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void template_app(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(Event), NULL);

    State _state;
    /* init state here */
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\n");
        furiac_exit(NULL);
    }

    Widget* widget = widget_alloc();

    widget_draw_callback_set(widget, render_callback, &state_mutex);
    widget_input_callback_set(widget, input_callback, event_queue);

    // Open GUI and register widget
    GuiApi* gui = (GuiApi*)furi_open("gui");
    if(gui == NULL) {
        printf("gui is not available\n");
        furiac_exit(NULL);
    }
    gui->add_widget(gui, widget, /* specify UI layer */);

    Event event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(
            event_queue,
            &event,
            NULL,
            /* specify timeout */
        );
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                // press events
                if(event.value.input.state && event.value.input.input == InputBack) {
                }

                if(event.value.input.state && event.value.input.input == InputUp) {
                }

                if(event.value.input.state && event.value.input.input == InputDown) {
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                }

                if(event.value.input.state && event.value.input.input == InputRight) {
                }

                if(event.value.input.input == InputOk) {
                }
            }
        } else {
            // event timeout
        }

        // common code, for example, force update UI
        // widget_update(widget);

        release_mutex(&state_mutex, state);
}