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
#include <float_tools.h>

#define TAG "frequency_analyzer"

#define RSSI_MIN -97
#define RSSI_MAX -60
#define RSSI_SCALE 2.3
#define TRIGGER_STEP 1
#define MAX_HISTORY 4

static const uint32_t subghz_frequency_list[] = {
    300000000, 302757000, 303875000, 304250000, 307000000, 307500000, 307800000, 309000000,
    310000000, 312000000, 312100000, 313000000, 313850000, 314000000, 314350000, 314980000,
    315000000, 318000000, 330000000, 345000000, 348000000, 350000000, 387000000, 390000000,
    418000000, 430000000, 431000000, 431500000, 433075000, 433220000, 433420000, 433657070,
    433889000, 433920000, 434075000, 434176948, 434390000, 434420000, 434775000, 438900000,
    440175000, 464000000, 779000000, 868350000, 868400000, 868800000, 868950000, 906400000,
    915000000, 925000000, 928000000};

typedef enum {
    SubGhzFrequencyAnalyzerStatusIDLE,
} SubGhzFrequencyAnalyzerStatus;

struct SubGhzFrequencyAnalyzer {
    View* view;
    SubGhzFrequencyAnalyzerWorker* worker;
    SubGhzFrequencyAnalyzerCallback callback;
    void* context;
    SubGhzTxRx* txrx;
    bool locked;
    SubGHzFrequencyAnalyzerFeedbackLevel
        feedback_level; // 0 - no feedback, 1 - vibro only, 2 - vibro and sound
    float rssi_last;
    uint8_t selected_index;
    uint8_t max_index;
    bool show_frame;
};

typedef struct {
    uint32_t frequency;
    uint32_t frequency_to_save;
    float rssi;
    uint32_t history_frequency[MAX_HISTORY];
    uint8_t history_frequency_rx_count[MAX_HISTORY];
    bool signal;
    float rssi_last;
    float trigger;
    SubGHzFrequencyAnalyzerFeedbackLevel feedback_level;
    uint8_t selected_index;
    uint8_t max_index;
    bool show_frame;
    bool is_ext_radio;
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
        if(rssi > RSSI_MAX) {
            rssi = RSSI_MAX;
        }
        rssi = (rssi - RSSI_MIN) / RSSI_SCALE;
        uint8_t column_number = 0;
        for(size_t i = 0; i <= (uint8_t)rssi; i++) {
            if((i + 1) % 4) {
                column_number++;
                canvas_draw_box(canvas, x + 2 * i, (y + 4) - column_number, 2, column_number);
            }
        }
    }

    // Last RSSI
    if(rssi_last) {
        if(rssi_last > RSSI_MAX) {
            rssi_last = RSSI_MAX;
        }
        int max_x = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) * 2;
        //if(!(max_x % 8)) max_x -= 2;
        int max_h = (int)((rssi_last - RSSI_MIN) / RSSI_SCALE) + 1;
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

static void subghz_frequency_analyzer_history_frequency_draw(
    Canvas* canvas,
    SubGhzFrequencyAnalyzerModel* model) {
    char buffer[64];
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

void subghz_frequency_analyzer_draw(Canvas* canvas, SubGhzFrequencyAnalyzerModel* model) {
    char buffer[64];

    // Title
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    canvas_draw_str(canvas, 0, 7, model->is_ext_radio ? "Ext" : "Int");
    canvas_draw_str(canvas, 20, 7, "Frequency Analyzer");

    // RSSI
    canvas_draw_str(canvas, 33, 62, "RSSI");
    subghz_frequency_analyzer_draw_rssi(
        canvas, model->rssi, model->rssi_last, model->trigger, 56, 57);

    // Last detected frequency
    subghz_frequency_analyzer_history_frequency_draw(canvas, model);

    // Frequency
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
        // TODO: Disable this
        //canvas_draw_box(canvas, 4, 11, 121, 19);
        //canvas_set_color(canvas, ColorWhite);
    }

    canvas_draw_str(canvas, 8, 26, buffer);
    canvas_draw_icon(canvas, 96, 15, &I_MHz_25x11);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    const uint8_t icon_x = 119;
    switch(model->feedback_level) {
    case SubGHzFrequencyAnalyzerFeedbackLevelAll:
        canvas_draw_icon(canvas, icon_x, 1, &I_Volup_8x6);
        break;
    case SubGHzFrequencyAnalyzerFeedbackLevelVibro:
        canvas_draw_icon(canvas, icon_x, 1, &I_Voldwn_6x6);
        break;
    case SubGHzFrequencyAnalyzerFeedbackLevelMute:
        canvas_draw_icon(canvas, icon_x, 1, &I_Voldwn_6x6);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 123, 1, 2, 6);
        canvas_set_color(canvas, ColorBlack);
        break;
    }

    // Buttons hint
    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, "T-");
    elements_button_right(canvas, "+T");
}

uint32_t subghz_frequency_find_correct(uint32_t input) {
    uint32_t prev_freq = 0;
    uint32_t current = 0;
    uint32_t result = 0;
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "input: %ld", input);
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

    if(event->key == InputKeyBack) {
        return false;
    }

    if(((event->type == InputTypePress) || (event->type == InputTypeRepeat)) &&
       ((event->key == InputKeyLeft) || (event->key == InputKeyRight))) {
        // Trigger setup
        float trigger_level = subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
        switch(event->key) {
        case InputKeyLeft:
            trigger_level -= TRIGGER_STEP;
            if(trigger_level < RSSI_MIN) {
                trigger_level = RSSI_MIN;
            }
            break;
        default:
        case InputKeyRight:
            trigger_level += TRIGGER_STEP;
            if(trigger_level > RSSI_MAX) {
                trigger_level = RSSI_MAX;
            }
            break;
        }
        subghz_frequency_analyzer_worker_set_trigger_level(instance->worker, trigger_level);
        FURI_LOG_I(TAG, "trigger = %.1f", (double)trigger_level);
        need_redraw = true;
    } else if(event->type == InputTypePress && event->key == InputKeyUp) {
        if(instance->feedback_level == 0) {
            instance->feedback_level = 2;
        } else {
            instance->feedback_level--;
        }
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "feedback_level = %d", instance->feedback_level);
#endif
        need_redraw = true;
    } else if(
        ((event->type == InputTypePress) || (event->type == InputTypeRepeat)) &&
        event->key == InputKeyDown) {
        instance->show_frame = instance->max_index > 0;
        if(instance->show_frame) {
            instance->selected_index = (instance->selected_index + 1) % instance->max_index;
            need_redraw = true;
        }
    } else if(event->key == InputKeyOk) {
        need_redraw = true;
        bool updated = false;
        uint32_t frequency_to_save = 0;
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                frequency_to_save = model->frequency_to_save;
                if(model->show_frame && !model->signal) {
                    uint32_t prev_freq_to_save = model->frequency_to_save;
                    uint32_t frequency_candidate = model->history_frequency[model->selected_index];
                    if(frequency_candidate == 0 ||
                       // !furi_hal_subghz_is_frequency_valid(frequency_candidate) ||
                       !subghz_txrx_radio_device_is_frequecy_valid(
                           instance->txrx, frequency_candidate) ||
                       prev_freq_to_save == frequency_candidate) {
                        frequency_candidate = 0;
                    } else {
                        frequency_candidate = subghz_frequency_find_correct(frequency_candidate);
                    }
                    if(frequency_candidate > 0 &&
                       frequency_candidate != model->frequency_to_save) {
#ifdef FURI_DEBUG
                        FURI_LOG_D(
                            TAG,
                            "frequency_to_save: %ld, candidate: %ld",
                            model->frequency_to_save,
                            frequency_candidate);
#endif
                        model->frequency_to_save = frequency_candidate;
                        updated = true;
                    }
                } else if(model->show_frame && model->signal) {
                    uint32_t prev_freq_to_save = model->frequency_to_save;
                    uint32_t frequency_candidate = subghz_frequency_find_correct(model->frequency);
                    if(frequency_candidate == 0 ||
                       // !furi_hal_subghz_is_frequency_valid(frequency_candidate) ||
                       !subghz_txrx_radio_device_is_frequecy_valid(
                           instance->txrx, frequency_candidate) ||
                       prev_freq_to_save == frequency_candidate) {
                        frequency_candidate = 0;
                    } else {
                        frequency_candidate = subghz_frequency_find_correct(frequency_candidate);
                    }
                    if(frequency_candidate > 0 &&
                       frequency_candidate != model->frequency_to_save) {
                        model->frequency_to_save = frequency_candidate;
                        updated = true;
                    }
                } else if(!model->show_frame && model->signal) {
                    uint32_t prev_freq_to_save = model->frequency_to_save;
                    uint32_t frequency_candidate = subghz_frequency_find_correct(model->frequency);
                    if(frequency_candidate == 0 ||
                       // !furi_hal_subghz_is_frequency_valid(frequency_candidate) ||
                       !subghz_txrx_radio_device_is_frequecy_valid(
                           instance->txrx, frequency_candidate) ||
                       prev_freq_to_save == frequency_candidate) {
                        frequency_candidate = 0;
                    } else {
                        frequency_candidate = subghz_frequency_find_correct(frequency_candidate);
                    }
                    if(frequency_candidate > 0 &&
                       frequency_candidate != model->frequency_to_save) {
                        model->frequency_to_save = frequency_candidate;
                        updated = true;
                    }
                }
            },
            true);

#ifdef FURI_DEBUG
        FURI_LOG_I(
            TAG,
            "updated: %d, long: %d, type: %d",
            updated,
            (event->type == InputTypeLong),
            event->type);
#endif

        if(updated) {
            instance->callback(SubGhzCustomEventViewFreqAnalOkShort, instance->context);
        }

        // First device receive short, then when user release button we get long
        if(event->type == InputTypeLong && frequency_to_save > 0) {
#ifdef FURI_DEBUG
            FURI_LOG_I(TAG, "Long press!");
#endif
            // Stop worker
            if(subghz_frequency_analyzer_worker_is_running(instance->worker)) {
                subghz_frequency_analyzer_worker_stop(instance->worker);
            }

            instance->callback(SubGhzCustomEventViewFreqAnalOkLong, instance->context);
        }
    }

    if(need_redraw) {
        SubGhzFrequencyAnalyzer* instance = context;
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                model->rssi_last = instance->rssi_last;
                model->trigger =
                    subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
                model->feedback_level = instance->feedback_level;
                model->max_index = instance->max_index;
                model->show_frame = instance->show_frame;
                model->selected_index = instance->selected_index;
            },
            true);
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

void subghz_frequency_analyzer_pair_callback(
    void* context,
    uint32_t frequency,
    float rssi,
    bool signal) {
    SubGhzFrequencyAnalyzer* instance = context;
    if(float_is_equal(rssi, 0.f) && instance->locked) {
        if(instance->callback) {
            instance->callback(SubGhzCustomEventSceneAnalyzerUnlock, instance->context);
        }
        //update history
        instance->show_frame = true;
        uint8_t max_index = instance->max_index;
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                bool in_array = false;
                uint32_t normal_frequency = subghz_frequency_find_correct(model->frequency);
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
                                if(j == i) {
                                    offset++;
                                }
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
                    model->history_frequency[3] = model->history_frequency[2];
                    model->history_frequency[2] = model->history_frequency[1];
                    model->history_frequency[1] = model->history_frequency[0];
                    model->history_frequency[0] = normal_frequency;

                    model->history_frequency_rx_count[3] = model->history_frequency_rx_count[2];
                    model->history_frequency_rx_count[2] = model->history_frequency_rx_count[1];
                    model->history_frequency_rx_count[1] = model->history_frequency_rx_count[0];
                    model->history_frequency_rx_count[0] = 0;
                }

                if(max_index < MAX_HISTORY) {
                    for(size_t i = 0; i < MAX_HISTORY; i++) {
                        if(model->history_frequency[i] > 0) {
                            max_index = i + 1;
                        }
                    }
                }
            },
            false);
        instance->max_index = max_index;
    } else if((rssi != 0.f) && (!instance->locked)) {
        // There is some signal
        FURI_LOG_I(TAG, "rssi = %.2f, frequency = %ld Hz", (double)rssi, frequency);
        frequency = round_int(frequency, 3); // Round 299999990Hz to 300000000Hz

        // Triggered!
        instance->rssi_last = rssi;
        if(instance->callback) {
            instance->callback(SubGhzCustomEventSceneAnalyzerLock, instance->context);
        }
    }

    // Update values
    if(rssi >= instance->rssi_last && (frequency != 0)) {
        instance->rssi_last = rssi;
    }

    instance->locked = (rssi != 0.f);
    with_view_model(
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        {
            model->rssi = rssi;
            model->rssi_last = instance->rssi_last;
            model->frequency = frequency;
            model->signal = signal;
            model->trigger = subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
            model->feedback_level = instance->feedback_level;
            model->max_index = instance->max_index;
            model->show_frame = instance->show_frame;
            model->selected_index = instance->selected_index;
        },
        true);
}

void subghz_frequency_analyzer_enter(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    //Start worker
    instance->worker = subghz_frequency_analyzer_worker_alloc(instance->context);

    subghz_frequency_analyzer_worker_set_pair_callback(
        instance->worker,
        (SubGhzFrequencyAnalyzerWorkerPairCallback)subghz_frequency_analyzer_pair_callback,
        instance);

    subghz_frequency_analyzer_worker_start(instance->worker, instance->txrx);

    instance->rssi_last = 0;
    instance->selected_index = 0;
    instance->max_index = 0;
    instance->show_frame = false;
    //subghz_frequency_analyzer_worker_set_trigger_level(instance->worker, RSSI_MIN);

    with_view_model(
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        {
            model->selected_index = 0;
            model->max_index = 0;
            model->show_frame = false;
            model->rssi = 0;
            model->rssi_last = 0;
            model->frequency = 0;
            model->history_frequency[3] = 0;
            model->history_frequency[2] = 0;
            model->history_frequency[1] = 0;
            model->history_frequency[0] = 0;
            model->history_frequency_rx_count[3] = 0;
            model->history_frequency_rx_count[2] = 0;
            model->history_frequency_rx_count[1] = 0;
            model->history_frequency_rx_count[0] = 0;
            model->frequency_to_save = 0;
            model->trigger = RSSI_MIN;
            model->is_ext_radio =
                (subghz_txrx_radio_device_get(instance->txrx) != SubGhzRadioDeviceTypeInternal);
        },
        true);
}

void subghz_frequency_analyzer_exit(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    // Stop worker
    if(subghz_frequency_analyzer_worker_is_running(instance->worker)) {
        subghz_frequency_analyzer_worker_stop(instance->worker);
    }
    subghz_frequency_analyzer_worker_free(instance->worker);

    furi_record_close(RECORD_NOTIFICATION);
}

SubGhzFrequencyAnalyzer* subghz_frequency_analyzer_alloc(SubGhzTxRx* txrx) {
    SubGhzFrequencyAnalyzer* instance = malloc(sizeof(SubGhzFrequencyAnalyzer));

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

    instance->txrx = txrx;

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
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        { frequency = model->frequency_to_save; },
        false);

    return frequency;
}

SubGHzFrequencyAnalyzerFeedbackLevel subghz_frequency_analyzer_feedback_level(
    SubGhzFrequencyAnalyzer* instance,
    SubGHzFrequencyAnalyzerFeedbackLevel level,
    bool update) {
    furi_assert(instance);
    if(update) {
        instance->feedback_level = level;
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            { model->feedback_level = instance->feedback_level; },
            true);
    }

    return instance->feedback_level;
}

float subghz_frequency_analyzer_get_trigger_level(SubGhzFrequencyAnalyzer* instance) {
    furi_assert(instance);
    return subghz_frequency_analyzer_worker_get_trigger_level(instance->worker);
}
