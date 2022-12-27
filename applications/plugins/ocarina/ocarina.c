#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#define NOTE_UP 587.33f
#define NOTE_LEFT 493.88f
#define NOTE_RIGHT 440.00f
#define NOTE_DOWN 349.23
#define NOTE_OK 293.66f

typedef struct {
    FuriMutex* model_mutex;

    FuriMessageQueue* event_queue;

    ViewPort* view_port;
    Gui* gui;
} Ocarina;

void draw_callback(Canvas* canvas, void* ctx) {
    Ocarina* ocarina = ctx;
    furi_check(furi_mutex_acquire(ocarina->model_mutex, FuriWaitForever) == FuriStatusOk);

    //canvas_draw_box(canvas, ocarina->model->x, ocarina->model->y, 4, 4);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_str(canvas, 50, 10, "Ocarina");
    canvas_draw_str(canvas, 30, 20, "OK button for A");

    furi_mutex_release(ocarina->model_mutex);
}

void input_callback(InputEvent* input, void* ctx) {
    Ocarina* ocarina = ctx;
    // Puts input onto event queue with priority 0, and waits until completion.
    furi_message_queue_put(ocarina->event_queue, input, FuriWaitForever);
}

Ocarina* ocarina_alloc() {
    Ocarina* instance = malloc(sizeof(Ocarina));

    instance->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    instance->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, draw_callback, instance);
    view_port_input_callback_set(instance->view_port, input_callback, instance);

    instance->gui = furi_record_open("gui");
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    return instance;
}

void ocarina_free(Ocarina* instance) {
    view_port_enabled_set(instance->view_port, false); // Disabsles our ViewPort
    gui_remove_view_port(instance->gui, instance->view_port); // Removes our ViewPort from the Gui
    furi_record_close("gui"); // Closes the gui record
    view_port_free(instance->view_port); // Frees memory allocated by view_port_alloc
    furi_message_queue_free(instance->event_queue);

    furi_mutex_free(instance->model_mutex);

    if(furi_hal_speaker_is_mine()) {
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
    }

    free(instance);
}

int32_t ocarina_app(void* p) {
    UNUSED(p);

    Ocarina* ocarina = ocarina_alloc();

    InputEvent event;
    for(bool processing = true; processing;) {
        // Pops a message off the queue and stores it in `event`.
        // No message priority denoted by NULL, and 100 ticks of timeout.
        FuriStatus status = furi_message_queue_get(ocarina->event_queue, &event, 100);
        furi_check(furi_mutex_acquire(ocarina->model_mutex, FuriWaitForever) == FuriStatusOk);

        float volume = 1.0f;
        if(status == FuriStatusOk) {
            if(event.type == InputTypePress) {
                switch(event.key) {
                case InputKeyUp:
                    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                        furi_hal_speaker_start(NOTE_UP, volume);
                    }
                    break;
                case InputKeyDown:
                    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                        furi_hal_speaker_start(NOTE_DOWN, volume);
                    }
                    break;
                case InputKeyLeft:
                    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                        furi_hal_speaker_start(NOTE_LEFT, volume);
                    }
                    break;
                case InputKeyRight:
                    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                        furi_hal_speaker_start(NOTE_RIGHT, volume);
                    }
                    break;
                case InputKeyOk:
                    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                        furi_hal_speaker_start(NOTE_OK, volume);
                    }
                    break;
                case InputKeyBack:
                    processing = false;
                    break;
                default:
                    break;
                }
            } else if(event.type == InputTypeRelease) {
                if(furi_hal_speaker_is_mine()) {
                    furi_hal_speaker_stop();
                    furi_hal_speaker_release();
                }
            }
        }

        furi_mutex_release(ocarina->model_mutex);
        view_port_update(ocarina->view_port); // signals our draw callback
    }
    ocarina_free(ocarina);
    return 0;
}
