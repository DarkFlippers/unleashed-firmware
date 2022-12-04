#include <furi.h>
#include <furi_hal.h>
#include <dialogs/dialogs.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include "BPM_Tapper_icons.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

//QUEUE

struct node {
    int interval;
    struct node* next;
};
typedef struct node node;

typedef struct {
    int size;
    int max_size;
    node* front;
    node* rear;
} queue;

static void init_queue(queue* q) {
    q->size = 0;
    q->max_size = 8;
    q->front = NULL;
    q->rear = NULL;
}

static void queue_remove(queue* q) {
    node* tmp;
    tmp = q->front;
    q->front = q->front->next;
    q->size--;
    free(tmp);
}

static void queue_add(queue* q, int value) {
    node* tmp = malloc(sizeof(node));
    tmp->interval = value;
    tmp->next = NULL;
    if(q->size == q->max_size) {
        queue_remove(q);
    }
    // check if empty
    if(q->rear == NULL) {
        q->front = tmp;
        q->rear = tmp;
    } else {
        q->rear->next = tmp;
        q->rear = tmp;
    }
    q->size++;
}

static float queue_avg(queue* q) {
    float avg = 0.0;
    if(q->size == 0) {
        return avg;
    } else {
        node* tmp;
        float sum = 0.0;
        tmp = q->front;
        while(tmp != NULL) {
            sum = sum + tmp->interval;
            tmp = tmp->next;
        }
        avg = sum / q->size;
        FURI_LOG_D("BPM-Tapper", "Sum: %.2f Avg: %.2f", (double)sum, (double)avg);
        return avg;
    }
}

// TOO SLOW!
//uint64_t dolphin_state_timestamp() {
//    FuriHalRtcDateTime datetime;
//    furi_hal_rtc_get_datetime(&datetime);
//    return furi_hal_rtc_datetime_to_timestamp(&datetime);
//}
//
typedef struct {
    int taps;
    double bpm;
    uint32_t last_stamp;
    uint32_t interval;
    queue* tap_queue;
} BPMTapper;

static void show_hello() {
    // BEGIN HELLO DIALOG
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();

    const char* header_text = "BPM Tapper";
    const char* message_text = "Tap center to start";

    dialog_message_set_header(message, header_text, 63, 3, AlignCenter, AlignTop);
    dialog_message_set_text(message, message_text, 0, 17, AlignLeft, AlignTop);
    dialog_message_set_buttons(message, NULL, "Tap", NULL);

    dialog_message_set_icon(message, &I_DolphinCommon_56x48, 72, 17);

    dialog_message_show(dialogs, message);

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);
    // END HELLO DIALOG
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void render_callback(Canvas* const canvas, void* ctx) {
    FuriString* tempStr;

    const BPMTapper* bpm_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(bpm_state == NULL) {
        return;
    }
    // border
    //canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_set_font(canvas, FontPrimary);

    tempStr = furi_string_alloc();

    furi_string_printf(tempStr, "Taps: %d", bpm_state->taps);
    canvas_draw_str_aligned(canvas, 5, 10, AlignLeft, AlignBottom, furi_string_get_cstr(tempStr));
    furi_string_reset(tempStr);

    furi_string_printf(tempStr, "Queue: %d", bpm_state->tap_queue->size);
    canvas_draw_str_aligned(canvas, 70, 10, AlignLeft, AlignBottom, furi_string_get_cstr(tempStr));
    furi_string_reset(tempStr);

    furi_string_printf(tempStr, "Interval: %ldms", bpm_state->interval);
    canvas_draw_str_aligned(canvas, 5, 20, AlignLeft, AlignBottom, furi_string_get_cstr(tempStr));
    furi_string_reset(tempStr);

    furi_string_printf(tempStr, "x2 %.2f /2 %.2f", bpm_state->bpm * 2, bpm_state->bpm / 2);
    canvas_draw_str_aligned(
        canvas, 64, 60, AlignCenter, AlignCenter, furi_string_get_cstr(tempStr));
    furi_string_reset(tempStr);

    furi_string_printf(tempStr, "%.2f", bpm_state->bpm);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas, 64, 40, AlignCenter, AlignCenter, furi_string_get_cstr(tempStr));
    furi_string_reset(tempStr);

    furi_string_free(tempStr);

    release_mutex((ValueMutex*)ctx, bpm_state);
}

static void bpm_state_init(BPMTapper* const plugin_state) {
    plugin_state->taps = 0;
    plugin_state->bpm = 120.0;
    plugin_state->last_stamp = 0; // furi_get_tick();
    plugin_state->interval = 0;
    queue* q;
    q = malloc(sizeof(queue));
    init_queue(q);
    plugin_state->tap_queue = q;
}

int32_t bpm_tapper_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    BPMTapper* bpm_state = malloc(sizeof(BPMTapper));
    // setup
    bpm_state_init(bpm_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, bpm_state, sizeof(bpm_state))) {
        FURI_LOG_E("BPM-Tapper", "cannot create mutex\r\n");
        free(bpm_state);
        return 255;
    }
    show_hello();

    // BEGIN IMPLEMENTATION

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        BPMTapper* bpm_state = (BPMTapper*)acquire_mutex_block(&state_mutex);
        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                    case InputKeyRight:
                    case InputKeyLeft:
                    case InputKeyOk:
                        bpm_state->taps++;
                        uint32_t new_stamp = furi_get_tick();
                        if(bpm_state->last_stamp == 0) {
                            bpm_state->last_stamp = new_stamp;
                            break;
                        }
                        bpm_state->interval = new_stamp - bpm_state->last_stamp;
                        bpm_state->last_stamp = new_stamp;
                        queue_add(bpm_state->tap_queue, bpm_state->interval);
                        float avg = queue_avg(bpm_state->tap_queue);
                        float bps = 1.0 / (avg / 1000.0);
                        bpm_state->bpm = bps * 60.0;
                        break;
                    case InputKeyBack:
                        // Exit the plugin
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D("BPM-Tapper", "FuriMessageQueue: event timeout");
            // event timeout
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, bpm_state);
    }
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    queue* q = bpm_state->tap_queue;
    free(q);
    free(bpm_state);

    return 0;
}
