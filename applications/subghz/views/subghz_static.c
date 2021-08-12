#include "subghz_static.h"
#include "../subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <notification/notification-messages.h>
#include <lib/subghz/protocols/subghz_protocol_princeton.h>

static const uint32_t subghz_static_keys[] = {
    0x0074BADE,
    0x0074BADD,
    0x0074BADB,
    0x00E34A4E,
};

struct SubghzStatic {
    View* view;
    SubGhzEncoderPrinceton* encoder;
};

typedef enum {
    SubghzStaticStatusRx,
    SubghzStaticStatusTx,
} SubghzStaticStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    uint8_t button;
} SubghzStaticModel;

void subghz_static_draw(Canvas* canvas, SubghzStaticModel* model) {
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

bool subghz_static_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzStatic* instance = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        instance->view, (SubghzStaticModel * model) {
            bool reconfigure = false;
            if(event->type == InputTypeShort) {
                if(event->key == InputKeyLeft) {
                    if(model->frequency > 0) model->frequency--;
                    reconfigure = true;
                } else if(event->key == InputKeyRight) {
                    if(model->frequency < subghz_frequencies_count - 1) model->frequency++;
                    reconfigure = true;
                } else if(event->key == InputKeyDown) {
                    if(model->button > 0) model->button--;
                } else if(event->key == InputKeyUp) {
                    if(model->button < 3) model->button++;
                }
            }

            if(reconfigure) {
                furi_hal_subghz_idle();
                model->real_frequency =
                    furi_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
                furi_hal_subghz_tx();
            }

            if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    NotificationApp* notification = furi_record_open("notification");
                    notification_message_block(notification, &sequence_set_red_255);

                    subghz_encoder_princeton_reset(
                        instance->encoder, subghz_static_keys[model->button], 20);
                    furi_hal_subghz_start_async_tx(
                        subghz_encoder_princeton_yield, instance->encoder);
                    while(!furi_hal_subghz_is_async_tx_complete()) osDelay(33);
                    furi_hal_subghz_stop_async_tx();

                    notification_message(notification, &sequence_reset_red);
                    furi_record_close("notification");
                }
            }

            return true;
        });

    return true;
}

void subghz_static_enter(void* context) {
    furi_assert(context);
    SubghzStatic* instance = context;

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOokAsync);

    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, false);

    with_view_model(
        instance->view, (SubghzStaticModel * model) {
            model->frequency = subghz_frequencies_433_92;
            model->real_frequency =
                furi_hal_subghz_set_frequency_and_path(subghz_frequencies[model->frequency]);
            model->button = 0;
            return true;
        });

    furi_hal_subghz_tx();
}

void subghz_static_exit(void* context) {
    furi_assert(context);
    furi_hal_subghz_sleep();
}

SubghzStatic* subghz_static_alloc() {
    SubghzStatic* instance = furi_alloc(sizeof(SubghzStatic));

    // View allocation and configuration
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLockFree, sizeof(SubghzStaticModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)subghz_static_draw);
    view_set_input_callback(instance->view, subghz_static_input);
    view_set_enter_callback(instance->view, subghz_static_enter);
    view_set_exit_callback(instance->view, subghz_static_exit);

    instance->encoder = subghz_encoder_princeton_alloc();

    return instance;
}

void subghz_static_free(SubghzStatic* instance) {
    furi_assert(instance);
    subghz_encoder_princeton_free(instance->encoder);
    view_free(instance->view);
    free(instance);
}

View* subghz_static_get_view(SubghzStatic* instance) {
    furi_assert(instance);
    return instance->view;
}
