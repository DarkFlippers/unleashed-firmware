#include "subghz_frequency_analyzer.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <gui/elements.h>
#include "../helpers/subghz_frequency_analyzer_worker.h"

#include <assets_icons.h>

#define TAG "frequency_analyzer"

#define RSSI_MIN -97
#define RSSI_MAX -60
#define RSSI_SCALE 2
#define TRIGGER_STEP 1

static const NotificationSequence sequence_hw_blink = {
    &message_blink_start_10,
    &message_blink_set_color_cyan,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence sequence_hw_blink_stop = {
    &message_blink_stop,
    NULL,
};

static const NotificationSequence sequence_saved = {
    &message_blink_stop,
    &message_blue_0,
    &message_green_255,
    &message_red_0,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};
//static const NotificationSequence sequence_not_saved = {
//    &message_blink_stop,
//    &message_green_255,
//    &message_blue_255,
//    &message_red_255,
//    NULL,
//};

static const uint32_t subghz_frequency_list[] = {
    300000000, 302757000, 303875000, 304250000, 307000000, 307500000, 307800000,
    309000000, 310000000, 312000000, 312100000, 313000000, 313850000, 314000000,
    314350000, 315000000, 318000000, 345000000, 348000000, 387000000, 390000000,
    418000000, 433075000, 433220000, 433420000, 433657070, 433889000, 433920000,
    434176948, 434420000, 434775000, 438900000, 464000000, 779000000, 868350000,
    868400000, 868950000, 906400000, 915000000, 925000000, 928000000};

typedef enum {
    SubGhzFrequencyAnalyzerStatusIDLE,
} SubGhzFrequencyAnalyzerStatus;

struct SubGhzFrequencyAnalyzer {
    View* view;
    SubGhzFrequencyAnalyzerWorker* worker;
    SubGhzFrequencyAnalyzerCallback callback;
    void* context;
    bool locked;
    uint8_t feedback_level; // 0 - no feedback, 1 - vibro only, 2 - vibro and sound
    float rssi_last;
    uint32_t frequency_last;
    uint32_t frequency_last_vis;
    NotificationApp* notifications;
};

typedef struct {
    uint32_t frequency;
    uint32_t frequency_last;
    uint32_t frequency_to_save;
    float rssi;
    float rssi_last;
    float trigger;
    uint8_t feedback_level;
} SubGhzFrequencyAnalyzerModel;

void subghz_frequency_analyzer_set_callback(
    SubGhzFrequencyAnalyzer* subghz_frequency_analyzer,
    SubGhzFrequencyAnalyzerCallback callback,
    void* context) {
    furi_assert(subghz_frequency_analyzer);
    furi_assert(callback);
    subghz_frequency_analyzer->callback = callback;
    subghz_frequency_analyzer->context = context;
}

void subghz_frequency_analyzer_draw_rssi(
    Canvas* canvas,
    float rssi,
    float rssi_last,
    float trigger,
    uint8_t x,
    uint8_t y) {
    // Current RSSI
    if(rssi) {
        if(rssi > RSSI_MAX) rssi = RSSI_MAX;
        rssi = (rssi - RSSI_MIN) / RSSI_SCALE;
        uint8_t column_number = 0;
        for(size_t i = 0; i <= (uint8_t)rssi; i++) {
            if((i + 1) % 4) {
                column_number++;
                canvas_draw_box(canvas, x + 2 * i, y - column_number, 2, 4 + column_number);
            }
        }
    }

    // Last RSSI
    if(rssi_last) {
        if(rssi_last > RSSI_MAX) rssi_last = RSSI_MAX;
        int max_x = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) * 2;
        //if(!(max_x % 8)) max_x -= 2;
        int max_h = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) + 4;
        max_h -= (max_h / 4) + 3;
        canvas_draw_line(canvas, x + max_x + 1, y - max_h, x + max_x + 1, y + 3);
    }

    // Trigger cursor
    trigger = (trigger - RSSI_MIN) / RSSI_SCALE;
    uint8_t tr_x = x + 2 * trigger;
    canvas_draw_dot(canvas, tr_x, y + 4);
    canvas_draw_line(canvas, tr_x - 1, y + 5, tr_x + 1, y + 5);

    canvas_draw_line(canvas, x, y + 3, x + (RSSI_MAX - RSSI_MIN) * 2 / RSSI_SCALE, y + 3);
}

void subghz_frequency_analyzer_draw(Canvas* canvas, SubGhzFrequencyAnalyzerModel* model) {
    char buffer[64];

    // Title
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 20, 8, "Frequency Analyzer");

    // RSSI
    canvas_draw_str(canvas, 33, 62, "RSSI");
    subghz_frequency_analyzer_draw_rssi(
        canvas, model->rssi, model->rssi_last, model->trigger, 57, 58);

    // Frequency
    canvas_set_font(canvas, FontBigNumbers);
    snprintf(
        buffer,
        sizeof(buffer),
        "%03ld.%03ld",
        model->frequency / 1000000 % 1000,
        model->frequency / 1000 % 1000);
    canvas_draw_str(canvas, 8, 30, buffer);
    canvas_draw_icon(canvas, 96, 19, &I_MHz_25x11);

    // Last detected frequency
    canvas_set_font(canvas, FontSecondary);
    if(model->frequency_last) {
        snprintf(
            buffer,
            sizeof(buffer),
            "Last: %03ld.%03ld MHz",
            model->frequency_last / 1000000 % 1000,
            model->frequency_last / 1000 % 1000);
    } else {
        strcpy(buffer, "Last: ---.--- MHz");
    }
    canvas_draw_str(canvas, 9, 42, buffer);

    switch(model->feedback_level) {
    case 2:
        canvas_draw_icon(canvas, 128 - 8 - 1, 1, &I_Volup_8x6);
        break;
    case 1:
        canvas_draw_icon(canvas, 128 - 8 - 1, 1, &I_Voldwn_6x6);
        break;
    case 0:
        canvas_draw_icon(canvas, 128 - 8 - 1, 1, &I_Voldwn_6x6);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 128 - 2 - 1 - 2, 1, 2, 6);
        canvas_set_color(canvas, ColorBlack);
        break;
    }

    // Buttons hint
    elements_button_left(canvas, "T-");
    elements_button_right(canvas, "T+");
}

uint32_t subghz_frequency_find_correct(uint32_t input) {
    uint32_t prev_freq = 0;
    uint32_t current = 0;
    uint32_t result = 0;
#if FURI_DEBUG
    FURI_LOG_D(TAG, "input: %d", input);
#endif
    for(size_t i = 0; i < sizeof(subghz_frequency_list); i++) {
        current = subghz_frequency_list[i];
        if(current == input) {
            result = current;
            break;
        }
        if(current > input && prev_freq < input) {
            if(current - input < input - prev_freq) {
                result = current;
            } else {
                result = prev_freq;
            }
            break;
        }
        prev_freq = current;
    }

    return result;
}

bool subghz_frequency_analyzer_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    bool need_redraw = false;

    if(event->key == InputKeyBack) return false;

    if(((event->type == InputTypePress) || (event->type == InputTypeRepeat)) &&
       ((event->key == InputKeyLeft) || (event->key == InputKeyRight))) {
        // Trigger setup
        float trigger_level = subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
        switch(event->key) {
        case InputKeyLeft:
            trigger_level -= TRIGGER_STEP;
            if(trigger_level < RSSI_MIN) trigger_level = RSSI_MIN;
            break;
        default:
        case InputKeyRight:
            trigger_level += TRIGGER_STEP;
            if(trigger_level > RSSI_MAX) trigger_level = RSSI_MAX;
            break;
        }
        subghz_frequency_analyzer_worker_set_trigger_level(instance->worker, trigger_level);
        FURI_LOG_I(TAG, "trigger = %.1f", (double)trigger_level);
        need_redraw = true;
    }

    if(event->type == InputTypePress && event->key == InputKeyDown) {
        if(instance->feedback_level == 0) {
            instance->feedback_level = 2;
        } else {
            instance->feedback_level--;
        }
        FURI_LOG_D(TAG, "feedback_level = %d", instance->feedback_level);
        need_redraw = true;
    }

    if(event->key == InputKeyOk) {
        bool updated = false;
        with_view_model(
            instance->view, (SubGhzFrequencyAnalyzerModel * model) {
                uint32_t prev_freq_to_save = model->frequency_to_save;
                uint32_t frequency_candidate = 0;
                if(model->frequency != 0) {
                    frequency_candidate = model->frequency;
                } else if(model->frequency_last != 0) {
                    frequency_candidate = model->frequency_last;
                }
                if(frequency_candidate == 0 ||
                   !furi_hal_subghz_is_frequency_valid(frequency_candidate) ||
                   prev_freq_to_save == frequency_candidate) {
                    frequency_candidate = 0;
                } else {
                    frequency_candidate = subghz_frequency_find_correct(frequency_candidate);
                }
                if(frequency_candidate > 0 && frequency_candidate != model->frequency_to_save) {
#if FURI_DEBUG
                    FURI_LOG_D(
                        TAG,
                        "frequency_to_save: %d, candidate: %d",
                        model->frequency_to_save,
                        frequency_candidate);
#endif
                    model->frequency_to_save = frequency_candidate;
                    notification_message(instance->notifications, &sequence_saved);
                    notification_message(instance->notifications, &sequence_hw_blink);
                    updated = true;
                }
                return true;
            });

#if FURI_DEBUG
        FURI_LOG_I(
            TAG,
            "updated: %d, long: %d, type: %d",
            updated,
            (event->type == InputTypeLong),
            event->type);
#endif

        if(updated) {
            instance->callback(SubGhzCustomEventViewReceiverOK, instance->context);
        }

        // First device receive short, then when user release button we get long
        if(event->type == InputTypeLong) {
#if FURI_DEBUG
            FURI_LOG_I(TAG, "Longpress!");
#endif
            // Stop blinking
            notification_message(instance->notifications, &sequence_hw_blink_stop);

            // Stop worker
            if(subghz_frequency_analyzer_worker_is_running(instance->worker)) {
                subghz_frequency_analyzer_worker_stop(instance->worker);
            }
            instance->callback(SubGhzCustomEventViewReceiverUnlock, instance->context);
        }
    }

    if(need_redraw) {
        SubGhzFrequencyAnalyzer* instance = context;
        with_view_model(
            instance->view, (SubGhzFrequencyAnalyzerModel * model) {
                model->rssi_last = instance->rssi_last;
                model->frequency_last = instance->frequency_last;
                model->trigger =
                    subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
                model->feedback_level = instance->feedback_level;
                return true;
            });
    }

    return true;
}

uint32_t round_int(uint32_t value, uint8_t n) {
    // Round value
    uint8_t on = n;
    while(n--) {
        uint8_t i = value % 10;
        value /= 10;
        if(i >= 5) value++;
    }
    while(on--) value *= 10;
    return value;
}

void subghz_frequency_analyzer_pair_callback(void* context, uint32_t frequency, float rssi) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    if((rssi == 0.f) && (instance->locked)) {
        notification_message(instance->notifications, &sequence_hw_blink);
        instance->frequency_last_vis = instance->frequency_last;
    }

    if((rssi != 0.f) && (frequency != 0)) {
        // Threre is some signal
        FURI_LOG_I(TAG, "rssi = %.2f, frequency = %d Hz", (double)rssi, frequency);
        frequency = round_int(frequency, 3); // Round 299999990Hz to 300000000Hz
        if(!instance->locked) {
            // Triggered!
            instance->rssi_last = rssi;
            notification_message(instance->notifications, &sequence_hw_blink_stop);

            switch(instance->feedback_level) {
            case 1: // 1 - only vibro
                notification_message(instance->notifications, &sequence_single_vibro);
                break;
            case 2: // 2 - vibro and beep
                notification_message(instance->notifications, &sequence_success);
                break;
            default: // 0 - no feedback
                break;
            }

            FURI_LOG_D(TAG, "triggered");
        }
        // Update values
        if(rssi >= instance->rssi_last) {
            instance->rssi_last = rssi;
            instance->frequency_last = frequency;
        }
    }

    instance->locked = (rssi != 0.f);
    with_view_model(
        instance->view, (SubGhzFrequencyAnalyzerModel * model) {
            model->rssi = rssi;
            model->rssi_last = instance->rssi_last;
            model->frequency = frequency;
            model->frequency_last = instance->frequency_last_vis;
            model->trigger = subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
            model->feedback_level = instance->feedback_level;
            return true;
        });
}

void subghz_frequency_analyzer_enter(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    // Notifications
    instance->notifications = furi_record_open(RECORD_NOTIFICATION);
    notification_message(instance->notifications, &sequence_hw_blink);

    //Start worker
    instance->worker = subghz_frequency_analyzer_worker_alloc(instance->context);

    subghz_frequency_analyzer_worker_set_pair_callback(
        instance->worker,
        (SubGhzFrequencyAnalyzerWorkerPairCallback)subghz_frequency_analyzer_pair_callback,
        instance);

    subghz_frequency_analyzer_worker_start(instance->worker);

    instance->rssi_last = 0;
    instance->frequency_last = 0;
    instance->frequency_last_vis = 0;
    subghz_frequency_analyzer_worker_set_trigger_level(instance->worker, RSSI_MIN);

    with_view_model(
        instance->view, (SubGhzFrequencyAnalyzerModel * model) {
            model->rssi = 0;
            model->rssi_last = 0;
            model->frequency = 0;
            model->frequency_last = 0;
            model->frequency_to_save = 0;
            model->trigger = RSSI_MIN;
            return true;
        });
}

void subghz_frequency_analyzer_exit(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    // Stop blinking
    notification_message(instance->notifications, &sequence_hw_blink_stop);

    // Stop worker
    if(subghz_frequency_analyzer_worker_is_running(instance->worker)) {
        subghz_frequency_analyzer_worker_stop(instance->worker);
    }
    subghz_frequency_analyzer_worker_free(instance->worker);

    furi_record_close(RECORD_NOTIFICATION);
}

SubGhzFrequencyAnalyzer* subghz_frequency_analyzer_alloc() {
    SubGhzFrequencyAnalyzer* instance = malloc(sizeof(SubGhzFrequencyAnalyzer));
    furi_assert(instance);

    instance->feedback_level = 2;

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(
        instance->view, ViewModelTypeLocking, sizeof(SubGhzFrequencyAnalyzerModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_frequency_analyzer_draw);
    view_set_input_callback(instance->view, subghz_frequency_analyzer_input);
    view_set_enter_callback(instance->view, subghz_frequency_analyzer_enter);
    view_set_exit_callback(instance->view, subghz_frequency_analyzer_exit);

    return instance;
}

void subghz_frequency_analyzer_free(SubGhzFrequencyAnalyzer* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* subghz_frequency_analyzer_get_view(SubGhzFrequencyAnalyzer* instance) {
    furi_assert(instance);
    return instance->view;
}

uint32_t subghz_frequency_analyzer_get_frequency_to_save(SubGhzFrequencyAnalyzer* instance) {
    furi_assert(instance);
    uint32_t frequency;
    with_view_model(
        instance->view, (SubGhzFrequencyAnalyzerModel * model) {
            frequency = model->frequency_to_save;
            return false;
        });

    return frequency;
}