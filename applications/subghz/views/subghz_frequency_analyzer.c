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

#define RSSI_MIN -101
#define RSSI_MAX -60
#define RSSI_SCALE 2
#define TRIGGER_STEP 1
#define TRIGGER_MIN RSSI_MIN + RSSI_SCALE * 2

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

typedef enum {
    SubGhzFrequencyAnalyzerStatusIDLE,
} SubGhzFrequencyAnalyzerStatus;

struct SubGhzFrequencyAnalyzer {
    View* view;
    SubGhzFrequencyAnalyzerWorker* worker;
    SubGhzFrequencyAnalyzerCallback callback;
    void* context;
    bool locked;
    float rssi_last;
    uint32_t frequency_last;
    float trigger;
    bool triggered;
    NotificationApp* notifications;
};

typedef struct {
    uint32_t frequency;
    uint32_t frequency_last;
    float rssi;
    float rssi_last;
    float trigger;
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
        rssi = (rssi - RSSI_MIN) / RSSI_SCALE;
        if(rssi > 20) rssi = 20;
        uint8_t column_number = 0;
        for(size_t i = 1; i < (uint8_t)rssi; i++) {
            if(i % 4) {
                column_number++;
                canvas_draw_box(canvas, x + 2 * i, y - column_number, 2, 4 + column_number);
            }
        }
    }

    // Last RSSI
    if(rssi_last) {
        int max_x = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE - 1) * 2;
        //if(!(max_x % 8)) max_x -= 2;
        int max_h = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE - 1) + 4;
        max_h -= (max_h / 4) + 3;
        if(max_x > 38) max_h = 38;
        if(max_h > 19) max_h = 19;
        if(max_x >= 0 && max_h > 0) {
            canvas_draw_line(canvas, x + max_x + 1, y - max_h, x + max_x + 1, y + 3);
        }
    }

    // Trigger cursor
    if(trigger >= RSSI_MIN + RSSI_SCALE * 2) {
        trigger = (trigger - RSSI_MIN) / RSSI_SCALE;
        uint8_t tr_x = x + 2 * trigger - 2;
        canvas_draw_dot(canvas, tr_x, y + 4);
        canvas_draw_line(canvas, tr_x - 1, y + 5, tr_x + 1, y + 5);
    }

    canvas_draw_line(canvas, x + 2, y + 3, x + 39, y + 3);
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
        canvas, model->rssi, model->rssi_last, model->trigger, 55, 58);

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

    // Buttons hint
    elements_button_left(canvas, "T-");
    elements_button_right(canvas, "T+");
}

bool subghz_frequency_analyzer_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    bool need_redraw = false;

    if(event->key == InputKeyBack) return false;

    if(((event->type == InputTypePress) || (event->type == InputTypeRepeat)) &&
       ((event->key == InputKeyLeft) || (event->key == InputKeyRight))) {
        // Trigger setup
        switch(event->key) {
        case InputKeyLeft:
            instance->trigger -= TRIGGER_STEP;
            if(instance->trigger < RSSI_MIN + RSSI_SCALE * 2) instance->trigger = TRIGGER_MIN;
            break;
        default:
        case InputKeyRight:
            if(instance->trigger < RSSI_MIN + RSSI_SCALE * 2)
                instance->trigger = TRIGGER_MIN;
            else
                instance->trigger += TRIGGER_STEP;
            if(instance->trigger > RSSI_MAX) instance->trigger = RSSI_MAX;
            break;
        }
        if(instance->trigger > RSSI_MIN)
            FURI_LOG_I(TAG, "trigger = %.1f", (double)instance->trigger);
        else
            FURI_LOG_I(TAG, "trigger disabled");
        need_redraw = true;
    }

    if(need_redraw) {
        SubGhzFrequencyAnalyzer* instance = context;
        with_view_model(
            instance->view, (SubGhzFrequencyAnalyzerModel * model) {
                model->rssi_last = instance->rssi_last;
                model->frequency_last = instance->frequency_last;
                model->trigger = instance->trigger;
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
        instance->triggered = false;
    }

    if((rssi != 0.f) && (frequency != 0)) {
        // Threre is some signal
        FURI_LOG_I(TAG, "rssi = %.2f, frequency = %d Hz", (double)rssi, frequency);
        frequency = round_int(frequency, 3); // Round 299999990Hz to 300000000Hz
        if((instance->trigger <= RSSI_MIN + RSSI_SCALE * 2) || (rssi >= instance->trigger)) {
            if(!instance->triggered) {
                // Triggered!
                instance->triggered = true;
                instance->rssi_last = rssi;
                notification_message(instance->notifications, &sequence_hw_blink_stop);
                notification_message(instance->notifications, &sequence_success);
                FURI_LOG_D(TAG, "triggered");
            }
            // Update values
            if(rssi > instance->rssi_last) instance->rssi_last = rssi;
            instance->frequency_last = frequency;
        } else {
            instance->triggered = false;
        }
    }

    instance->locked = (rssi != 0.f);
    with_view_model(
        instance->view, (SubGhzFrequencyAnalyzerModel * model) {
            model->rssi = rssi;
            model->rssi_last = instance->rssi_last;
            model->frequency = frequency;
            model->frequency_last = instance->frequency_last;
            model->trigger = instance->trigger;
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
    instance->trigger = TRIGGER_MIN;
    instance->triggered = false;

    with_view_model(
        instance->view, (SubGhzFrequencyAnalyzerModel * model) {
            model->rssi = 0;
            model->rssi_last = 0;
            model->frequency = 0;
            model->frequency_last = 0;
            model->trigger = instance->trigger;
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
