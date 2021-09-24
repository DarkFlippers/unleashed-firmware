#include <furi.h>
#include <furi-hal.h>

#include <gui/gui.h>
#include <notification/notification-messages.h>

typedef struct {
    const char* name;
    const GpioPin* pin;
} GpioItem;

static const GpioItem GPIO_PINS[] = {
    {"1.2: PA7", &gpio_ext_pa7},
    {"1.3: PA6", &gpio_ext_pa6},
    {"1.4: PA4", &gpio_ext_pa4},
    {"1.5: PB3", &gpio_ext_pb3},
    {"1.6: PB2", &gpio_ext_pb2},
    {"1.7: PC3", &gpio_ext_pc3},
    {"2.7: PC1", &gpio_ext_pc1},
    {"2.8: PC0", &gpio_ext_pc0},
    {"*.*: ALL", NULL},
};

static const size_t GPIO_PINS_COUNT = sizeof(GPIO_PINS) / sizeof(GPIO_PINS[0]);

typedef struct {
    osMessageQueueId_t input_queue;
    uint8_t gpio_index;
    ViewPort* view_port;
    Gui* gui;
    NotificationApp* notification;
} GpioTest;

static void gpio_test_render_callback(Canvas* canvas, void* ctx) {
    GpioTest* gpio_test = ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GPIO Control");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, GPIO_PINS[gpio_test->gpio_index].name);
}

static void gpio_test_input_callback(InputEvent* input_event, void* ctx) {
    GpioTest* gpio_test = ctx;

    osMessageQueuePut(gpio_test->input_queue, input_event, 0, 0);
}

static void gpio_test_configure_pins(GpioMode mode) {
    for(size_t i = 0; i < GPIO_PINS_COUNT; i++) {
        if(!GPIO_PINS[i].pin) continue;
        hal_gpio_write(GPIO_PINS[i].pin, false);
        hal_gpio_init(GPIO_PINS[i].pin, mode, GpioPullNo, GpioSpeedVeryHigh);
    }
}

static void gpio_test_set_pin(uint8_t index, bool level) {
    if(GPIO_PINS[index].pin) {
        hal_gpio_write(GPIO_PINS[index].pin, level);
    } else {
        for(size_t i = 0; i < GPIO_PINS_COUNT; i++) {
            if(!GPIO_PINS[i].pin) continue;
            hal_gpio_write(GPIO_PINS[i].pin, level);
        }
    }
}

GpioTest* gpio_test_alloc() {
    GpioTest* instance = furi_alloc(sizeof(GpioTest));

    gpio_test_configure_pins(GpioModeOutputPushPull);

    instance->input_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);
    furi_check(instance->input_queue);

    instance->view_port = view_port_alloc();
    view_port_draw_callback_set(instance->view_port, gpio_test_render_callback, instance);
    view_port_input_callback_set(instance->view_port, gpio_test_input_callback, instance);

    instance->gui = furi_record_open("gui");
    gui_add_view_port(instance->gui, instance->view_port, GuiLayerFullscreen);

    instance->notification = furi_record_open("notification");

    return instance;
}

void gpio_test_free(GpioTest* instance) {
    furi_assert(instance);

    furi_record_close("notification");

    view_port_enabled_set(instance->view_port, false);
    gui_remove_view_port(instance->gui, instance->view_port);
    furi_record_close("gui");

    view_port_free(instance->view_port);

    osMessageQueueDelete(instance->input_queue);

    gpio_test_configure_pins(GpioModeAnalog);

    free(instance);
}

int32_t gpio_test_app(void* p) {
    GpioTest* gpio_test = gpio_test_alloc();

    InputEvent event;
    while(osMessageQueueGet(gpio_test->input_queue, &event, NULL, osWaitForever) == osOK) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                notification_message(gpio_test->notification, &sequence_reset_green);
                break;
            }

            if(event.key == InputKeyRight) {
                if(gpio_test->gpio_index < (GPIO_PINS_COUNT - 1)) {
                    gpio_test->gpio_index++;
                }
            }

            if(event.key == InputKeyLeft) {
                if(gpio_test->gpio_index > 0) {
                    gpio_test->gpio_index--;
                }
            }
        } else {
            if(event.key == InputKeyOk) {
                if(event.type == InputTypePress) {
                    gpio_test_set_pin(gpio_test->gpio_index, true);
                    notification_message(gpio_test->notification, &sequence_set_green_255);
                } else if(event.type == InputTypeRelease) {
                    gpio_test_set_pin(gpio_test->gpio_index, false);
                    notification_message(gpio_test->notification, &sequence_reset_green);
                }
            }
        }

        view_port_update(gpio_test->view_port);
    }

    gpio_test_free(gpio_test);

    return 0;
}
