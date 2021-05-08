#include "subghz_test_basic.h"
#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

struct SubghzTestBasic {
    View* view;
    osTimerId timer;
};

typedef enum {
    SubghzTestBasicModelStatusRx,
    SubghzTestBasicModelStatusTx,
} SubghzTestBasicModelStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    ApiHalSubGhzPath path;
    float rssi;
    SubghzTestBasicModelStatus status;
} SubghzTestBasicModel;

void subghz_test_basic_draw(Canvas* canvas, SubghzTestBasicModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "CC1101 Basic Test");

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
    } else if(model->path == ApiHalSubGhzPath1) {
        path_name = "433MHz";
    } else if(model->path == ApiHalSubGhzPath2) {
        path_name = "315MHz";
    } else if(model->path == ApiHalSubGhzPath3) {
        path_name = "868MHz";
    }
    snprintf(buffer, sizeof(buffer), "Path: %d - %s", model->path, path_name);
    canvas_draw_str(canvas, 2, 36, buffer);
    if(model->status == SubghzTestBasicModelStatusRx) {
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

bool subghz_test_basic_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzTestBasic* subghz_test_basic = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        subghz_test_basic->view, (SubghzTestBasicModel * model) {
            osTimerStop(subghz_test_basic->timer);
            api_hal_subghz_idle();

            if(event->type == InputTypeShort) {
                if(event->key == InputKeyLeft) {
                    if(model->frequency > 0) model->frequency--;
                } else if(event->key == InputKeyRight) {
                    if(model->frequency < subghz_frequencies_count - 1) model->frequency++;
                } else if(event->key == InputKeyDown) {
                    if(model->path > 0) model->path--;
                } else if(event->key == InputKeyUp) {
                    if(model->path < ApiHalSubGhzPath3) model->path++;
                }

                model->real_frequency =
                    api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
                api_hal_subghz_set_path(model->path);
            }

            if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    model->status = SubghzTestBasicModelStatusTx;
                } else if(event->type == InputTypeRelease) {
                    model->status = SubghzTestBasicModelStatusRx;
                }
            }

            if(model->status == SubghzTestBasicModelStatusRx) {
                hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
                api_hal_subghz_rx();
                osTimerStart(subghz_test_basic->timer, 1024 / 4);
            } else {
                hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
                hal_gpio_write(&gpio_cc1101_g0, false);
                api_hal_subghz_tx();
            }

            return true;
        });

    return true;
}

void subghz_test_basic_enter(void* context) {
    furi_assert(context);
    SubghzTestBasic* subghz_test_basic = context;

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);

    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    with_view_model(
        subghz_test_basic->view, (SubghzTestBasicModel * model) {
            model->frequency = 4; // 433
            model->real_frequency =
                api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
            model->path = ApiHalSubGhzPathIsolate; // isolate
            model->rssi = 0.0f;
            model->status = SubghzTestBasicModelStatusRx;
            return true;
        });

    api_hal_subghz_rx();

    osTimerStart(subghz_test_basic->timer, 1024 / 4);
}

void subghz_test_basic_exit(void* context) {
    furi_assert(context);
    SubghzTestBasic* subghz_test_basic = context;

    osTimerStop(subghz_test_basic->timer);

    // Reinitialize IC to default state
    api_hal_subghz_init();
}

void subghz_test_basic_rssi_timer_callback(void* context) {
    furi_assert(context);
    SubghzTestBasic* subghz_test_basic = context;

    with_view_model(
        subghz_test_basic->view, (SubghzTestBasicModel * model) {
            model->rssi = api_hal_subghz_get_rssi();
            return true;
        });
}

uint32_t subghz_test_basic_back(void* context) {
    return SubGhzViewMenu;
}

SubghzTestBasic* subghz_test_basic_alloc() {
    SubghzTestBasic* subghz_test_basic = furi_alloc(sizeof(SubghzTestBasic));

    // View allocation and configuration
    subghz_test_basic->view = view_alloc();
    view_allocate_model(
        subghz_test_basic->view, ViewModelTypeLockFree, sizeof(SubghzTestBasicModel));
    view_set_context(subghz_test_basic->view, subghz_test_basic);
    view_set_draw_callback(subghz_test_basic->view, (ViewDrawCallback)subghz_test_basic_draw);
    view_set_input_callback(subghz_test_basic->view, subghz_test_basic_input);
    view_set_enter_callback(subghz_test_basic->view, subghz_test_basic_enter);
    view_set_exit_callback(subghz_test_basic->view, subghz_test_basic_exit);
    view_set_previous_callback(subghz_test_basic->view, subghz_test_basic_back);

    subghz_test_basic->timer = osTimerNew(
        subghz_test_basic_rssi_timer_callback, osTimerPeriodic, subghz_test_basic, NULL);

    return subghz_test_basic;
}

void subghz_test_basic_free(SubghzTestBasic* subghz_test_basic) {
    furi_assert(subghz_test_basic);
    osTimerDelete(subghz_test_basic->timer);
    view_free(subghz_test_basic->view);
    free(subghz_test_basic);
}

View* subghz_test_basic_get_view(SubghzTestBasic* subghz_test_basic) {
    furi_assert(subghz_test_basic);
    return subghz_test_basic->view;
}
