#include "ra_fa_view.h"
#include "ra_fa_worker.h"

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <float_tools.h>

#define TAG "RaFaView"

#define RSSI_MIN     (-97.0f)
#define RSSI_MAX     (-60.0f)
#define RSSI_SCALE   2.3f
#define TRIGGER_STEP 1
#define MAX_HISTORY  4

struct RaFaView {
    View* view;
    RaFaWorker* worker;
    RaFaEnv* env;
    bool locked;
    uint32_t feedback_level;
    float rssi_last;
    uint8_t selected_index;
    uint8_t max_index;
    bool show_frame;
};

typedef struct {
    uint32_t frequency;
    /** Last frequency seen above the trigger. Unlike frequency it is not cleared when the
     * signal drops, so OK still has something to act on once the remote is released. */
    uint32_t frequency_last;
    uint32_t frequency_to_save;
    float rssi;
    uint32_t history_frequency[MAX_HISTORY];
    uint8_t history_frequency_rx_count[MAX_HISTORY];
    bool signal;
    float rssi_last;
    float trigger;
    uint32_t feedback_level;
    uint8_t selected_index;
    uint8_t max_index;
    bool show_frame;
} RaFaViewModel;

static void ra_fa_view_draw_rssi(
    Canvas* canvas,
    float rssi,
    float rssi_last,
    float trigger,
    uint8_t x,
    uint8_t y) {
    // Current RSSI
    if(!float_is_equal(rssi, 0.f)) {
        if(rssi > RSSI_MAX) rssi = RSSI_MAX;
        rssi = (rssi - RSSI_MIN) / RSSI_SCALE;
        uint8_t column_number = 0;
        for(size_t i = 0; i <= (uint8_t)rssi; i++) {
            if((i + 1) % 4) {
                column_number++;
                canvas_draw_box(canvas, x + 2 * i, (y + 4) - column_number, 2, column_number);
            }
        }
    }

    // Peak RSSI
    if(!float_is_equal(rssi_last, 0.f)) {
        if(rssi_last > RSSI_MAX) rssi_last = RSSI_MAX;
        int max_x = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) * 2;
        int max_h = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) + 1;
        max_h -= (max_h / 4) + 3;
        canvas_draw_line(canvas, x + max_x + 1, y - max_h, x + max_x + 1, y + 3);
    }

    // Trigger cursor
    trigger = (trigger - RSSI_MIN) / RSSI_SCALE;
    uint8_t tr_x = (uint8_t)((float)x + (2 * trigger));
    canvas_draw_dot(canvas, tr_x, y + 4);
    canvas_draw_line(canvas, tr_x - 1, y + 5, tr_x + 1, y + 5);

    canvas_draw_line(canvas, x, y + 3, x + (RSSI_MAX - RSSI_MIN) * 2 / RSSI_SCALE, y + 3);
}

static void ra_fa_view_draw_history(Canvas* canvas, RaFaViewModel* model) {
    char buffer[16] = {0};
    const uint8_t x1 = 2;
    const uint8_t x2 = 66;
    const uint8_t y = 37;

    canvas_set_font(canvas, FontSecondary);
    uint8_t line = 0;
    bool show_frame = model->show_frame && model->max_index > 0;
    for(uint8_t i = 0; i < MAX_HISTORY; i++) {
        uint8_t current_x;
        uint8_t current_y = y + line * 11;

        if(i % 2 == 0) {
            current_x = x1;
        } else {
            current_x = x2;
            line++;
        }
        if(model->history_frequency[i]) {
            snprintf(
                buffer,
                sizeof(buffer),
                "%03ld.%03ld",
                model->history_frequency[i] / 1000000 % 1000,
                model->history_frequency[i] / 1000 % 1000);
            canvas_draw_str(canvas, current_x, current_y, buffer);
        } else {
            canvas_draw_str(canvas, current_x, current_y, "---.---");
        }
        if(model->history_frequency_rx_count[i] > 0) {
            snprintf(buffer, sizeof(buffer), "x%d", model->history_frequency_rx_count[i]);
            canvas_draw_str(canvas, current_x + 41, current_y, buffer);
        } else {
            canvas_draw_str(canvas, current_x + 41, current_y, "MHz");
        }

        if(show_frame && i == model->selected_index) {
            elements_frame(canvas, current_x - 2, current_y - 9, 63, 11);
        }
    }
}

static void ra_fa_view_draw(Canvas* canvas, RaFaViewModel* model) {
    char buffer[16] = {0};

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 20, 7, "Frequency Analyzer");

    canvas_draw_str(canvas, 33, 62, "RSSI");
    ra_fa_view_draw_rssi(canvas, model->rssi, model->rssi_last, model->trigger, 56, 57);

    ra_fa_view_draw_history(canvas, model);

    canvas_set_font(canvas, FontBigNumbers);
    snprintf(
        buffer,
        sizeof(buffer),
        "%03ld.%03ld",
        model->frequency / 1000000 % 1000,
        model->frequency / 1000 % 1000);
    if(model->signal) {
        canvas_draw_box(canvas, 4, 10, 121, 19);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_color(canvas, ColorBlack);
    }

    canvas_draw_str(canvas, 8, 26, buffer);
    canvas_draw_icon(canvas, 96, 15, &I_MHz_25x11);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    const uint8_t icon_x = 119;
    switch(model->feedback_level) {
    case RaFaFeedbackAll:
        canvas_draw_icon(canvas, icon_x, 1, &I_Volup_8x6);
        break;
    case RaFaFeedbackVibro:
        canvas_draw_icon(canvas, icon_x, 1, &I_Voldwn_6x6);
        break;
    default:
        canvas_draw_icon(canvas, icon_x, 1, &I_Voldwn_6x6);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 123, 1, 2, 6);
        canvas_set_color(canvas, ColorBlack);
        break;
    }

    elements_button_left(canvas, "T-");
    elements_button_right(canvas, "+T");
}

static void ra_fa_view_sync_model(RaFaView* instance) {
    with_view_model(
        instance->view,
        RaFaViewModel * model,
        {
            model->rssi_last = instance->rssi_last;
            model->trigger = ra_fa_worker_get_trigger_level(instance->worker);
            model->feedback_level = instance->feedback_level;
            model->max_index = instance->max_index;
            model->show_frame = instance->show_frame;
            model->selected_index = instance->selected_index;
        },
        true);
}

static bool ra_fa_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    RaFaView* instance = context;

    if(event->key == InputKeyBack) return false;

    bool need_redraw = false;
    bool is_press_or_repeat = (event->type == InputTypePress) || (event->type == InputTypeRepeat);

    if(is_press_or_repeat && (event->key == InputKeyLeft || event->key == InputKeyRight)) {
        float trigger_level = ra_fa_worker_get_trigger_level(instance->worker);
        if(event->key == InputKeyLeft) {
            trigger_level -= TRIGGER_STEP;
            if(trigger_level < RSSI_MIN) trigger_level = RSSI_MIN;
        } else {
            trigger_level += TRIGGER_STEP;
            if(trigger_level > RSSI_MAX) trigger_level = RSSI_MAX;
        }
        ra_fa_worker_set_trigger_level(instance->worker, trigger_level);
        need_redraw = true;
    } else if(event->type == InputTypePress && event->key == InputKeyUp) {
        if(instance->feedback_level == RaFaFeedbackAll) {
            instance->feedback_level = RaFaFeedbackMute;
        } else {
            instance->feedback_level--;
        }
        need_redraw = true;
    } else if(is_press_or_repeat && event->key == InputKeyDown) {
        instance->show_frame = instance->max_index > 0;
        if(instance->show_frame) {
            instance->selected_index = (instance->selected_index + 1) % instance->max_index;
            need_redraw = true;
        }
    } else if(
        (event->type == InputTypeShort || event->type == InputTypeLong) &&
        event->key == InputKeyOk) {
        bool updated = false;
        uint32_t frequency_to_save;

        with_view_model(
            instance->view,
            RaFaViewModel * model,
            {
                frequency_to_save = model->frequency_to_save;
                uint32_t prev_freq_to_save = model->frequency_to_save;
                uint32_t frequency_candidate = 0;

                if(model->show_frame && !model->signal) {
                    frequency_candidate = model->history_frequency[model->selected_index];
                } else if(model->signal) {
                    frequency_candidate =
                        ra_fa_worker_get_nearest_frequency(instance->worker, model->frequency);
                }

                /* Neither boxed nor in the history yet still leaves the last reading worth
                 * acting on, so OK works without having to keep the remote held down. */
                if(frequency_candidate == 0 && model->frequency_last != 0) {
                    frequency_candidate = ra_fa_worker_get_nearest_frequency(
                        instance->worker, model->frequency_last);
                }

                frequency_candidate =
                    (frequency_candidate == 0 ||
                     !furi_hal_subghz_is_frequency_valid(frequency_candidate) ||
                     prev_freq_to_save == frequency_candidate) ?
                        0 :
                        ra_fa_worker_get_nearest_frequency(instance->worker, frequency_candidate);

                if(frequency_candidate > 0 && frequency_candidate != model->frequency_to_save) {
                    model->frequency_to_save = frequency_candidate;
                    frequency_to_save = frequency_candidate;
                    updated = true;
                }
            },
            false);

        if(updated) {
            instance->env->frequency = frequency_to_save;
            view_dispatcher_send_custom_event(
                instance->env->view_dispatcher, instance->env->event_picked);
        }

        // Holding OK hands the frequency to the Read screen
        if(event->type == InputTypeLong && frequency_to_save > 0) {
            instance->env->frequency = frequency_to_save;
            view_dispatcher_send_custom_event(
                instance->env->view_dispatcher, instance->env->event_go_read);
        }
    }

    if(need_redraw) ra_fa_view_sync_model(instance);

    return true;
}

/** Round 299999990Hz to 300000000Hz */
static uint32_t ra_fa_view_round_int(uint32_t value, uint8_t n) {
    uint8_t on = n;
    while(n--) {
        uint8_t i = value % 10;
        value /= 10;
        if(i >= 5) value++;
    }
    while(on--)
        value *= 10;
    return value;
}

static void ra_fa_view_pair_callback(void* context, uint32_t frequency, float rssi, bool signal) {
    RaFaView* instance = context;

    if(float_is_equal(rssi, 0.f) && instance->locked) {
        view_dispatcher_send_custom_event(
            instance->env->view_dispatcher, instance->env->event_unlock);

        instance->show_frame = true;
        uint8_t max_index = instance->max_index;
        with_view_model(
            instance->view,
            RaFaViewModel * model,
            {
                bool in_array = false;
                uint32_t normal_frequency =
                    ra_fa_worker_get_nearest_frequency(instance->worker, model->frequency);
                for(size_t i = 0; i < MAX_HISTORY; i++) {
                    if(model->history_frequency[i] == normal_frequency) {
                        in_array = true;
                        if(model->history_frequency[i] > 0) {
                            if(model->history_frequency_rx_count[i] == 0) {
                                model->history_frequency_rx_count[i]++;
                            }
                            model->history_frequency_rx_count[i]++;
                        }
                        if(i > 0) {
                            size_t offset = 0;
                            uint8_t temp_rx_count = model->history_frequency_rx_count[i];

                            for(size_t j = MAX_HISTORY - 1; j > 0; j--) {
                                if(j == i) offset++;
                                model->history_frequency[j] = model->history_frequency[j - offset];
                                model->history_frequency_rx_count[j] =
                                    model->history_frequency_rx_count[j - offset];
                            }
                            model->history_frequency[0] = normal_frequency;
                            model->history_frequency_rx_count[0] = temp_rx_count;
                        }
                        break;
                    }
                }

                if(!in_array) {
                    for(size_t j = MAX_HISTORY - 1; j > 0; j--) {
                        model->history_frequency[j] = model->history_frequency[j - 1];
                        model->history_frequency_rx_count[j] =
                            model->history_frequency_rx_count[j - 1];
                    }
                    model->history_frequency[0] = normal_frequency;
                    model->history_frequency_rx_count[0] = 0;
                }

                if(max_index < MAX_HISTORY) {
                    for(size_t i = 0; i < MAX_HISTORY; i++) {
                        if(model->history_frequency[i] > 0) max_index = i + 1;
                    }
                }
            },
            false);
        instance->max_index = max_index;
    } else if(!float_is_equal(rssi, 0.f) && !instance->locked) {
        frequency = ra_fa_view_round_int(frequency, 3);
        instance->rssi_last = rssi;
        view_dispatcher_send_custom_event(
            instance->env->view_dispatcher, instance->env->event_lock);
    }

    if(rssi >= instance->rssi_last && frequency != 0) instance->rssi_last = rssi;

    instance->locked = !float_is_equal(rssi, 0.f);
    with_view_model(
        instance->view,
        RaFaViewModel * model,
        {
            model->rssi = rssi;
            model->rssi_last = instance->rssi_last;
            model->frequency = frequency;
            if(frequency != 0) {
                model->frequency_last = frequency;
            }
            model->signal = signal;
            model->trigger = ra_fa_worker_get_trigger_level(instance->worker);
            model->feedback_level = instance->feedback_level;
            model->max_index = instance->max_index;
            model->show_frame = instance->show_frame;
            model->selected_index = instance->selected_index;
        },
        true);
}

static void ra_fa_view_enter(void* context) {
    furi_assert(context);
    RaFaView* instance = context;

    instance->worker = ra_fa_worker_alloc(instance->env->setting, instance->env->trigger);
    ra_fa_worker_set_pair_callback(instance->worker, ra_fa_view_pair_callback, instance);
    ra_fa_worker_start(instance->worker);

    instance->locked = false;
    instance->rssi_last = 0;
    instance->selected_index = 0;
    instance->max_index = 0;
    instance->show_frame = false;

    with_view_model(
        instance->view,
        RaFaViewModel * model,
        {
            memset(model, 0, sizeof(RaFaViewModel));
            model->trigger = instance->env->trigger;
            model->feedback_level = instance->feedback_level;
        },
        true);
}

static void ra_fa_view_exit(void* context) {
    furi_assert(context);
    RaFaView* instance = context;

    if(instance->worker == NULL) return;

    // Read the trigger level out before the worker goes away - it is the one holding it
    instance->env->trigger = ra_fa_worker_get_trigger_level(instance->worker);
    instance->env->feedback = instance->feedback_level;

    if(ra_fa_worker_is_running(instance->worker)) ra_fa_worker_stop(instance->worker);
    ra_fa_worker_free(instance->worker);
    instance->worker = NULL;
}

RaFaView* ra_fa_view_alloc(RaFaEnv* env) {
    furi_assert(env);
    RaFaView* instance = malloc(sizeof(RaFaView));

    instance->env = env;
    instance->worker = NULL;
    instance->locked = false;
    instance->feedback_level = env->feedback;
    instance->rssi_last = 0;
    instance->selected_index = 0;
    instance->max_index = 0;
    instance->show_frame = false;

    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(RaFaViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)ra_fa_view_draw);
    view_set_input_callback(instance->view, ra_fa_view_input);
    view_set_enter_callback(instance->view, ra_fa_view_enter);
    view_set_exit_callback(instance->view, ra_fa_view_exit);

    return instance;
}

void ra_fa_view_free(RaFaView* instance) {
    furi_assert(instance);
    view_free(instance->view);
    free(instance);
}

View* ra_fa_view_get_view(RaFaView* instance) {
    furi_assert(instance);
    return instance->view;
}

void ra_fa_view_store(RaFaView* instance) {
    furi_assert(instance);
    instance->env->feedback = instance->feedback_level;
    if(instance->worker != NULL) {
        instance->env->trigger = ra_fa_worker_get_trigger_level(instance->worker);
    }
}
