#include "subghz_static.h"
#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <api-hal.h>
#include <input/input.h>

static const uint8_t subghz_static_keys[][4] = {
    {0x74, 0xBA, 0xDE},
    {0x74, 0xBA, 0xDD},
    {0x74, 0xBA, 0xDB},
    {0xE3, 0x4A, 0x4E},
};

#define SUBGHZ_PT_ONE 376
#define SUBGHZ_PT_ZERO (SUBGHZ_PT_ONE * 3)
#define SUBGHZ_PT_GUARD 10600

struct SubghzStatic {
    View* view;
};

typedef enum {
    SubghzStaticStatusRx,
    SubghzStaticStatusTx,
} SubghzStaticStatus;

typedef struct {
    uint8_t frequency;
    uint32_t real_frequency;
    ApiHalSubGhzPath path;
    uint8_t button;
} SubghzStaticModel;

void subghz_static_draw(Canvas* canvas, SubghzStaticModel* model) {
    char buffer[64];

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "CC1101 Static");

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
    snprintf(buffer, sizeof(buffer), "Key: %d", model->button);
    canvas_draw_str(canvas, 2, 48, buffer);
}

bool subghz_static_input(InputEvent* event, void* context) {
    furi_assert(context);
    SubghzStatic* subghz_static = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    with_view_model(
        subghz_static->view, (SubghzStaticModel * model) {
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
                model->path = subghz_frequencies_paths[model->frequency];
            }

            if(reconfigure) {
                api_hal_subghz_idle();
                model->real_frequency =
                    api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
                api_hal_subghz_set_path(model->path);
                api_hal_subghz_tx();
            }

            if(event->key == InputKeyOk) {
                if(event->type == InputTypePress) {
                    const uint8_t* key = subghz_static_keys[model->button];

                    api_hal_light_set(LightRed, 0xff);
                    __disable_irq();
                    for(uint8_t r = 0; r < 20; r++) {
                        //Payload
                        for(uint8_t i = 0; i < 24; i++) {
                            uint8_t byte = i / 8;
                            uint8_t bit = i % 8;
                            bool value = (key[byte] >> (7 - bit)) & 1;
                            // Payload send
                            hal_gpio_write(&cc1101_g0_gpio, false);
                            delay_us(value ? SUBGHZ_PT_ONE : SUBGHZ_PT_ZERO);
                            hal_gpio_write(&cc1101_g0_gpio, true);
                            delay_us(value ? SUBGHZ_PT_ZERO : SUBGHZ_PT_ONE);
                        }
                        // Last bit
                        hal_gpio_write(&cc1101_g0_gpio, false);
                        delay_us(SUBGHZ_PT_ONE);
                        hal_gpio_write(&cc1101_g0_gpio, true);
                        // Guard time
                        delay_us(10600);
                    }
                    __enable_irq();
                    api_hal_light_set(LightRed, 0x00);
                }
            }

            return true;
        });

    return true;
}

void subghz_static_enter(void* context) {
    furi_assert(context);
    SubghzStatic* subghz_static = context;

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);

    hal_gpio_init(&cc1101_g0_gpio, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&cc1101_g0_gpio, true);

    with_view_model(
        subghz_static->view, (SubghzStaticModel * model) {
            model->frequency = 4;
            model->real_frequency =
                api_hal_subghz_set_frequency(subghz_frequencies[model->frequency]);
            model->path = subghz_frequencies_paths[model->frequency];
            model->button = 0;
            api_hal_subghz_set_path(model->path);
            return true;
        });

    api_hal_subghz_tx();
}

void subghz_static_exit(void* context) {
    furi_assert(context);
    // SubghzStatic* subghz_static = context;

    // Reinitialize IC to default state
    api_hal_subghz_init();
}

uint32_t subghz_static_back(void* context) {
    return SubGhzViewMenu;
}

SubghzStatic* subghz_static_alloc() {
    SubghzStatic* subghz_static = furi_alloc(sizeof(SubghzStatic));

    // View allocation and configuration
    subghz_static->view = view_alloc();
    view_allocate_model(subghz_static->view, ViewModelTypeLockFree, sizeof(SubghzStaticModel));
    view_set_context(subghz_static->view, subghz_static);
    view_set_draw_callback(subghz_static->view, (ViewDrawCallback)subghz_static_draw);
    view_set_input_callback(subghz_static->view, subghz_static_input);
    view_set_enter_callback(subghz_static->view, subghz_static_enter);
    view_set_exit_callback(subghz_static->view, subghz_static_exit);
    view_set_previous_callback(subghz_static->view, subghz_static_back);

    return subghz_static;
}

void subghz_static_free(SubghzStatic* subghz_static) {
    furi_assert(subghz_static);
    view_free(subghz_static->view);
    free(subghz_static);
}

View* subghz_static_get_view(SubghzStatic* subghz_static) {
    furi_assert(subghz_static);
    return subghz_static->view;
}
