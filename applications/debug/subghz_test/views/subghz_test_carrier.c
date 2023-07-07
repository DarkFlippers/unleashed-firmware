#include "subghz_test_carrier.h"
#include "../subghz_test_app_i.h"
#include "../helpers/subghz_test_frequency.h"
#include <lib/subghz/devices/cc1101_configs.h>

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>

struct SubGhzTestCarrier {
    View* view;
    FuriTimer* timer;
    SubGhzTestCarrierCallback callback;
    // const SubGhzDevice* radio_device;
    void* context;
};

typedef enum {
    SubGhzTestCarrierModelStatusRx,
    SubGhzTestCarrierModelStatusTx,
} SubGhzTestCarrierModelStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    FuriHalSubGhzPath path;
    float rssi;
    SubGhzTestCarrierModelStatus status;
} SubGhzTestCarrierModel;

void subghz_test_carrier_set_callback(
    SubGhzTestCarrier* subghz_test_carrier,
    SubGhzTestCarrierCallback callback,
    void* context) {
    furi_assert(subghz_test_carrier);
    furi_assert(callback);
    subghz_test_carrier->callback = callback;
    subghz_test_carrier->context = context;
}

void subghz_test_carrier_draw(Canvas* canvas, SubGhzTestCarrierModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "CC1101 Basic Test");

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
    if(model->status == SubGhzTestCarrierModelStatusRx) {
        snprintf(
            buffer,
            sizeof(buffer),
            "RSSI: %ld.%ld dBm",
            (int32_t)(model->rssi),
            (int32_t)fabs(model->rssi * 10) % 10);
        canvas_draw_str(canvas, 0, 42, buffer);
    } else {
        canvas_draw_str(canvas, 0, 42, "TX");
    }
}

bool subghz_test_carrier_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzTestCarrier* subghz_test_carrier = context;
    // const SubGhzDevice* radio_device = subghz_test_carrier->radio_device;

    if(event->key == InputKeyBack || event->type != InputTypeShort) {
        return false;
    }

    with_view_model(
        subghz_test_carrier->view,
        SubGhzTestCarrierModel * model,
        {
            furi_hal_subghz_idle();
            // subghz_devices_idle(radio_device);

            if(event->key == InputKeyLeft) {
                if(model->frequency > 0) model->frequency--;
            } else if(event->key == InputKeyRight) {
                if(model->frequency < subghz_frequencies_count_testing - 1) model->frequency++;
            } else if(event->key == InputKeyDown) {
                if(model->path > 0) model->path--;
            } else if(event->key == InputKeyUp) {
                if(model->path < FuriHalSubGhzPath868) model->path++;
            } else if(event->key == InputKeyOk) {
                if(model->status == SubGhzTestCarrierModelStatusTx) {
                    model->status = SubGhzTestCarrierModelStatusRx;
                } else {
                    model->status = SubGhzTestCarrierModelStatusTx;
                }
            }

            model->real_frequency =
                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
            furi_hal_subghz_set_path(model->path);
            // model->real_frequency = subghz_devices_set_frequency(
            //     radio_device, subghz_frequencies_testing[model->frequency]);

            if(model->status == SubGhzTestCarrierModelStatusRx) {
                furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
                furi_hal_subghz_rx();
                // furi_hal_gpio_init(
                //     subghz_devices_get_data_gpio(radio_device),
                //     GpioModeInput,
                //     GpioPullNo,
                //     GpioSpeedLow);
                // subghz_devices_set_rx(radio_device);
            } else {
                furi_hal_gpio_init(
                    &gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
                furi_hal_gpio_write(&gpio_cc1101_g0, true);
                if(!furi_hal_subghz_tx()) {
                    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
                    subghz_test_carrier->callback(
                        SubGhzTestCarrierEventOnlyRx, subghz_test_carrier->context);
                }
                // if(!subghz_devices_set_tx(radio_device)) {
                //     furi_hal_gpio_init(
                //         subghz_devices_get_data_gpio(radio_device),
                //         GpioModeInput,
                //         GpioPullNo,
                //         GpioSpeedLow);
                //     subghz_test_carrier->callback(
                //         SubGhzTestCarrierEventOnlyRx, subghz_test_carrier->context);
                // }
            }
        },
        true);

    return true;
}

void subghz_test_carrier_enter(void* context) {
    furi_assert(context);
    SubGhzTestCarrier* subghz_test_carrier = context;
    // furi_assert(subghz_test_carrier->radio_device);
    // const SubGhzDevice* radio_device = subghz_test_carrier->radio_device;

    furi_hal_subghz_reset();
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);

    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    // subghz_devices_reset(radio_device);
    // subghz_devices_load_preset(radio_device, FuriHalSubGhzPresetOok650Async, NULL);

    // furi_hal_gpio_init(
    //     subghz_devices_get_data_gpio(radio_device), GpioModeInput, GpioPullNo, GpioSpeedLow);

    with_view_model(
        subghz_test_carrier->view,
        SubGhzTestCarrierModel * model,
        {
            model->frequency = subghz_frequencies_433_92_testing; // 433
            model->real_frequency =
                furi_hal_subghz_set_frequency(subghz_frequencies_testing[model->frequency]);
            // model->real_frequency = subghz_devices_set_frequency(
            //     radio_device, subghz_frequencies_testing[model->frequency]);
            model->path = FuriHalSubGhzPathIsolate; // isolate
            model->rssi = 0.0f;
            model->status = SubGhzTestCarrierModelStatusRx;
        },
        true);

    furi_hal_subghz_rx();
    // subghz_devices_set_rx(radio_device);

    furi_timer_start(subghz_test_carrier->timer, furi_kernel_get_tick_frequency() / 4);
}

void subghz_test_carrier_exit(void* context) {
    furi_assert(context);
    SubGhzTestCarrier* subghz_test_carrier = context;

    furi_timer_stop(subghz_test_carrier->timer);

    // Reinitialize IC to default state
    furi_hal_subghz_sleep();
    // subghz_devices_sleep(subghz_test_carrier->radio_device);
}

void subghz_test_carrier_rssi_timer_callback(void* context) {
    furi_assert(context);
    SubGhzTestCarrier* subghz_test_carrier = context;

    with_view_model(
        subghz_test_carrier->view,
        SubGhzTestCarrierModel * model,
        {
            if(model->status == SubGhzTestCarrierModelStatusRx) {
                model->rssi = furi_hal_subghz_get_rssi();
                // model->rssi = subghz_devices_get_rssi(subghz_test_carrier->radio_device);
            }
        },
        false);
}

SubGhzTestCarrier* subghz_test_carrier_alloc() {
    SubGhzTestCarrier* subghz_test_carrier = malloc(sizeof(SubGhzTestCarrier));

    // View allocation and configuration
    subghz_test_carrier->view = view_alloc();
    view_allocate_model(
        subghz_test_carrier->view, ViewModelTypeLocking, sizeof(SubGhzTestCarrierModel));
    view_set_context(subghz_test_carrier->view, subghz_test_carrier);
    view_set_draw_callback(subghz_test_carrier->view, (ViewDrawCallback)subghz_test_carrier_draw);
    view_set_input_callback(subghz_test_carrier->view, subghz_test_carrier_input);
    view_set_enter_callback(subghz_test_carrier->view, subghz_test_carrier_enter);
    view_set_exit_callback(subghz_test_carrier->view, subghz_test_carrier_exit);

    subghz_test_carrier->timer = furi_timer_alloc(
        subghz_test_carrier_rssi_timer_callback, FuriTimerTypePeriodic, subghz_test_carrier);

    return subghz_test_carrier;
}

void subghz_test_carrier_free(SubGhzTestCarrier* subghz_test_carrier) {
    furi_assert(subghz_test_carrier);
    furi_timer_free(subghz_test_carrier->timer);
    view_free(subghz_test_carrier->view);
    free(subghz_test_carrier);
}

View* subghz_test_carrier_get_view(SubGhzTestCarrier* subghz_test_carrier) {
    furi_assert(subghz_test_carrier);
    return subghz_test_carrier->view;
}

// void subghz_test_carrier_set_radio(
//     SubGhzTestCarrier* subghz_test_carrier,
//     const SubGhzDevice* radio_device) {
//     furi_assert(subghz_test_carrier);
//     subghz_test_carrier->radio_device = radio_device;
// }
