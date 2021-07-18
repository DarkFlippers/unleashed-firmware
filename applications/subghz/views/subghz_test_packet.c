#include "subghz_test_packet.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>
#include <toolbox/level_duration.h>
#include <lib/subghz/protocols/subghz_protocol_princeton.h>

#define SUBGHZ_TEST_PACKET_COUNT 1000

#define SUBGHZ_PT_SHORT 376
#define SUBGHZ_PT_LONG (SUBGHZ_PT_SHORT * 3)
#define SUBGHZ_PT_GUARD 10600

struct SubghzTestPacket {
    View* view;
    osTimerId timer;
    size_t tx_buffer_size;
    uint32_t* tx_buffer;
    SubGhzProtocolPrinceton* princeton;

    volatile size_t packet_rx;
};

typedef enum {
    SubghzTestPacketModelStatusRx,
    SubghzTestPacketModelStatusTx,
} SubghzTestPacketModelStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    ApiHalSubGhzPath path;
    float rssi;
    size_t packets;
    SubghzTestPacketModelStatus status;
} SubghzTestPacketModel;

volatile bool subghz_test_packet_overrun = false;

static void subghz_test_packet_rx_callback(bool level, uint32_t duration, void* context) {
    furi_assert(context);
    SubghzTestPacket* instance = context;
    subghz_protocol_princeton_parse(instance->princeton, level, duration);
}

static void subghz_test_packet_rx_pt_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubghzTestPacket* instance = context;
    instance->packet_rx++;
}

static void subghz_test_packet_rssi_timer_callback(void* context) {
    furi_assert(context);
    SubghzTestPacket* instance = context;

    with_view_model(
        instance->view, (SubghzTestPacketModel * model) {
            if(model->status == SubghzTestPacketModelStatusRx) {
                model->rssi = api_hal_subghz_get_rssi();
                model->packets = instance->packet_rx;
            } else {
                model->packets =
                    SUBGHZ_TEST_PACKET_COUNT - api_hal_subghz_get_async_tx_repeat_left();
            }
            return true;
        });
}

static void subghz_test_packet_draw(Canvas* canvas, SubghzTestPacketModel* model) {
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
    if(model->path == ApiHalSubGhzPathIsolate) {
        path_name = "isolate";
    } else if(model->path == ApiHalSubGhzPath433) {
        path_name = "433MHz";
    } else if(model->path == ApiHalSubGhzPath315) {
        path_name = "315MHz";
    } else if(model->path == ApiHalSubGhzPath868) {
        path_name = "868MHz";
    }
    snprintf(buffer, sizeof(buffer), "Path: %d - %s", model->path, path_name);
    canvas_draw_str(canvas, 0, 31, buffer);

    snprintf(buffer, sizeof(buffer), "Packets: %d", model->packets);
    canvas_draw_str(canvas, 0, 42, buffer);

    if(model->status == SubghzTestPacketModelStatusRx) {
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
    SubghzTestPacket* instance = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        instance->view, (SubghzTestPacketModel * model) {
            if(model->status == SubghzTestPacketModelStatusRx) {
                api_hal_subghz_stop_async_rx();
            } else {
                api_hal_subghz_stop_async_tx();
            }

            if(event->type == InputTypeShort) {
                if(event->key == InputKeyLeft) {
                    if(model->frequency > 0) model->frequency--;
                } else if(event->key == InputKeyRight) {
                    if(model->frequency < subghz_frequencies_count - 1) model->frequency++;
                } else if(event->key == InputKeyDown) {
                    if(model->path > 0) model->path--;
                } else if(event->key == InputKeyUp) {
                    if(model->path < ApiHalSubGhzPath868) model->path++;
                } else if(event->key == InputKeyOk) {
                    if(model->status == SubghzTestPacketModelStatusTx) {
                        model->status = SubghzTestPacketModelStatusRx;
                    } else {
                        model->status = SubghzTestPacketModelStatusTx;
                    }
                }

                model->real_frequency =
                    api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
                api_hal_subghz_set_path(model->path);
            }

            if(model->status == SubghzTestPacketModelStatusRx) {
                api_hal_subghz_start_async_rx();
            } else {
                api_hal_subghz_start_async_tx(
                    instance->tx_buffer, instance->tx_buffer_size, SUBGHZ_TEST_PACKET_COUNT);
            }

            return true;
        });

    return true;
}

void subghz_test_packet_enter(void* context) {
    furi_assert(context);
    SubghzTestPacket* instance = context;

    instance->tx_buffer_size = 25 * 2 * sizeof(uint32_t);
    instance->tx_buffer = furi_alloc(instance->tx_buffer_size);

    const uint32_t key = 0x00ABCDEF;

    size_t pos = 0;
    for(uint8_t i = 0; i < 24; i++) {
        uint8_t byte = i / 8;
        uint8_t bit = i % 8;
        bool value = (((uint8_t*)&key)[2 - byte] >> (7 - bit)) & 1;
        if(value) {
            instance->tx_buffer[pos++] = SUBGHZ_PT_SHORT;
            instance->tx_buffer[pos++] = SUBGHZ_PT_LONG;
        } else {
            instance->tx_buffer[pos++] = SUBGHZ_PT_LONG;
            instance->tx_buffer[pos++] = SUBGHZ_PT_SHORT;
        }
    }
    instance->tx_buffer[pos++] = SUBGHZ_PT_SHORT;
    instance->tx_buffer[pos++] = SUBGHZ_PT_SHORT + SUBGHZ_PT_GUARD;

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    api_hal_subghz_set_async_rx_callback(subghz_test_packet_rx_callback, instance);

    with_view_model(
        instance->view, (SubghzTestPacketModel * model) {
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
            model->path = ApiHalSubGhzPathIsolate; // isolate
            model->rssi = 0.0f;
            model->status = SubghzTestPacketModelStatusRx;
            return true;
        });

    api_hal_subghz_start_async_rx();

    osTimerStart(instance->timer, 1024 / 4);
}

void subghz_test_packet_exit(void* context) {
    furi_assert(context);
    SubghzTestPacket* instance = context;

    osTimerStop(instance->timer);

    // Reinitialize IC to default state
    with_view_model(
        instance->view, (SubghzTestPacketModel * model) {
            if(model->status == SubghzTestPacketModelStatusRx) {
                api_hal_subghz_stop_async_rx();
            } else {
                api_hal_subghz_stop_async_tx();
            }
            return true;
        });
    api_hal_subghz_set_async_rx_callback(NULL, NULL);
    api_hal_subghz_sleep();
}

uint32_t subghz_test_packet_back(void* context) {
    return SubGhzViewMenu;
}

SubghzTestPacket* subghz_test_packet_alloc() {
    SubghzTestPacket* instance = furi_alloc(sizeof(SubghzTestPacket));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLockFree, sizeof(SubghzTestPacketModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_test_packet_draw);
    view_set_input_callback(instance->view, subghz_test_packet_input);
    view_set_enter_callback(instance->view, subghz_test_packet_enter);
    view_set_exit_callback(instance->view, subghz_test_packet_exit);
    view_set_previous_callback(instance->view, subghz_test_packet_back);

    instance->timer =
        osTimerNew(subghz_test_packet_rssi_timer_callback, osTimerPeriodic, instance, NULL);

    instance->princeton = subghz_protocol_princeton_alloc();
    subghz_protocol_common_set_callback(
        (SubGhzProtocolCommon*)instance->princeton, subghz_test_packet_rx_pt_callback, instance);

    return instance;
}

void subghz_test_packet_free(SubghzTestPacket* instance) {
    furi_assert(instance);

    subghz_protocol_princeton_free(instance->princeton);
    osTimerDelete(instance->timer);
    view_free(instance->view);
    free(instance);
}

View* subghz_test_packet_get_view(SubghzTestPacket* instance) {
    furi_assert(instance);
    return instance->view;
}
