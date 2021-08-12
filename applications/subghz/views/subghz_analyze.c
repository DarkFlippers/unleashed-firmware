#include "subghz_analyze.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>

#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/protocols/subghz_protocol.h>

#include <assets_icons.h>

struct SubghzAnalyze {
    View* view;
    SubGhzWorker* worker;
    SubGhzProtocol* protocol;
};

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    uint32_t counter;
    string_t text;
    uint16_t scene;
    SubGhzProtocolCommon parser;
} SubghzAnalyzeModel;

static const char subghz_symbols[] = {'-', '\\', '|', '/'};

void subghz_analyze_draw(Canvas* canvas, SubghzAnalyzeModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    snprintf(
        buffer,
        sizeof(buffer),
        "Analyze: %03ld.%03ldMHz %c",
        model->real_frequency / 1000000 % 1000,
        model->real_frequency / 1000 % 1000,
        subghz_symbols[model->counter % 4]);
    canvas_draw_str(canvas, 0, 8, buffer);

    switch(model->scene) {
    case 1:
        canvas_draw_icon(canvas, 0, 10, &I_RFIDDolphinReceive_97x61);
        canvas_invert_color(canvas);
        canvas_draw_box(canvas, 80, 12, 20, 20);
        canvas_invert_color(canvas);
        canvas_draw_icon(canvas, 75, 18, &I_sub1_10px);
        elements_multiline_text_aligned(
            canvas, 90, 38, AlignCenter, AlignTop, "Detecting\r\nSubGhz");
        break;

    default:
        canvas_set_font(canvas, FontSecondary);
        elements_multiline_text(canvas, 0, 20, string_get_cstr(model->text));
        break;
    }
}

bool subghz_analyze_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzAnalyze* subghz_analyze = context;

    if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            bool model_updated = false;

            if(event->key == InputKeyLeft) {
                if(model->frequency > 0) model->frequency--;
                model_updated = true;
            } else if(event->key == InputKeyRight) {
                if(model->frequency < subghz_frequencies_count - 1) model->frequency++;
                model_updated = true;
            }

            if(model_updated) {
                furi_hal_subghz_idle();
                model->real_frequency =
                    furi_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
                furi_hal_subghz_rx();
            }

            return model_updated;
        });

    return true;
}

void subghz_analyze_text_callback(string_t text, void* context) {
    furi_assert(context);
    SubghzAnalyze* subghz_analyze = context;

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            model->counter++;
            string_set(model->text, text);
            model->scene = 0;
            return true;
        });
}

void subghz_analyze_protocol_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubghzAnalyze* subghz_analyze = context;
    char buffer[64];
    snprintf(
        buffer,
        sizeof(buffer),
        "%s\r\n"
        "K:%lX%lX\r\n"
        "SN:%lX\r\n"
        "BTN:%X",
        parser->name,
        (uint32_t)(parser->code_found >> 32),
        (uint32_t)(parser->code_found & 0x00000000FFFFFFFF),
        parser->serial,
        parser->btn);

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            model->counter++;
            model->parser = *parser;
            string_set(model->text, buffer);
            model->scene = 0;
            return true;
        });
}

void subghz_analyze_enter(void* context) {
    furi_assert(context);
    SubghzAnalyze* subghz_analyze = context;

    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOokAsync);

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                furi_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
            model->scene = 1;
            return true;
        });

    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, subghz_analyze->worker);

    subghz_worker_start(subghz_analyze->worker);

    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();
}

void subghz_analyze_exit(void* context) {
    furi_assert(context);
    SubghzAnalyze* subghz_analyze = context;

    subghz_worker_stop(subghz_analyze->worker);

    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_sleep();
}

SubghzAnalyze* subghz_analyze_alloc() {
    SubghzAnalyze* subghz_analyze = furi_alloc(sizeof(SubghzAnalyze));

    // View allocation and configuration
    subghz_analyze->view = view_alloc();
    view_allocate_model(subghz_analyze->view, ViewModelTypeLocking, sizeof(SubghzAnalyzeModel));
    view_set_context(subghz_analyze->view, subghz_analyze);
    view_set_draw_callback(subghz_analyze->view, (ViewDrawCallback)subghz_analyze_draw);
    view_set_input_callback(subghz_analyze->view, subghz_analyze_input);
    view_set_enter_callback(subghz_analyze->view, subghz_analyze_enter);
    view_set_exit_callback(subghz_analyze->view, subghz_analyze_exit);

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            string_init(model->text);
            return true;
        });

    subghz_analyze->worker = subghz_worker_alloc();
    subghz_analyze->protocol = subghz_protocol_alloc();

    subghz_worker_set_overrun_callback(
        subghz_analyze->worker, (SubGhzWorkerOverrunCallback)subghz_protocol_reset);
    subghz_worker_set_pair_callback(
        subghz_analyze->worker, (SubGhzWorkerPairCallback)subghz_protocol_parse);
    subghz_worker_set_context(subghz_analyze->worker, subghz_analyze->protocol);

    subghz_protocol_load_keeloq_file(
        subghz_analyze->protocol, "/ext/assets/subghz/keeloq_mfcodes");
    subghz_protocol_load_nice_flor_s_file(
        subghz_analyze->protocol, "/ext/assets/subghz/nice_floor_s_rx");
    subghz_protocol_enable_dump_text(
        subghz_analyze->protocol, subghz_analyze_text_callback, subghz_analyze);

    return subghz_analyze;
}

void subghz_analyze_free(SubghzAnalyze* subghz_analyze) {
    furi_assert(subghz_analyze);

    subghz_protocol_free(subghz_analyze->protocol);
    subghz_worker_free(subghz_analyze->worker);

    with_view_model(
        subghz_analyze->view, (SubghzAnalyzeModel * model) {
            string_clear(model->text);
            return true;
        });
    view_free(subghz_analyze->view);
    free(subghz_analyze);
}

View* subghz_analyze_get_view(SubghzAnalyze* subghz_analyze) {
    furi_assert(subghz_analyze);
    return subghz_analyze->view;
}
