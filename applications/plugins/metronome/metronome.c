#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <m-string.h>
#include <stdlib.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/canvas.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "gui_extensions.h"

#define BPM_STEP_SIZE_FINE 0.5d
#define BPM_STEP_SIZE_COARSE 10.0d
#define BPM_BOUNDARY_LOW 10.0d
#define BPM_BOUNDARY_HIGH 300.0d
#define BEEP_DELAY_MS 50

#define wave_bitmap_left_width 4
#define wave_bitmap_left_height 14
static uint8_t wave_bitmap_left_bits[] =
    {0x08, 0x0C, 0x06, 0x06, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x06, 0x06, 0x0C, 0x08};

#define wave_bitmap_right_width 4
#define wave_bitmap_right_height 14
static uint8_t wave_bitmap_right_bits[] =
    {0x01, 0x03, 0x06, 0x06, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x06, 0x06, 0x03, 0x01};

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

enum OutputMode { Loud, Vibro, Silent };

typedef struct {
    double bpm;
    bool playing;
    int beats_per_bar;
    int note_length;
    int current_beat;
    enum OutputMode output_mode;
    FuriTimer* timer;
    NotificationApp* notifications;
} MetronomeState;

static void render_callback(Canvas* const canvas, void* ctx) {
    const MetronomeState* metronome_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(metronome_state == NULL) {
        return;
    }

    string_t tempStr;
    string_init(tempStr);

    canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontPrimary);

    // draw bars/beat
    string_printf(tempStr, "%d/%d", metronome_state->beats_per_bar, metronome_state->note_length);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, string_get_cstr(tempStr));
    string_reset(tempStr);

    // draw BPM value
    string_printf(tempStr, "%.2f", metronome_state->bpm);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, string_get_cstr(tempStr));
    string_reset(tempStr);

    // draw volume indicator
    // always draw first waves
    canvas_draw_xbm(
        canvas, 20, 17, wave_bitmap_left_width, wave_bitmap_left_height, wave_bitmap_left_bits);
    canvas_draw_xbm(
        canvas,
        canvas_width(canvas) - 20 - wave_bitmap_right_width,
        17,
        wave_bitmap_right_width,
        wave_bitmap_right_height,
        wave_bitmap_right_bits);
    if(metronome_state->output_mode < Silent) {
        canvas_draw_xbm(
            canvas, 16, 17, wave_bitmap_left_width, wave_bitmap_left_height, wave_bitmap_left_bits);
        canvas_draw_xbm(
            canvas,
            canvas_width(canvas) - 16 - wave_bitmap_right_width,
            17,
            wave_bitmap_right_width,
            wave_bitmap_right_height,
            wave_bitmap_right_bits);
    }
    if(metronome_state->output_mode < Vibro) {
        canvas_draw_xbm(
            canvas, 12, 17, wave_bitmap_left_width, wave_bitmap_left_height, wave_bitmap_left_bits);
        canvas_draw_xbm(
            canvas,
            canvas_width(canvas) - 12 - wave_bitmap_right_width,
            17,
            wave_bitmap_right_width,
            wave_bitmap_right_height,
            wave_bitmap_right_bits);
    }
    // draw button prompts
    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, "Slow");
    elements_button_right(canvas, "Fast");
    if(metronome_state->playing) {
        elements_button_center(canvas, "Stop ");
    } else {
        elements_button_center(canvas, "Start");
    }
    elements_button_top_left(canvas, "Push");
    elements_button_top_right(canvas, "Hold");

    // draw progress bar
    elements_progress_bar(
        canvas, 8, 36, 112, (float)metronome_state->current_beat / metronome_state->beats_per_bar);

    // cleanup
    string_clear(tempStr);
    release_mutex((ValueMutex*)ctx, metronome_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void timer_callback(void* ctx) {
    // this is where we go BEEP!
    MetronomeState* metronome_state = acquire_mutex((ValueMutex*)ctx, 25);
    metronome_state->current_beat++;
    if(metronome_state->current_beat > metronome_state->beats_per_bar) {
        metronome_state->current_beat = 1;
    }
    if(metronome_state->current_beat == 1) {
        // pronounced beat
        notification_message(metronome_state->notifications, &sequence_set_only_red_255);
        switch(metronome_state->output_mode) {
        case Loud:
            if(furi_hal_speaker_acquire(1000)) {
                furi_hal_speaker_start(440.0f, 1.0f);
            }
            break;
        case Vibro:
            notification_message(metronome_state->notifications, &sequence_set_vibro_on);
            break;
        case Silent:
            break;
        }
    } else {
        // unpronounced beat
        notification_message(metronome_state->notifications, &sequence_set_only_green_255);
        switch(metronome_state->output_mode) {
        case Loud:
            if(furi_hal_speaker_acquire(1000)) {
                furi_hal_speaker_start(220.0f, 1.0f);
            }
            break;
        case Vibro:
            notification_message(metronome_state->notifications, &sequence_set_vibro_on);
            break;
        case Silent:
            break;
        }
    };

    // this is a bit of a kludge... if we are on vibro and unpronounced, stop vibro after half the usual duration
    switch(metronome_state->output_mode) {
    case Loud:
        furi_delay_ms(BEEP_DELAY_MS);
        if(furi_hal_speaker_is_mine()) {
            furi_hal_speaker_stop();
            furi_hal_speaker_release();
        }
        break;
    case Vibro:
        if(metronome_state->current_beat == 1) {
            furi_delay_ms(BEEP_DELAY_MS);
            notification_message(metronome_state->notifications, &sequence_reset_vibro);
        } else {
            furi_delay_ms((int)BEEP_DELAY_MS / 2);
            notification_message(metronome_state->notifications, &sequence_reset_vibro);
            furi_delay_ms((int)BEEP_DELAY_MS / 2);
        }
        break;
    case Silent:
        break;
    }
    notification_message(metronome_state->notifications, &sequence_reset_rgb);

    release_mutex((ValueMutex*)ctx, metronome_state);
}

static uint32_t state_to_sleep_ticks(MetronomeState* metronome_state) {
    // calculate time between beeps
    uint32_t tps = furi_kernel_get_tick_frequency();
    double multiplier = 4.0d / metronome_state->note_length;
    double bps = (double)metronome_state->bpm / 60;
    return (uint32_t)(round(tps / bps) - ((BEEP_DELAY_MS / 1000) * tps)) * multiplier;
}

static void update_timer(MetronomeState* metronome_state) {
    if(furi_timer_is_running(metronome_state->timer)) {
        furi_timer_stop(metronome_state->timer);
        furi_timer_start(metronome_state->timer, state_to_sleep_ticks(metronome_state));
    }
}

static void increase_bpm(MetronomeState* metronome_state, double amount) {
    metronome_state->bpm += amount;
    if(metronome_state->bpm > (double)BPM_BOUNDARY_HIGH) {
        metronome_state->bpm = BPM_BOUNDARY_HIGH;
    }
    update_timer(metronome_state);
}

static void decrease_bpm(MetronomeState* metronome_state, double amount) {
    metronome_state->bpm -= amount;
    if(metronome_state->bpm < (double)BPM_BOUNDARY_LOW) {
        metronome_state->bpm = BPM_BOUNDARY_LOW;
    }
    update_timer(metronome_state);
}

static void cycle_beats_per_bar(MetronomeState* metronome_state) {
    metronome_state->beats_per_bar++;
    if(metronome_state->beats_per_bar > metronome_state->note_length) {
        metronome_state->beats_per_bar = 1;
    }
}

static void cycle_note_length(MetronomeState* metronome_state) {
    metronome_state->note_length *= 2;
    if(metronome_state->note_length > 16) {
        metronome_state->note_length = 2;
        metronome_state->beats_per_bar = 1;
    }
    update_timer(metronome_state);
}

static void cycle_output_mode(MetronomeState* metronome_state) {
    metronome_state->output_mode++;
    if(metronome_state->output_mode > Silent) {
        metronome_state->output_mode = Loud;
    }
}

static void metronome_state_init(MetronomeState* const metronome_state) {
    metronome_state->bpm = 120.0;
    metronome_state->playing = false;
    metronome_state->beats_per_bar = 4;
    metronome_state->note_length = 4;
    metronome_state->current_beat = 0;
    metronome_state->output_mode = Loud;
    metronome_state->notifications = furi_record_open(RECORD_NOTIFICATION);
}

int32_t metronome_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    MetronomeState* metronome_state = malloc(sizeof(MetronomeState));
    metronome_state_init(metronome_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, metronome_state, sizeof(MetronomeState))) {
        FURI_LOG_E("Metronome", "cannot create mutex\r\n");
        free(metronome_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    metronome_state->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, &state_mutex);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        MetronomeState* metronome_state = (MetronomeState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort) {
                    // push events
                    switch(event.input.key) {
                    case InputKeyUp:
                        cycle_beats_per_bar(metronome_state);
                        break;
                    case InputKeyDown:
                        cycle_output_mode(metronome_state);
                        break;
                    case InputKeyRight:
                        increase_bpm(metronome_state, BPM_STEP_SIZE_FINE);
                        break;
                    case InputKeyLeft:
                        decrease_bpm(metronome_state, BPM_STEP_SIZE_FINE);
                        break;
                    case InputKeyOk:
                        metronome_state->playing = !metronome_state->playing;
                        if(metronome_state->playing) {
                            furi_timer_start(
                                metronome_state->timer, state_to_sleep_ticks(metronome_state));
                        } else {
                            furi_timer_stop(metronome_state->timer);
                        }
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                } else if(event.input.type == InputTypeLong) {
                    // hold events
                    switch(event.input.key) {
                    case InputKeyUp:
                        cycle_note_length(metronome_state);
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        increase_bpm(metronome_state, BPM_STEP_SIZE_COARSE);
                        break;
                    case InputKeyLeft:
                        decrease_bpm(metronome_state, BPM_STEP_SIZE_COARSE);
                        break;
                    case InputKeyOk:
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                } else if(event.input.type == InputTypeRepeat) {
                    // repeat events
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        increase_bpm(metronome_state, BPM_STEP_SIZE_COARSE);
                        break;
                    case InputKeyLeft:
                        decrease_bpm(metronome_state, BPM_STEP_SIZE_COARSE);
                        break;
                    case InputKeyOk:
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D("Metronome", "FuriMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, metronome_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    furi_timer_free(metronome_state->timer);
    furi_record_close(RECORD_NOTIFICATION);
    free(metronome_state);

    return 0;
}
