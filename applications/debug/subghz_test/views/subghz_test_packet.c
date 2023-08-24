#include "subghz_test_packet.h"
#include "../subghz_test_app_i.h"
#include "../helpers/subghz_test_frequency.h"
#include <lib/subghz/devices/cc1101_configs.h>

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <toolbox/level_duration.h>
#include "../protocol/princeton_for_testing.h"

#define SUBGHZ_TEST_PACKET_COUNT 500

struct SubGhzTestPacket {
    View* view;
    FuriTimer* timer;

    SubGhzDecoderPrinceton* decoder;
    SubGhzEncoderPrinceton* encoder;
    volatile size_t packet_rx;
    SubGhzTestPacketCallback callback;
    void* context;
};

typedef enum {
    SubGhzTestPacketModelStatusRx,
    SubGhzTestPacketModelStatusOnlyRx,
    SubGhzTestPacketModelStatusTx,
} SubGhzTestPacketModelStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    FuriHalSubGhzPath path;
    float rssi;
    size_t packets;
    SubGhzTestPacketModelStatus status;
} SubGhzTestPacketModel;

volatile bool subghz_test_packet_overrun = false;

void subghz_test_packet_set_callback(
    SubGhzTestPacket* subghz_test_packet,
    SubGhzTestPacketCallback callback,
    void* context) {
    furi_assert(subghz_test_packet);
    furi_assert(callback);
    subghz_test_packet->callback = callback;
    subghz_test_packet->context = context;
}

static void subghz_test_packet_rx_callback(bool level, uint32_t duration, void* context) {
    furi_assert(context);
    SubGhzTestPacket* instance = context;
    subghz_decoder_princeton_for_testing_parse(instance->decoder, level, duration);
}

static void subghz_test_packet_rx_pt_callback(SubGhzDecoderPrinceton* parser, void* context) {
    UNUSED(parser);
    furi_assert(context);
    SubGhzTestPacket* instance = context;
    instance->packet_rx++;
}

static void subghz_test_packet_rssi_timer_callback(void* context) {
    furi_assert(context);
    SubGhzTestPacket* instance = context;

    with_view_model(
        instance->view,
        SubGhzTestPacketModel * model,
        {
            if(model->status == SubGhzTestPacketModelStatusRx) {
                model->rssi = furi_hal_subghz_get_rssi();
                model->packets = instance->packet_rx;
            } else if(model->status == SubGhzTestPacketModelStatusTx) {
                model->packets =
                    SUBGHZ_TEST_PACKET_COUNT -
                    subghz_encoder_princeton_for_testing_get_repeat_left(instance->encoder);
            }
        },
        true);
}

static void subghz_test_packet_draw(Canvas* canvas, SubGhzTestPacketModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "CC1101 Packet Test");

    canvas_set_font(canvas, FontSecondary);
    // Frequency
    snprintf(
        buffer,
        sizeof(buffer),
        "Freq: %03ld.%03ld.%03ld Hz",
        model->real_frequency / 1000000 % 1000,
        model->real_frequency / 1000 % 1000,
        model->real_frequency % 1000);
    canvas_draw_str(canvas, 0, 20, buffer);
    // Path
    char* path_name = "Unknown";
    if(model->path == FuriHalSubGhzPathIsolate) {
        path_name = "isolate";
    } else if(model->path == FuriHalSubGhzPath433) {
        path_name = "433MHz";
    } else if(model->path == FuriHalSubGhzPath315) {
        path_name = "315MHz";
    } else if(model->path == FuriHalSubGhzPath868) {
        path_name = "868MHz";
    }
    snprintf(buffer, sizeof(buffer), "Path: %d - %s", model->path, path_name);
    canvas_draw_str(canvas, 0, 31, buffer);

    snprintf(buffer, sizeof(buffer), "Packets: %zu", model->packets);
    canvas_draw_str(canvas, 0, 42, buffer);

    if(model->status == SubGhzTestPacketModelStatusRx) {
        snprintf(
            buffer,
            sizeof(buffer),
            "RSSI: %ld.%ld dBm",
            (int32_t)(model->rssi),
            (int32_t)fabs(model->rssi * 10) % 10);
        canvas_draw_str(canvas, 0, 53, buffer);
    } else {
        canvas_draw_str(canvas, 0, 53, "TX");
    }
}

static bool subghz_test_packet_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzTestPacket* instance = context;

    if(event->key == InputKeyBack || event->type != InputTypeShort) {
        return false;
    }

    with_view_model(
        instance->view,
        SubGhzTestPacketModel * model,
        {
            if(model->status == SubGhzTestPacketModelStatusRx) {
                furi_hal_subghz_stop_async_rx();
            } else if(model->status == SubGhzTestPacketModelStatusTx) {
                subghz_encoder_princeton_for_testing_stop(instance->encoder, furi_get_tick());
                furi_hal_subghz_stop_async_tx();
            }

            if(event->key == InputKeyLeft) {
                if(model->frequency > 0) model->frequency--;
            } else if(event->key == InputKeyRight) {
                if(model->frequency < subghz_frequencies_count_testing - 1) model->frequency++;
            } else if(event->key == InputKeyDown) {
                if(model->path > 0) model->path--;
            } else if(event->key == InputKeyUp) {
                if(model->path < FuriHalSubGhzPath868) model->path++;
            } else if(event->key == InputKeyOk) {
                if(model->status == SubGhzTestPacketModelStatusRx) {
                    model->status = SubGhzTestPacketModelStatusTx;
                } else {
                    model->status = SubGhzTestPacketModelStatusRx;
                }
            }

            model->real_frequency =
                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
            furi_hal_subghz_set_path(model->path);

            if(model->status == SubGhzTestPacketModelStatusRx) {
                furi_hal_subghz_start_async_rx(subghz_test_packet_rx_callback, instance);
            } else {
                subghz_encoder_princeton_for_testing_set(
                    instance->encoder,
                    0x00AABBCC,
                    SUBGHZ_TEST_PACKET_COUNT,
                    subghz_frequencies_testing[model->frequency]);
                if(!furi_hal_subghz_start_async_tx(
                       subghz_encoder_princeton_for_testing_yield, instance->encoder)) {
                    model->status = SubGhzTestPacketModelStatusOnlyRx;
                    instance->callback(SubGhzTestPacketEventOnlyRx, instance->context);
                }
            }
        },
        true);

    return true;
}

void subghz_test_packet_enter(void* context) {
    furi_assert(context);
    SubGhzTestPacket* instance = context;

    furi_hal_subghz_reset();
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);

    with_view_model(
        instance->view,
        SubGhzTestPacketModel * model,
        {
            model->frequency = subghz_frequencies_433_92_testing;
            model->real_frequency =
                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
            model->path = FuriHalSubGhzPathIsolate; // isolate
            model->rssi = 0.0f;
            model->status = SubGhzTestPacketModelStatusRx;
        },
        true);

    furi_hal_subghz_start_async_rx(subghz_test_packet_rx_callback, instance);

    furi_timer_start(instance->timer, furi_kernel_get_tick_frequency() / 4);
}

void subghz_test_packet_exit(void* context) {
    furi_assert(context);
    SubGhzTestPacket* instance = context;

    furi_timer_stop(instance->timer);

    // Reinitialize IC to default state
    with_view_model(
        instance->view,
        SubGhzTestPacketModel * model,
        {
            if(model->status == SubGhzTestPacketModelStatusRx) {
                furi_hal_subghz_stop_async_rx();
            } else if(model->status == SubGhzTestPacketModelStatusTx) {
                subghz_encoder_princeton_for_testing_stop(instance->encoder, furi_get_tick());
                furi_hal_subghz_stop_async_tx();
            }
        },
        true);
    furi_hal_subghz_sleep();
}

SubGhzTestPacket* subghz_test_packet_alloc() {
    SubGhzTestPacket* instance = malloc(sizeof(SubGhzTestPacket));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubGhzTestPacketModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_test_packet_draw);
    view_set_input_callback(instance->view, subghz_test_packet_input);
    view_set_enter_callback(instance->view, subghz_test_packet_enter);
    view_set_exit_callback(instance->view, subghz_test_packet_exit);

    instance->timer =
        furi_timer_alloc(subghz_test_packet_rssi_timer_callback, FuriTimerTypePeriodic, instance);

    instance->decoder = subghz_decoder_princeton_for_testing_alloc();
    subghz_decoder_princeton_for_testing_set_callback(
        instance->decoder, subghz_test_packet_rx_pt_callback, instance);
    instance->encoder = subghz_encoder_princeton_for_testing_alloc();

    return instance;
}

void subghz_test_packet_free(SubGhzTestPacket* instance) {
    furi_assert(instance);

    subghz_decoder_princeton_for_testing_free(instance->decoder);
    subghz_encoder_princeton_for_testing_free(instance->encoder);

    furi_timer_free(instance->timer);
    view_free(instance->view);
    free(instance);
}

View* subghz_test_packet_get_view(SubGhzTestPacket* instance) {
    furi_assert(instance);
    return instance->view;
}
