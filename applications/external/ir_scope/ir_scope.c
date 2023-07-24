// Author: github.com/kallanreed
#include <furi.h>
#include <furi_hal.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <furi_hal_infrared.h>
#include <gui/gui.h>

#define TAG "IR Scope"
#define COLS 128
#define ROWS 8

typedef struct {
    bool autoscale;
    uint16_t us_per_sample;
    size_t timings_cnt;
    uint32_t* timings;
    uint32_t timings_sum;
    FuriMutex* mutex;
} IRScopeState;

static void state_set_autoscale(IRScopeState* state) {
    if(state->autoscale) state->us_per_sample = state->timings_sum / (ROWS * COLS);
}

static void canvas_draw_str_outline(Canvas* canvas, int x, int y, const char* str) {
    canvas_set_color(canvas, ColorWhite);
    for(int y1 = -1; y1 <= 1; ++y1)
        for(int x1 = -1; x1 <= 1; ++x1) canvas_draw_str(canvas, x + x1, y + y1, str);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str(canvas, x, y, str);
}

static void render_callback(Canvas* canvas, void* ctx) {
    const IRScopeState* state = (IRScopeState*)ctx;

    furi_mutex_acquire(state->mutex, FuriWaitForever);

    canvas_clear(canvas);
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Draw the signal chart.
    bool on = false;
    bool done = false;
    size_t ix = 0;
    int timing_cols = -1; // Count of columns used to draw the current timing
    for(size_t row = 0; row < ROWS && !done; ++row) {
        for(size_t col = 0; col < COLS && !done; ++col) {
            done = ix >= state->timings_cnt;

            if(!done && timing_cols < 0) {
                timing_cols = state->timings[ix] / state->us_per_sample;
                on = !on;
            }

            if(timing_cols == 0) ++ix;

            int y = row * 8 + 7;
            canvas_draw_line(canvas, col, y, col, y - (on ? 5 : 0));
            --timing_cols;
        }
    }

    canvas_set_font(canvas, FontSecondary);
    if(state->autoscale)
        canvas_draw_str_outline(canvas, 100, 64, "Auto");
    else {
        char buf[20];
        snprintf(buf, sizeof(buf), "%uus", state->us_per_sample);
        canvas_draw_str_outline(canvas, 100, 64, buf);
    }

    furi_mutex_release(state->mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void ir_received_callback(void* ctx, InfraredWorkerSignal* signal) {
    furi_check(signal);
    IRScopeState* state = (IRScopeState*)ctx;

    furi_mutex_acquire(state->mutex, FuriWaitForever);

    const uint32_t* timings;
    infrared_worker_get_raw_signal(signal, &timings, &state->timings_cnt);

    if(state->timings) {
        free(state->timings);
        state->timings_sum = 0;
    }

    state->timings = malloc(state->timings_cnt * sizeof(uint32_t));

    // Copy and sum.
    for(size_t i = 0; i < state->timings_cnt; ++i) {
        state->timings[i] = timings[i];
        state->timings_sum += timings[i];
    }

    state_set_autoscale(state);

    furi_mutex_release(state->mutex);
}

int32_t ir_scope_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    furi_check(event_queue);

    if(furi_hal_infrared_is_busy()) {
        FURI_LOG_E(TAG, "Infrared is busy.");
        return -1;
    }

    IRScopeState state = {
        .autoscale = false, .us_per_sample = 200, .timings = NULL, .timings_cnt = 0, .mutex = NULL};
    state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!state.mutex) {
        FURI_LOG_E(TAG, "Cannot create mutex.");
        return -1;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InfraredWorker* worker = infrared_worker_alloc();
    infrared_worker_rx_enable_signal_decoding(worker, false);
    infrared_worker_rx_enable_blink_on_receiving(worker, true);
    infrared_worker_rx_set_received_signal_callback(worker, ir_received_callback, &state);
    infrared_worker_rx_start(worker);

    InputEvent event;
    bool processing = true;
    while(processing) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeRelease) {
                furi_mutex_acquire(state.mutex, FuriWaitForever);

                if(event.key == InputKeyBack) {
                    processing = false;
                } else if(event.key == InputKeyUp) {
                    state.us_per_sample = MIN(1000, state.us_per_sample + 25);
                    state.autoscale = false;
                } else if(event.key == InputKeyDown) {
                    state.us_per_sample = MAX(25, state.us_per_sample - 25);
                    state.autoscale = false;
                } else if(event.key == InputKeyOk) {
                    state.autoscale = !state.autoscale;
                    if(state.autoscale)
                        state_set_autoscale(&state);
                    else
                        state.us_per_sample = 200;
                }

                furi_mutex_release(state.mutex);
            }
        }
        view_port_update(view_port);
    }

    // Clean up.
    infrared_worker_rx_stop(worker);
    infrared_worker_free(worker);

    if(state.timings) free(state.timings);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(state.mutex);

    return 0;
}
