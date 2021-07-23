#include "subghz_capture.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>

#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/protocols/subghz_protocol.h>

#include <assets_icons.h>

struct SubghzCapture {
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
} SubghzCaptureModel;

static const char subghz_symbols[] = {'-', '\\', '|', '/'};

void subghz_capture_draw(Canvas* canvas, SubghzCaptureModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    snprintf(
        buffer,
        sizeof(buffer),
        "Capture: %03ld.%03ldMHz %c",
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

bool subghz_capture_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzCapture* subghz_capture = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        subghz_capture->view, (SubghzCaptureModel * model) {
            bool reconfigure = false;
            if(event->type == InputTypeShort) {
                if(event->key == InputKeyLeft) {
                    if(model->frequency > 0) model->frequency--;
                    reconfigure = true;
                } else if(event->key == InputKeyRight) {
                    if(model->frequency < subghz_frequencies_count - 1) model->frequency++;
                    reconfigure = true;
                }
            }

            if(reconfigure) {
                api_hal_subghz_idle();
                model->real_frequency =
                    api_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
                api_hal_subghz_rx();
            }

            return reconfigure;
        });

    return true;
}

void subghz_capture_text_callback(string_t text, void* context) {
    furi_assert(context);
    SubghzCapture* subghz_capture = context;

    with_view_model(
        subghz_capture->view, (SubghzCaptureModel * model) {
            model->counter++;
            string_set(model->text, text);
            model->scene = 0;
            return true;
        });
}

void subghz_capture_protocol_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubghzCapture* subghz_capture = context;
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
        subghz_capture->view, (SubghzCaptureModel * model) {
            model->counter++;
            model->parser = *parser;
            string_set(model->text, buffer);
            model->scene = 0;
            return true;
        });
}

void subghz_capture_enter(void* context) {
    furi_assert(context);
    SubghzCapture* subghz_capture = context;

    api_hal_subghz_reset();
    api_hal_subghz_idle();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);

    with_view_model(
        subghz_capture->view, (SubghzCaptureModel * model) {
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                api_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
            model->scene = 1;
            return true;
        });

    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    api_hal_subghz_set_async_rx_callback(subghz_worker_rx_callback, subghz_capture->worker);
    api_hal_subghz_start_async_rx();

    subghz_worker_start(subghz_capture->worker);

    api_hal_subghz_flush_rx();
    api_hal_subghz_rx();
}

void subghz_capture_exit(void* context) {
    furi_assert(context);
    SubghzCapture* subghz_capture = context;

    subghz_worker_stop(subghz_capture->worker);

    api_hal_subghz_stop_async_rx();
    api_hal_subghz_sleep();
}

uint32_t subghz_capture_back(void* context) {
    return SubGhzViewMenu;
}

SubghzCapture* subghz_capture_alloc() {
    SubghzCapture* subghz_capture = furi_alloc(sizeof(SubghzCapture));

    // View allocation and configuration
    subghz_capture->view = view_alloc();
    view_allocate_model(subghz_capture->view, ViewModelTypeLocking, sizeof(SubghzCaptureModel));
    view_set_context(subghz_capture->view, subghz_capture);
    view_set_draw_callback(subghz_capture->view, (ViewDrawCallback)subghz_capture_draw);
    view_set_input_callback(subghz_capture->view, subghz_capture_input);
    view_set_enter_callback(subghz_capture->view, subghz_capture_enter);
    view_set_exit_callback(subghz_capture->view, subghz_capture_exit);
    view_set_previous_callback(subghz_capture->view, subghz_capture_back);

    with_view_model(
        subghz_capture->view, (SubghzCaptureModel * model) {
            string_init(model->text);
            return true;
        });

    subghz_capture->worker = subghz_worker_alloc();
    subghz_capture->protocol = subghz_protocol_alloc();

    subghz_worker_set_overrun_callback(
        subghz_capture->worker, (SubGhzWorkerOverrunCallback)subghz_protocol_reset);
    subghz_worker_set_pair_callback(
        subghz_capture->worker, (SubGhzWorkerPairCallback)subghz_protocol_parse);
    subghz_worker_set_context(subghz_capture->worker, subghz_capture->protocol);

    subghz_protocol_load_keeloq_file(
        subghz_capture->protocol, "/ext/assets/subghz/keeloq_mfcodes");
    subghz_protocol_load_nice_flor_s_file(
        subghz_capture->protocol, "/ext/assets/subghz/nice_floor_s_rx");
    subghz_protocol_enable_dump_text(
        subghz_capture->protocol, subghz_capture_text_callback, subghz_capture);

    return subghz_capture;
}

void subghz_capture_free(SubghzCapture* subghz_capture) {
    furi_assert(subghz_capture);

    subghz_protocol_free(subghz_capture->protocol);
    subghz_worker_free(subghz_capture->worker);

    with_view_model(
        subghz_capture->view, (SubghzCaptureModel * model) {
            string_clear(model->text);
            return true;
        });
    view_free(subghz_capture->view);
    free(subghz_capture);
}

View* subghz_capture_get_view(SubghzCapture* subghz_capture) {
    furi_assert(subghz_capture);
    return subghz_capture->view;
}
