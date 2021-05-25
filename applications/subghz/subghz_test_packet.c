#include "subghz_test_packet.h"
#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

static const uint8_t subghz_test_packet_data[] = {
    0x30, // 48bytes to transmit
    'F',  'L', 'I', 'P', 'P', 'E', 'R', ' ', 'T', 'E', 'S', 'T', ' ', 'P', 'A', 'C',
    'K',  'E', 'T', ' ', 'D', 'A', 'T', 'A', ' ', 'A', 'N', 'D', ' ', 'P', 'A', 'D',
    'F',  'L', 'I', 'P', 'P', 'E', 'R', ' ', 'T', 'E', 'S', 'T', ' ', 'P', 'A', 'C',

};

struct SubghzTestPacket {
    View* view;
    osTimerId timer;
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
    SubghzTestPacketModelStatus status;
} SubghzTestPacketModel;

void subghz_test_packet_draw(Canvas* canvas, SubghzTestPacketModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "CC1101 Packet Test");

    canvas_set_font(canvas, FontSecondary);
    // Frequency
    snprintf(
        buffer,
        sizeof(buffer),
        "Freq: %03ld.%03ld.%03ld Hz",
        model->real_frequency / 1000000 % 1000,
        model->real_frequency / 1000 % 1000,
        model->real_frequency % 1000);
    canvas_draw_str(canvas, 2, 24, buffer);
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
    canvas_draw_str(canvas, 2, 36, buffer);
    if(model->status == SubghzTestPacketModelStatusRx) {
        snprintf(
            buffer,
            sizeof(buffer),
            "RSSI: %ld.%ld dBm",
            (int32_t)(model->rssi),
            (int32_t)fabs(model->rssi * 10) % 10);
        canvas_draw_str(canvas, 2, 48, buffer);
    } else {
        canvas_draw_str(canvas, 2, 48, "TX");
    }
}

bool subghz_test_packet_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzTestPacket* subghz_test_packet = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        subghz_test_packet->view, (SubghzTestPacketModel * model) {
            osTimerStop(subghz_test_packet->timer);
            api_hal_subghz_idle();

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
                    api_hal_subghz_set_frequency(subghz_frequencies[model->frequency].frequency);
                api_hal_subghz_set_path(model->path);
            }

            if(model->status == SubghzTestPacketModelStatusRx) {
                api_hal_subghz_rx();
                osTimerStart(subghz_test_packet->timer, 1024 / 4);
            } else {
                api_hal_subghz_write_packet(
                    subghz_test_packet_data, sizeof(subghz_test_packet_data));
                api_hal_subghz_tx();
            }

            return true;
        });

    return true;
}

void subghz_test_packet_enter(void* context) {
    furi_assert(context);
    SubghzTestPacket* subghz_test_packet = context;

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPreset2FskPacket);

    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    with_view_model(
        subghz_test_packet->view, (SubghzTestPacketModel * model) {
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                api_hal_subghz_set_frequency(subghz_frequencies[model->frequency].frequency);
            model->path = ApiHalSubGhzPathIsolate; // isolate
            model->rssi = 0.0f;
            model->status = SubghzTestPacketModelStatusRx;
            return true;
        });

    api_hal_subghz_rx();

    osTimerStart(subghz_test_packet->timer, 1024 / 4);
}

void subghz_test_packet_exit(void* context) {
    furi_assert(context);
    SubghzTestPacket* subghz_test_packet = context;

    osTimerStop(subghz_test_packet->timer);

    // Reinitialize IC to default state
    api_hal_subghz_init();
}

void subghz_test_packet_rssi_timer_callback(void* context) {
    furi_assert(context);
    SubghzTestPacket* subghz_test_packet = context;

    with_view_model(
        subghz_test_packet->view, (SubghzTestPacketModel * model) {
            model->rssi = api_hal_subghz_get_rssi();
            return true;
        });
}

uint32_t subghz_test_packet_back(void* context) {
    return SubGhzViewMenu;
}

SubghzTestPacket* subghz_test_packet_alloc() {
    SubghzTestPacket* subghz_test_packet = furi_alloc(sizeof(SubghzTestPacket));

    // View allocation and configuration
    subghz_test_packet->view = view_alloc();
    view_allocate_model(
        subghz_test_packet->view, ViewModelTypeLockFree, sizeof(SubghzTestPacketModel));
    view_set_context(subghz_test_packet->view, subghz_test_packet);
    view_set_draw_callback(subghz_test_packet->view, (ViewDrawCallback)subghz_test_packet_draw);
    view_set_input_callback(subghz_test_packet->view, subghz_test_packet_input);
    view_set_enter_callback(subghz_test_packet->view, subghz_test_packet_enter);
    view_set_exit_callback(subghz_test_packet->view, subghz_test_packet_exit);
    view_set_previous_callback(subghz_test_packet->view, subghz_test_packet_back);

    subghz_test_packet->timer = osTimerNew(
        subghz_test_packet_rssi_timer_callback, osTimerPeriodic, subghz_test_packet, NULL);

    return subghz_test_packet;
}

void subghz_test_packet_free(SubghzTestPacket* subghz_test_packet) {
    furi_assert(subghz_test_packet);
    osTimerDelete(subghz_test_packet->timer);
    view_free(subghz_test_packet->view);
    free(subghz_test_packet);
}

View* subghz_test_packet_get_view(SubghzTestPacket* subghz_test_packet) {
    furi_assert(subghz_test_packet);
    return subghz_test_packet->view;
}
