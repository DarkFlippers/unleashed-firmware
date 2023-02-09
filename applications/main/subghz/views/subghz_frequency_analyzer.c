#include "subghz_frequency_analyzer.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>
#include "../helpers/subghz_frequency_analyzer_worker.h"
#include "../helpers/subghz_frequency_analyzer_log_item_array.h"

#include <assets_icons.h>
#include <float_tools.h>

#define LOG_FREQUENCY_MAX_ITEMS 60 // uint8_t (limited by 'seq' of SubGhzFrequencyAnalyzerLogItem)

#define SNPRINTF_FREQUENCY(buff, freq) \
    snprintf(buff, sizeof(buff), "%03ld.%03ld", freq / 1000000 % 1000, freq / 1000 % 1000);

typedef enum {
    SubGhzFrequencyAnalyzerStatusIDLE,
} SubGhzFrequencyAnalyzerStatus;

typedef enum {
    SubGhzFrequencyAnalyzerFragmentBottomTypeMain,
    SubGhzFrequencyAnalyzerFragmentBottomTypeLog,
} SubGhzFrequencyAnalyzerFragmentBottomType;

struct SubGhzFrequencyAnalyzer {
    View* view;
    SubGhzFrequencyAnalyzerWorker* worker;
    SubGhzFrequencyAnalyzerCallback callback;
    void* context;
    bool locked;
    uint32_t last_frequency;
};

typedef struct {
    uint32_t frequency;
    uint8_t rssi;
    uint32_t history_frequency[3];
    bool signal;
    SubGhzFrequencyAnalyzerLogItemArray_t log_frequency;
    SubGhzFrequencyAnalyzerFragmentBottomType fragment_bottom_type;
    SubGhzFrequencyAnalyzerLogOrderBy log_frequency_order_by;
    uint8_t log_frequency_scroll_offset;
} SubGhzFrequencyAnalyzerModel;

static inline uint8_t rssi_sanitize(float rssi) {
    return (
        !float_is_equal(rssi, 0.f) ? (uint8_t)(rssi - SUBGHZ_FREQUENCY_ANALYZER_THRESHOLD) : 0);
}

void subghz_frequency_analyzer_set_callback(
    SubGhzFrequencyAnalyzer* subghz_frequency_analyzer,
    SubGhzFrequencyAnalyzerCallback callback,
    void* context) {
    furi_assert(subghz_frequency_analyzer);
    furi_assert(callback);
    subghz_frequency_analyzer->callback = callback;
    subghz_frequency_analyzer->context = context;
}

void subghz_frequency_analyzer_draw_rssi(Canvas* canvas, uint8_t rssi, uint8_t x, uint8_t y) {
    uint8_t column_number = 0;
    if(rssi) {
        rssi = rssi / 3 + 2;
        if(rssi > 20) rssi = 20;
        for(uint8_t i = 1; i < rssi; i++) {
            if(i % 4) {
                column_number++;
                canvas_draw_box(canvas, x + 2 * i, y - column_number, 2, column_number);
            }
        }
    }
}

void subghz_frequency_analyzer_draw_log_rssi(Canvas* canvas, uint8_t rssi, uint8_t x, uint8_t y) {
    uint8_t column_height = 6;
    if(rssi) {
        if(rssi > 54) rssi = 54;
        for(uint8_t i = 1; i < rssi; i++) {
            if(i % 5) {
                canvas_draw_box(canvas, x + i, y - column_height, 1, column_height);
            }
        }
    }
}

static void subghz_frequency_analyzer_log_frequency_draw(
    Canvas* canvas,
    SubGhzFrequencyAnalyzerModel* model) {
    char buffer[64];
    const uint8_t offset_x = 0;
    const uint8_t offset_y = 43;
    canvas_set_font(canvas, FontKeyboard);

    const size_t items_count = SubGhzFrequencyAnalyzerLogItemArray_size(model->log_frequency);
    if(items_count == 0) {
        canvas_draw_rframe(canvas, offset_x + 27, offset_y - 3, 73, 16, 5);
        canvas_draw_str_aligned(
            canvas, offset_x + 64, offset_y + 8, AlignCenter, AlignBottom, "No records");
        return;
    } else if(items_count > 3) {
        elements_scrollbar_pos(
            canvas,
            offset_x + 127,
            offset_y - 8,
            29,
            model->log_frequency_scroll_offset,
            items_count - 2);
    }

    SubGhzFrequencyAnalyzerLogItem_t* log_frequency_item;
    for(uint8_t i = 0; i < 3; ++i) {
        const uint8_t item_pos = model->log_frequency_scroll_offset + i;
        if(item_pos >= items_count) {
            break;
        }
        log_frequency_item =
            SubGhzFrequencyAnalyzerLogItemArray_get(model->log_frequency, item_pos);
        // Frequency
        SNPRINTF_FREQUENCY(buffer, (*log_frequency_item)->frequency)
        canvas_draw_str(canvas, offset_x, offset_y + i * 10, buffer);

        // Count
        snprintf(buffer, sizeof(buffer), "%3d", (*log_frequency_item)->count);
        canvas_draw_str(canvas, offset_x + 48, offset_y + i * 10, buffer);

        // Max RSSI
        subghz_frequency_analyzer_draw_log_rssi(
            canvas, (*log_frequency_item)->rssi_max, offset_x + 69, (offset_y + i * 10));
    }

    canvas_set_font(canvas, FontSecondary);
}

static void subghz_frequency_analyzer_history_frequency_draw(
    Canvas* canvas,
    SubGhzFrequencyAnalyzerModel* model) {
    char buffer[64];
    uint8_t x = 66;
    uint8_t y = 43;

    canvas_set_font(canvas, FontKeyboard);
    for(uint8_t i = 0; i < 3; i++) {
        if(model->history_frequency[i]) {
            SNPRINTF_FREQUENCY(buffer, model->history_frequency[i])
            canvas_draw_str(canvas, x, y + i * 10, buffer);
        } else {
            canvas_draw_str(canvas, x, y + i * 10, "---.---");
        }
        canvas_draw_str(canvas, x + 44, y + i * 10, "MHz");
    }
    canvas_set_font(canvas, FontSecondary);
}

void subghz_frequency_analyzer_draw(Canvas* canvas, SubGhzFrequencyAnalyzerModel* model) {
    furi_assert(canvas);
    furi_assert(model);
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    if(model->fragment_bottom_type == SubGhzFrequencyAnalyzerFragmentBottomTypeLog) {
        const size_t items_count = SubGhzFrequencyAnalyzerLogItemArray_size(model->log_frequency);
        const char* log_order_by_name =
            subghz_frequency_analyzer_log_get_order_name(model->log_frequency_order_by);
        if(items_count < LOG_FREQUENCY_MAX_ITEMS) {
            snprintf(buffer, sizeof(buffer), "Frequency Analyzer [%s]", log_order_by_name);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignBottom, buffer);
        } else {
            snprintf(buffer, sizeof(buffer), "The log is full! [%s]", log_order_by_name);
            canvas_draw_str(canvas, 2, 8, buffer);
        }
        subghz_frequency_analyzer_log_frequency_draw(canvas, model);
    } else {
        canvas_draw_str(canvas, 20, 8, "Frequency Analyzer");
        canvas_draw_str(canvas, 0, 64, "RSSI");
        subghz_frequency_analyzer_draw_rssi(canvas, model->rssi, 20, 64);

        subghz_frequency_analyzer_history_frequency_draw(canvas, model);
    }

    // Frequency
    canvas_set_font(canvas, FontBigNumbers);
    SNPRINTF_FREQUENCY(buffer, model->frequency);
    if(model->signal) {
        canvas_draw_box(canvas, 4, 11, 121, 22);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 8, 29, buffer);
    canvas_draw_icon(canvas, 96, 18, &I_MHz_25x11);
}

static void subghz_frequency_analyzer_log_frequency_sort(SubGhzFrequencyAnalyzerModel* model) {
    furi_assert(model);
    M_LET((cmp, model->log_frequency_order_by), SubGhzFrequencyAnalyzerLogItemArray_compare_by_t)
    SubGhzFrequencyAnalyzerLogItemArray_sort_fo(
        model->log_frequency, SubGhzFrequencyAnalyzerLogItemArray_compare_by_as_interface(cmp));
}

bool subghz_frequency_analyzer_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    if((event->type == InputTypeShort) &&
       ((event->key == InputKeyLeft) || (event->key == InputKeyRight))) {
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                if(event->key == InputKeyLeft) {
                    if(model->fragment_bottom_type == 0) {
                        model->fragment_bottom_type = SubGhzFrequencyAnalyzerFragmentBottomTypeLog;
                    } else {
                        --model->fragment_bottom_type;
                    }
                } else if(event->key == InputKeyRight) {
                    if(model->fragment_bottom_type ==
                       SubGhzFrequencyAnalyzerFragmentBottomTypeLog) {
                        model->fragment_bottom_type = 0;
                    } else {
                        ++model->fragment_bottom_type;
                    }
                }
            },
            true);
    } else if((event->type == InputTypeShort) && (event->key == InputKeyOk)) {
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                if(model->fragment_bottom_type == SubGhzFrequencyAnalyzerFragmentBottomTypeLog) {
                    ++model->log_frequency_order_by;
                    if(model->log_frequency_order_by >
                       SubGhzFrequencyAnalyzerLogOrderByFrequencyAsc) {
                        model->log_frequency_order_by = 0;
                    }
                    subghz_frequency_analyzer_log_frequency_sort(model);
                }
            },
            true);
    } else if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                if(model->fragment_bottom_type == SubGhzFrequencyAnalyzerFragmentBottomTypeLog) {
                    if(event->key == InputKeyUp) {
                        if(model->log_frequency_scroll_offset > 0) {
                            --model->log_frequency_scroll_offset;
                        }
                    } else if(event->key == InputKeyDown) {
                        const size_t items_count =
                            SubGhzFrequencyAnalyzerLogItemArray_size(model->log_frequency);
                        if((model->log_frequency_scroll_offset + 3u) < items_count) {
                            ++model->log_frequency_scroll_offset;
                        }
                    }
                }
            },
            true);
    }

    return true;
}

static void subghz_frequency_analyzer_log_frequency_search_it(
    SubGhzFrequencyAnalyzerLogItemArray_it_t* itref,
    SubGhzFrequencyAnalyzerLogItemArray_t* log_frequency,
    uint32_t frequency) {
    furi_assert(log_frequency);

    SubGhzFrequencyAnalyzerLogItemArray_it(*itref, *log_frequency);
    SubGhzFrequencyAnalyzerLogItem_t* item;
    while(!SubGhzFrequencyAnalyzerLogItemArray_end_p(*itref)) {
        item = SubGhzFrequencyAnalyzerLogItemArray_ref(*itref);
        if((*item)->frequency == frequency) {
            break;
        }
        SubGhzFrequencyAnalyzerLogItemArray_next(*itref);
    }
}

static bool subghz_frequency_analyzer_log_frequency_insert(SubGhzFrequencyAnalyzerModel* model) {
    furi_assert(model);
    const size_t items_count = SubGhzFrequencyAnalyzerLogItemArray_size(model->log_frequency);
    if(items_count < LOG_FREQUENCY_MAX_ITEMS) {
        SubGhzFrequencyAnalyzerLogItem_t* item =
            SubGhzFrequencyAnalyzerLogItemArray_push_new(model->log_frequency);
        (*item)->frequency = model->frequency;
        (*item)->count = 1;
        (*item)->rssi_max = model->rssi;
        (*item)->seq = items_count;
        return true;
    }
    return false;
}

static void subghz_frequency_analyzer_log_frequency_update(
    SubGhzFrequencyAnalyzerModel* model,
    bool need_insert) {
    furi_assert(model);
    if(!model->frequency) {
        return;
    }

    SubGhzFrequencyAnalyzerLogItemArray_it_t it;
    subghz_frequency_analyzer_log_frequency_search_it(
        &it, &model->log_frequency, model->frequency);
    if(!SubGhzFrequencyAnalyzerLogItemArray_end_p(it)) {
        SubGhzFrequencyAnalyzerLogItem_t* item = SubGhzFrequencyAnalyzerLogItemArray_ref(it);
        if((*item)->rssi_max < model->rssi) {
            (*item)->rssi_max = model->rssi;
        }

        if(need_insert && (*item)->count < UINT8_MAX) {
            ++(*item)->count;
            subghz_frequency_analyzer_log_frequency_sort(model);
        }
    } else if(need_insert) {
        if(subghz_frequency_analyzer_log_frequency_insert(model)) {
            subghz_frequency_analyzer_log_frequency_sort(model);
        }
    }
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
        instance->last_frequency = 0;
        //update history
        with_view_model(
            instance->view,
            SubGhzFrequencyAnalyzerModel * model,
            {
                model->history_frequency[2] = model->history_frequency[1];
                model->history_frequency[1] = model->history_frequency[0];
                model->history_frequency[0] = model->frequency;
            },
            false);
    } else if(!float_is_equal(rssi, 0.f) && !instance->locked) {
        if(instance->callback) {
            instance->callback(SubGhzCustomEventSceneAnalyzerLock, instance->context);
        }
    }

    instance->locked = !float_is_equal(rssi, 0.f);
    with_view_model(
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        {
            model->rssi = rssi_sanitize(rssi);
            model->frequency = frequency;
            model->signal = signal;
            if(frequency) {
                subghz_frequency_analyzer_log_frequency_update(
                    model, frequency != instance->last_frequency);
                instance->last_frequency = frequency;
            }
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

    subghz_frequency_analyzer_worker_start(instance->worker);

    with_view_model(
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        {
            model->rssi = 0u;
            model->frequency = 0;
            model->fragment_bottom_type = SubGhzFrequencyAnalyzerFragmentBottomTypeMain;
            model->log_frequency_order_by = SubGhzFrequencyAnalyzerLogOrderBySeqDesc;
            model->log_frequency_scroll_offset = 0;
            model->history_frequency[0] = model->history_frequency[1] =
                model->history_frequency[2] = 0;
            SubGhzFrequencyAnalyzerLogItemArray_init(model->log_frequency);
        },
        true);
}

void subghz_frequency_analyzer_exit(void* context) {
    furi_assert(context);
    SubGhzFrequencyAnalyzer* instance = context;

    //Stop worker
    if(subghz_frequency_analyzer_worker_is_running(instance->worker)) {
        subghz_frequency_analyzer_worker_stop(instance->worker);
    }
    subghz_frequency_analyzer_worker_free(instance->worker);

    with_view_model(
        instance->view,
        SubGhzFrequencyAnalyzerModel * model,
        {
            model->rssi = 0;
            model->frequency = 0;
            model->fragment_bottom_type = SubGhzFrequencyAnalyzerFragmentBottomTypeMain;
            model->log_frequency_order_by = SubGhzFrequencyAnalyzerLogOrderBySeqDesc;
            model->log_frequency_scroll_offset = 0;
            model->history_frequency[0] = model->history_frequency[1] =
                model->history_frequency[2] = 0;
            SubGhzFrequencyAnalyzerLogItemArray_clear(model->log_frequency);
        },
        true);
}

SubGhzFrequencyAnalyzer* subghz_frequency_analyzer_alloc() {
    SubGhzFrequencyAnalyzer* instance = malloc(sizeof(SubGhzFrequencyAnalyzer));

    // View allocation and configuration
    instance->last_frequency = 0;
    instance->view = view_alloc();
    view_allocate_model(
        instance->view, ViewModelTypeLocking, sizeof(SubGhzFrequencyAnalyzerModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_frequency_analyzer_draw);
    view_set_input_callback(instance->view, subghz_frequency_analyzer_input);
    view_set_enter_callback(instance->view, subghz_frequency_analyzer_enter);
    view_set_exit_callback(instance->view, subghz_frequency_analyzer_exit);

    with_view_model(
        instance->view, SubGhzFrequencyAnalyzerModel * model, { model->rssi = 0; }, true);

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
