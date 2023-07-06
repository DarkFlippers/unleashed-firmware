#include "subghz_test_static.h"
#include "../subghz_test_app_i.h"
#include "../helpers/subghz_test_frequency.h"
#include <lib/subghz/devices/cc1101_configs.h>

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include "../protocol/princeton_for_testing.h"

#define TAG "SubGhzTestStatic"

typedef enum {
    SubGhzTestStaticStatusIDLE,
    SubGhzTestStaticStatusTX,
} SubGhzTestStaticStatus;

static const uint32_t subghz_test_static_keys[] = {
    0x0074BADE,
    0x0074BADD,
    0x0074BADB,
    0x00E34A4E,
};

struct SubGhzTestStatic {
    View* view;
    SubGhzTestStaticStatus status_tx;
    SubGhzEncoderPrinceton* encoder;
    SubGhzTestStaticCallback callback;
    void* context;
};

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    uint8_t button;
} SubGhzTestStaticModel;

void subghz_test_static_set_callback(
    SubGhzTestStatic* subghz_test_static,
    SubGhzTestStaticCallback callback,
    void* context) {
    furi_assert(subghz_test_static);
    furi_assert(callback);
    subghz_test_static->callback = callback;
    subghz_test_static->context = context;
}

void subghz_test_static_draw(Canvas* canvas, SubGhzTestStaticModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 8, "CC1101 Static");

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
    snprintf(buffer, sizeof(buffer), "Key: %d", model->button);
    canvas_draw_str(canvas, 0, 31, buffer);
}

bool subghz_test_static_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubGhzTestStatic* instance = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        instance->view,
        SubGhzTestStaticModel * model,
        {
            if(event->type == InputTypeShort) {
                if(event->key == InputKeyLeft) {
                    if(model->frequency > 0) model->frequency--;
                } else if(event->key == InputKeyRight) {
                    if(model->frequency < subghz_frequencies_count_testing - 1) model->frequency++;
                } else if(event->key == InputKeyDown) {
                    if(model->button > 0) model->button--;
                } else if(event->key == InputKeyUp) {
                    if(model->button < 3) model->button++;
                }
            }

            model->real_frequency = subghz_frequencies_testing[model->frequency];

            if(event->key == InputKeyOk) {
                NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
                if(event->type == InputTypePress) {
                    furi_hal_subghz_idle();
                    furi_hal_subghz_set_frequency_and_path(
                        subghz_frequencies_testing[model->frequency]);
                    if(!furi_hal_subghz_tx()) {
                        instance->callback(SubGhzTestStaticEventOnlyRx, instance->context);
                    } else {
                        notification_message_block(notification, &sequence_set_red_255);

                        FURI_LOG_I(TAG, "TX Start");

                        subghz_encoder_princeton_for_testing_set(
                            instance->encoder,
                            subghz_test_static_keys[model->button],
                            10000,
                            subghz_frequencies_testing[model->frequency]);

                        furi_hal_subghz_start_async_tx(
                            subghz_encoder_princeton_for_testing_yield, instance->encoder);
                        instance->status_tx = SubGhzTestStaticStatusTX;
                    }
                } else if(event->type == InputTypeRelease) {
                    if(instance->status_tx == SubGhzTestStaticStatusTX) {
                        FURI_LOG_I(TAG, "TX Stop");
                        subghz_encoder_princeton_for_testing_stop(
                            instance->encoder, furi_get_tick());
                        subghz_encoder_princeton_for_testing_print_log(instance->encoder);
                        furi_hal_subghz_stop_async_tx();
                        notification_message(notification, &sequence_reset_red);
                    }
                    instance->status_tx = SubGhzTestStaticStatusIDLE;
                }
                furi_record_close(RECORD_NOTIFICATION);
            }
        },
        true);

    return true;
}

void subghz_test_static_enter(void* context) {
    furi_assert(context);
    SubGhzTestStatic* instance = context;

    furi_hal_subghz_reset();
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);

    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_cc1101_g0, false);
    instance->status_tx = SubGhzTestStaticStatusIDLE;

    with_view_model(
        instance->view,
        SubGhzTestStaticModel * model,
        {
            model->frequency = subghz_frequencies_433_92_testing;
            model->real_frequency = subghz_frequencies_testing[model->frequency];
            model->button = 0;
        },
        true);
}

void subghz_test_static_exit(void* context) {
    furi_assert(context);
    furi_hal_subghz_sleep();
}

SubGhzTestStatic* subghz_test_static_alloc() {
    SubGhzTestStatic* instance = malloc(sizeof(SubGhzTestStatic));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubGhzTestStaticModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_test_static_draw);
    view_set_input_callback(instance->view, subghz_test_static_input);
    view_set_enter_callback(instance->view, subghz_test_static_enter);
    view_set_exit_callback(instance->view, subghz_test_static_exit);

    instance->encoder = subghz_encoder_princeton_for_testing_alloc();

    return instance;
}

void subghz_test_static_free(SubGhzTestStatic* instance) {
    furi_assert(instance);
    subghz_encoder_princeton_for_testing_free(instance->encoder);
    view_free(instance->view);
    free(instance);
}

View* subghz_test_static_get_view(SubGhzTestStatic* instance) {
    furi_assert(instance);
    return instance->view;
}
